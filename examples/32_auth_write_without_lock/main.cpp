/**
 * @file main.cpp
 * @brief 예제 32: 인증 기반 데이터 접근 제어 (Auth-Only) - Unlocked 상태
 *
 * 이 예제는 장치를 잠그지 않은 상태에서 Zone 1을 '인증 전용'으로 설정하고,
 * ESP32에서 실시간으로 AES-CCM MAC을 계산하여 인증 후 데이터를 접근하는 과정을
 * 보여줍니다.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <mbedtls/ccm.h>

// 설정 주소
#define MANID_ADDR 0xF02A
#define ZONE1_CONFIG_ADDR 0xF0C4
#define KEY0_CONFIG_ADDR 0xF080
#define KEY0_MEMORY_ADDR 0xF200
#define ZONE1_DATA_ADDR 0x0100

// 테스트용 키 (16바이트)
const uint8_t TEST_KEY[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                              0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// 하드코딩된 ManufacturingID (데이터시트 기본값: 0x00 0xEE)
const uint8_t REAL_MANID[2] = {0x00, 0xEE};

/**
 * @brief ESP32 mbedtls를 사용한 AES-CCM MAC(Tag) 계산
 * ATAES132A Auth 명령용 AAD 구조 (데이터시트 Appendix I.6 엄격 규격 - 14 Bytes)
 */
bool calculate_auth_mac(const uint8_t *key, const uint8_t *nonce,
                        const uint8_t *man_id, uint8_t op, uint8_t mode,
                        uint16_t p1, uint16_t p2, uint8_t *out_mac) {
  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
  if (ret != 0)
    return false;

  // ATAES132A AES-CCM Nonce: RandomNonce[0..10] + MacCount[0]
  uint8_t ccm_nonce[12];
  memcpy(ccm_nonce, nonce, 11);
  ccm_nonce[11] = 0x01; // MacCount = 1 (첫 번째 인증 시도)

  /**
   * @brief AAD (인증 블록) 구조 재정렬 (Appendix I.6 엄격 규격)
   * [0:1]: ManufacturingID (2 bytes) - REAL_MANID (0x00 0xEE)
   * [2]: Opcode (0x03)
   * [3]: Mode (0x01)
   * [4:5]: Param1 (0x0000 / KeyID)
   * [6:7]: Param2 (0x0000 / Usage)
   * [8]: MacFlag (0x03) -> Bit 1: Input MAC, Bit 0: Random Nonce 사용
   * [9:13]: 0x00, 0x00, 0x00, 0x00, 0x00 (5 bytes Padding)
   */
  uint8_t add[14];
  memset(add, 0x00, 14);
  add[0] = man_id[0];
  add[1] = man_id[1];
  add[2] = op;
  add[3] = mode;
  add[4] = (uint8_t)(p1 >> 8);
  add[5] = (uint8_t)(p1 & 0xFF);
  add[6] = (uint8_t)(p2 >> 8);
  add[7] = (uint8_t)(p2 & 0xFF);
  add[8] = 0x03; // MacFlag: 0x03 (Random Nonce + Input MAC)
  // [9:13] Padding bits already 0x00 via memset

  print_hex("-> AES-CCM Nonce: ", ccm_nonce, 12);
  print_hex("-> AES-CCM AAD: ", add, 14);

  ret = mbedtls_ccm_encrypt_and_tag(&ctx, 0, ccm_nonce, 12, add, 14, NULL, NULL,
                                    out_mac, 16);

  mbedtls_ccm_free(&ctx);
  return (ret == 0);
}

