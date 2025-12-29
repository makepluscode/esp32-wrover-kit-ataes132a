/**
 * @file main.cpp
 * @brief 예제: 99_config_lock - 보안 설정 및 영구 잠금 (정밀 구현 v2)
 *
 * 이 예제는 UNLOCKED 상태인 ATAES132A 칩을 보안 모듈로 설정하고 최종 Lock을
 * 수행합니다. 수정 사항: BlockRead 32바이트 분할 처리 및 Zone 0 쓰기 상태
 * 확보를 위한 단계 재구성.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <mbedtls/ccm.h>

// 설정 주소 정의
#define LOCK_CONFIG_ADDR 0xF020
#define MANID_ADDR 0xF02A
#define ZONE_CONFIG_BASE 0xF0C0
#define KEY_CONFIG_BASE 0xF080
#define KEY_MEMORY_BASE 0xF200
#define USER_ZONE0_ADDR 0x0000
#define USER_ZONE1_ADDR 0x0100
#define USER_ZONE3_ADDR 0x0300

// 테스트용 키 및 상수
const uint8_t TEST_KEY0[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
                               0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};
const uint8_t TEST_KEY1[16] = {0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
                               0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};
const uint8_t REAL_MANID[2] = {0x00, 0xEE}; // Default ManID

// 함수 프로토타입
bool askUserConfirmation();
void run_example();
bool calculate_auth_mac(const uint8_t *key, const uint8_t *nonce,
                        const uint8_t *man_id, uint8_t op, uint8_t mode,
                        uint16_t p1, uint16_t p2, uint8_t *out_mac);

void setup() {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial)
    delay(10);

  Serial.println("\n========================================");
  Serial.println("ATAES132A 보안 설정 및 영구 잠금 (v2)");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 초기화 실패");
    return;
  }

  if (askUserConfirmation()) {
    run_example();
  } else {
    Serial.println("사용자에 의해 중단되었습니다.");
  }
}

void loop() { delay(1000); }

bool askUserConfirmation() {
  Serial.println("!!! 경고: 이 작업은 칩의 설정을 영구적으로 잠급니다. !!!");
  Serial.println("!!! 잠금 이후에는 설정을 변경할 수 없습니다. !!!");
  Serial.println("계속하시겠습니까? (Y/N)");

  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'Y' || c == 'y')
        return true;
      if (c == 'N' || c == 'n')
        return false;
    }
    delay(10);
  }
}

void run_example() {
  uint8_t ret;
  uint8_t data[128];
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // --- [Phase 0] 기기 진단 및 초기 상태 출력 ---
  Serial.println("\n[Phase 0] 기기 진단...");
  aes132c_wakeup();
  delay(50);

  // 0-1. LockConfig 확인 (BlockRead 0x10 사용)
  ret = aes132m_execute(AES132_BLOCK_READ, 0x00, LOCK_CONFIG_ADDR, 4, 0, NULL,
                        0, NULL, 0, NULL, 0, NULL, tx_buf, rx_buf);
  if (ret != 0 || rx_buf[1] != 0) {
    Serial.printf("상태 읽기 실패: 0x%02X (Ret: 0x%02X)\n", ret, rx_buf[1]);
    return;
  }
  Serial.printf("실제 LockConfig 상태 (0xF020): 0x%02X\n", rx_buf[2]);
  if (rx_buf[2] != 0x55) {
    Serial.println("칩이 이미 잠겨있습니다. 중단합니다.");
    return;
  }

  // 0-2. ZoneConfig 전체 덤프 (BlockRead 32바이트 분할 처리)
  Serial.println("현재 ZoneConfig 덤프 (0xF0C0~0xF0FF):");
  for (int chunk = 0; chunk < 2; chunk++) {
    uint16_t addr = ZONE_CONFIG_BASE + (chunk * 32);
    ret = aes132m_execute(AES132_BLOCK_READ, 0x00, addr, 32, 0, NULL, 0, NULL,
                          0, NULL, 0, NULL, tx_buf, rx_buf);
    if (ret == 0 && rx_buf[1] == 0) {
      for (int i = 0; i < 8; i++) {
        Serial.printf("Zone %d: ", (chunk * 8) + i);
        print_hex("", &rx_buf[2 + (i * 4)], 4);
      }
    } else {
      Serial.printf("ZoneConfig 덤프 실패 (Addr 0x%04X): 0x%02X\n", addr,
                    rx_buf[1]);
    }
  }

  // --- [Phase 1] 초기 보안 설정 (쓰기 권한 확보) ---
  Serial.println("\n[Phase 1] 초기 보안 설정 (Zone 0 쓰기 권한 확보)...");

  // Zone 0을 일단 자유 쓰기 상태(0x00 00 00 00)로 설정하여 Magic Number 기입
  // 준비
  uint8_t open_conf[4] = {0x00, 0x00, 0x00, 0x00};
  ret = aes132m_write_memory(4, ZONE_CONFIG_BASE, open_conf);
  print_result("Zone 0 임시 설정 (Open)", ret);

  // Zone 1, 2 설정을 미리 진행 (AuthWrite, AuthID=0)
  uint8_t z12_conf[4] = {0x02, 0x00, 0x00, 0x00};
  aes132m_write_memory(4, ZONE_CONFIG_BASE + 4, z12_conf);
  aes132m_write_memory(4, ZONE_CONFIG_BASE + 8, z12_conf);

  // KeyConfig 0-4 설정
  uint8_t k_conf[4] = {0x89, 0x00, 0x00, 0x00};
  for (int i = 0; i < 5; i++) {
    aes132m_write_memory(4, KEY_CONFIG_BASE + (i * 4), k_conf);
  }
  Serial.println("-> 초기 설정 기입 완료.");

  // --- [Phase 2] 유저 데이터 기입 ---
  Serial.println("\n[Phase 2] User Zone 0 데이터 기입 (Magic Number)...");
  for (int i = 0; i < 128; i += 2) {
    data[i] = 0xC0;
    data[i + 1] = 0xDE;
  }
  for (int i = 0; i < 128; i += 32) {
    ret = aes132m_write_memory(32, USER_ZONE0_ADDR + i, &data[i]);
    if (ret != 0) {
      Serial.printf("Zone 0 기입 실패 (Offset %d): 0x%02X\n", i, ret);
      return;
    }
  }
  Serial.println("-> Zone 0 데이터 기입 완료.");

  // --- [Phase 3] 최종 보안 정책 적용 ---
  Serial.println("\n[Phase 3] 최종 보안 정책 적용 (Zone 0 -> Read-only)...");
  uint8_t z0_final_conf[4] = {0x10, 0x00, 0x00, 0x00};
  ret = aes132m_write_memory(4, ZONE_CONFIG_BASE, z0_final_conf);
  print_result("Zone 0 최종 설정 (Read-only)", ret);

  // --- [Phase 4] 비밀 키(Key) 로딩 ---
  Serial.println("\n[Phase 4] 비밀 키 로딩 (Direct Write)...");
  aes132m_write_memory(16, KEY_MEMORY_BASE, (uint8_t *)TEST_KEY0);
  aes132m_write_memory(16, KEY_MEMORY_BASE + 16, (uint8_t *)TEST_KEY1);
  Serial.println("-> Key 0, 1 로딩 완료.");

  // --- [Phase 5] 물리적 잠금 (Lock Command) ---
  Serial.println("\n[Phase 5] 물리적 잠금 실행 (영구적)...");

  // 1. Config Memory Lock: Mode 0x02
  ret = aes132m_execute(AES132_LOCK, 0x02, 0x0000, 0x0000, 0, NULL, 0, NULL, 0,
                        NULL, 0, NULL, tx_buf, rx_buf);
  print_result("Config Memory Lock (Mode 0x02)", ret);
  if (ret != 0)
    return;

  // 2. Key Memory Lock: Mode 0x01
  ret = aes132m_execute(AES132_LOCK, 0x01, 0x0000, 0x0000, 0, NULL, 0, NULL, 0,
                        NULL, 0, NULL, tx_buf, rx_buf);
  print_result("Key Memory Lock (Mode 0x01)", ret);
  if (ret != 0)
    return;

  // --- 사후 검증 로직 ---
  Serial.println("\n[Final Check] 사후 검증 수행...");

  // 1. RO 테스트
  Serial.println("1. RO 테스트 (Zone 0 쓰기 시도)...");
  uint8_t dummy[16] = {0xFF};
  ret = aes132m_write_memory(16, USER_ZONE0_ADDR, dummy);
  Serial.printf("-> 결과: 0x%02X (0x04 or 0x08 기대)\n", ret);

  // 2. AuthWrite 테스트
  Serial.println("2. AuthWrite 테스트 (Zone 1)...");
  ret = aes132m_write_memory(16, USER_ZONE1_ADDR, dummy);
  Serial.printf("-> 인증 전 쓰기 시도: 0x%02X (에러 기대)\n", ret);

  // Nonce 생성 및 Auth 수행
  uint8_t nonce[12];
  uint8_t seed[12] = {0};
  ret = aes132m_execute(AES132_NONCE, 0x01, 0, 0, 12, seed, 0, NULL, 0, NULL, 0,
                        NULL, tx_buf, rx_buf);
  if (ret == 0) {
    memcpy(nonce, &rx_buf[2], 12);
    uint8_t mac_tag[16];
    if (calculate_auth_mac(TEST_KEY0, nonce, REAL_MANID, AES132_AUTH, 0x01,
                           0x0000, 0x0000, mac_tag)) {
      ret = aes132m_execute(AES132_AUTH, 0x01, 0x0000, 0x0000, 16, mac_tag, 0,
                            NULL, 0, NULL, 0, NULL, tx_buf, rx_buf);
      print_result("인증 명령 (Key 0)", ret);
      if (ret == 0) {
        uint8_t test_data[16] = "AUTH_SUCCESS";
        ret = aes132m_write_memory(16, USER_ZONE1_ADDR, test_data);
        print_result("인증 후 Zone 1 쓰기", ret);
      }
    }
  }

  // 3. Open 테스트
  Serial.println("3. Open 테스트 (Zone 3)...");
  uint8_t test_val[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  aes132m_write_memory(4, USER_ZONE3_ADDR, test_val);
  uint8_t read_val[4] = {0};
  ret = aes132m_read_memory(4, USER_ZONE3_ADDR, read_val);
  if (ret == 0 && memcmp(test_val, read_val, 4) == 0) {
    Serial.println("-> Zone 3 R/W 확인 완료.");
  } else {
    Serial.println("-> Zone 3 테스트 실패.");
  }

  Serial.println("\n[Success] 모든 단계가 완료되었으며 칩이 잠겼습니다.");
}

/**
 * @brief AES-CCM MAC 계산 (Auth 명령용)
 */
