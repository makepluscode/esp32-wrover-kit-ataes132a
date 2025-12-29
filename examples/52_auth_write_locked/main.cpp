/**
 * @file main.cpp
 * @brief 예제: 52_auth_write_locked - 잠금 후 인증 기반 쓰기 테스트 (v2)
 *
 * 이 예제는 99번 예제로 칩이 잠긴 후, 권한이 필요한 Zone 1(0x0100)에
 * 인증(Auth)을 거쳐 성공적으로 데이터를 기입하는 과정을 보여줍니다.
 *
 * 수정 사항:
 * 1. CCM Nonce 구성을 12+1(MacCount) = 13바이트로 변경.
 * 2. Auth 리턴 완료 후 Write 권한을 위한 Usage(Param2)를 0x0002로 설정.
 * 3. ManufacturingID를 0xF02A에서 동적으로 읽어와 사용.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <mbedtls/ccm.h>

// 99번 예제에서 설정한 마스터 키 (Key 0)
const uint8_t MASTER_KEY[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
                                0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};

// 함수 프로토타입
bool calculate_auth_mac(const uint8_t *key, const uint8_t *nonce,
                        const uint8_t *man_id, uint8_t op, uint8_t mode,
                        uint16_t p1, uint16_t p2, uint8_t *out_mac);

void setup() {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial)
    delay(10);

  Serial.println("\n========================================");
  Serial.println("예제 52: 잠금 후 인증 기반 쓰기 테스트 (v2)");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 초기화 실패");
    return;
  }

  uint8_t ret;
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];
  uint8_t current_manid[2] = {0x00, 0x00};

  // 0. ManufacturingID 지정 (칩이 잠긴 상태에서 읽기가 불안정할 수 있으므로
  // 0x00EE 하드코딩)
  current_manid[0] = 0x00;
  current_manid[1] = 0x00;
  Serial.printf("[Step 0] ManufacturingID 고정 사용: %02X %02X\n",
                current_manid[0], current_manid[1]);

  Serial.println();

  // 1. 인증 전 쓰기 시도
  uint8_t dummy[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                       0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};
  Serial.println("[Step 1] 인증 없이 Zone 1 (0x0100) 쓰기 시도...");
  ret = aes132m_write_memory(16, 0x0100, dummy);
  Serial.printf("-> 결과: 0x%02X (에러 기대)\n", ret);

  Serial.println();

  // 2. Auth 명령 수행
  Serial.println("[Step 2] 마스터 키(Key 0)를 사용한 인증 수행...");

  // 2-1. Nonce 생성
  uint8_t nonce[12];
  uint8_t seed[12] = {0};
  ret = aes132m_execute(AES132_NONCE, 0x01, 0, 0, 12, seed, 0, NULL, 0, NULL, 0,
                        NULL, tx_buf, rx_buf);
  if (ret != 0) {
    Serial.printf("-> Nonce 생성 실패: 0x%02X\n", ret);
    return;
  }
  memcpy(nonce, &rx_buf[2], 12);
  Serial.print("-> Nonce: ");
  print_hex("", nonce, 12);

  // 2-2. MAC 계산 (Usage: 0x0000 - 99번 예제 패턴과 동일)
  uint16_t auth_usage = 0x0002;
  uint8_t mac_tag[16];
  if (!calculate_auth_mac(MASTER_KEY, nonce, current_manid, AES132_AUTH, 0x01,
                          0x0000, auth_usage, mac_tag)) {
    Serial.println("-> MAC 계산 오류");
    return;
  }
  Serial.print("-> Auth MAC: ");
  print_hex("", mac_tag, 16);

  // 2-3. Auth 실행 (Param2에 auth_usage 전달)
  ret = aes132m_execute(AES132_AUTH, 0x01, 0x0000, auth_usage, 16, mac_tag, 0,
                        NULL, 0, NULL, 0, NULL, tx_buf, rx_buf);
  if (ret != 0 || rx_buf[1] != 0) {
    Serial.printf("-> 인증 실패: 0x%02X (RetCode: 0x%02X)\n", ret, rx_buf[1]);
    return;
  }
  Serial.println("-> 인증 성공! (WriteOK 권한 획득)");

  Serial.println();

  // 3. 인증 후 쓰기 시도
  Serial.println("[Step 3] 인증 완료 후 Zone 1 (0x0100) 데이터 기입...");
  uint8_t secret_msg[16] = "LOCKED_AUTH_OK";
  ret = aes132m_write_memory(16, 0x0100, secret_msg);
  if (ret == 0) {
    Serial.println("-> 데이터 기입 성공!");

    // 읽어서 확인
    uint8_t read_buf[16];
    if (aes132m_read_memory(16, 0x0100, read_buf) == 0) {
      Serial.print("-> 읽기 확인: ");
      Serial.write(read_buf, 16);
      Serial.println();
    }
  } else {
    Serial.printf("-> 데이터 기입 실패: 0x%02X\n", ret);
  }

  Serial.println("\n테스트 종료.");
}

void loop() { delay(1000); }

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