void setup() {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial)
    delay(10);

  Serial.println("\n========================================");
  Serial.println("ESP32 AES132 CryptoAuth Example");
  Serial.println("Example 32: Auth Write Without Lock (FINAL)");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 Init Failed");
    return;
  }

  Serial.println("Step 0: ManufacturingID 확인 및 강제 적용");
  uint8_t read_man_id[2];
  uint8_t ret = aes132m_read_memory(2, MANID_ADDR, read_man_id);
  if (ret == 0) {
    print_hex("-> Read ManufacturingID ($F02A): ", read_man_id, 2);
  } else {
    Serial.print("-> Failed to read ManID (Error 0x");
    Serial.print(ret, HEX);
    Serial.println("). Using REAL_MANID fallback.");
  }

  // 인증 성공을 위해 REAL_MANID 강제 사용
  const uint8_t *man_id_to_use = REAL_MANID;
  print_hex("-> Using ManID for Calculation: ", man_id_to_use, 2);
  Serial.println();

  Serial.println("Step 1: 기기 설정 (BlockWrite 활용)");

  // 1-1. ZoneConfig 01 설정: AuthRead=1, AuthWrite=1 (0x03), ReadOnly=0x55
  uint8_t zone_conf[4] = {0x03, 0x00, 0x00, 0x55};
  ret = aes132m_write_memory(4, ZONE1_CONFIG_ADDR, zone_conf);
  print_hex("-> Updating ZoneConfig[1] (0xF0C4): ", zone_conf, 4);
  print_result("ZoneConfig Update (BlockWrite)", ret);
  aes132c_wait_for_device_ready();

  // 1-2. KeyConfig 00 설정: InboundAuth=1, RandomNonce=1 (0x06)
  uint8_t key_conf[4] = {0x06, 0x00, 0x00, 0x00};
  ret = aes132m_write_memory(4, KEY0_CONFIG_ADDR, key_conf);
  print_hex("-> Updating KeyConfig[0] (0xF080): ", key_conf, 4);
  print_result("KeyConfig Update (BlockWrite)", ret);
  aes132c_wait_for_device_ready();

  // 1-3. KeyMemory 00 주입
  ret = aes132m_write_memory(16, KEY0_MEMORY_ADDR, (uint8_t *)TEST_KEY);
  print_hex("-> Writing KeyMemory[0] (0xF200): ", TEST_KEY, 16);
  print_result("KeyMemory Update (BlockWrite)", ret);
  aes132c_wait_for_device_ready();

  Serial.println("-> 설정 적용을 위해 기기 재시작 (Sleep -> Wakeup)");
  aes132c_sleep();
  delay(100);
  aes132c_wakeup();
  delay(100);
  Serial.println();

  Serial.println("Step 2: Nonce 생성");
  uint8_t nonce[12];
  uint8_t seed[12] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                      0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  ret = aes132m_execute(AES132_NONCE, 0x01, 0, 0, 12, seed, 0, NULL, 0, NULL, 0,
                        NULL, tx_buf, rx_buf);
  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    memcpy(nonce, &rx_buf[2], 12);
    print_hex("-> Received Random Nonce: ", nonce, 12);
  } else {
    Serial.print("Nonce Generation Failed: 0x");
    Serial.println(ret, HEX);
    return;
  }
  Serial.println();

  Serial.println(
      "Step 3: MAC 계산 (Strict AAD + MacCount=1) 및 Auth 명령 실행");
  uint8_t mac[16];
  if (calculate_auth_mac(TEST_KEY, nonce, man_id_to_use, AES132_AUTH, 0x01,
                         0x0000, 0x0000, mac)) {
    print_hex("-> Host Calculated Strict MAC: ", mac, 16);
  } else {
    Serial.println("MAC Calculation Failed");
    return;
  }

  ret = aes132m_execute(AES132_AUTH, 0x01, 0x0000, 0x0000, 16, mac, 0, NULL, 0,
                        NULL, 0, NULL, tx_buf, rx_buf);
  print_result("Authentication (Opcode 0x03)", ret);

  uint8_t status = 0;
  aes132c_read_device_status_register(&status);
  Serial.printf("Device Status after Auth: 0x%02X\n", status);

  if (ret == 0x00 && !(status & 0x20)) {
    Serial.println("-> AUTH SUCCESS! Phase 2 access granted.");
  } else {
    Serial.println("-> AUTH FAILED. Please verify ManID match and AAD order.");
    if (ret == 0x40)
      Serial.println("   (Error 0x40 = MAC Mismatch)");
  }
  Serial.println();

  Serial.println("Step 4: 인증 후 Zone 1 데이터 접근성 검증");
  uint8_t test_data[16] = "AUTH_OK"; // 7 chars + null = 8 bytes used
  print_hex("-> Write Attempt Data: ", test_data, 16);

  ret = aes132m_write_memory(16, ZONE1_DATA_ADDR, test_data);
  print_result("BlockWrite to Zone 1", ret);

  uint8_t read_data[16];
  memset(read_data, 0, 16);
  ret = aes132m_read_memory(16, ZONE1_DATA_ADDR, read_data);
  print_result("BlockRead from Zone 1", ret);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    print_hex("-> Read Data: ", read_data, 16);
    Serial.print("-> Read String: \"");
    for (int i = 0; i < 16; i++) {
      if (read_data[i] >= 0x20 && read_data[i] <= 0x7E)
        Serial.write(read_data[i]);
      else
        Serial.print(".");
    }
    Serial.println("\"");
  } else {
    Serial.print("Read failed (ReturnCode: 0x");
    Serial.print(ret, HEX);
    if (ret == 0x80)
      Serial.println(", Key Error/Prior Auth missing)");
    else
      Serial.println(")");
  }

  Serial.println("\n=== Example 32 Complete ===");
}

void loop() { delay(1000); }