bool calculate_auth_mac(const uint8_t *key, const uint8_t *nonce,
                        const uint8_t *man_id, uint8_t op, uint8_t mode,
                        uint16_t p1, uint16_t p2, uint8_t *out_mac) {
  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);
  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
  if (ret != 0)
    return false;

  // [수정] 배열 크기를 13으로 변경 (데이터시트 Appendix I 규격)
  uint8_t ccm_nonce[13];

  // [수정] 칩 Nonce 12바이트 전체 복사
  memcpy(ccm_nonce, nonce, 12);

  // [수정] 13번째 바이트(인덱스 12)에 MacCount 설정
  ccm_nonce[12] = 0x01;

  uint8_t add[14];
  memset(add, 0, 14);
  add[0] = man_id[0];
  add[1] = man_id[1];
  add[2] = op;
  add[3] = mode;
  add[4] = p1 >> 8;
  add[5] = p1 & 0xFF;
  add[6] = p2 >> 8;
  add[7] = p2 & 0xFF;
  add[8] = 0x03; // MacFlag: Random Nonce + Input MAC

  // [수정] mbedtls 호출 시 nonce 길이를 13으로 전달
  ret = mbedtls_ccm_encrypt_and_tag(&ctx, 0, ccm_nonce, 13, add, 14, NULL, NULL,
                                    out_mac, 16);
  mbedtls_ccm_free(&ctx);
  return (ret == 0);
}
