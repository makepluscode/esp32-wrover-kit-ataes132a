/**
 * @file main.cpp
 * @brief Example 32: Auth-Based Data Access Control (Unlocked State)
 *
 * Demonstrates ATAES132A authentication mechanism:
 * - Configure Zone 1 to require authentication for read/write
 * - Calculate AES-CCM MAC using ESP32's mbedtls library
 * - Execute Auth command and verify protected zone access
 *
 * Key Configuration:
 * - Nonce Mode: Inbound (0x00) - Host seed used directly as nonce
 * - MacFlag: 0x02 (Fixed Nonce + Input MAC)
 * - Param2: 0x0002 (WriteOK permission)
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <mbedtls/ccm.h>

// =============================================================================
// Configuration Addresses
// =============================================================================
#define MANID_ADDR 0xF02B        // ManufacturingID address
#define ZONE1_CONFIG_ADDR 0xF0C4 // ZoneConfig[1] address
#define KEY0_CONFIG_ADDR 0xF080  // KeyConfig[0] address
#define KEY0_MEMORY_ADDR 0xF200  // KeyMemory[0] address
#define ZONE1_DATA_ADDR 0x0100   // Zone 1 data address

// =============================================================================
// Test Key (16 bytes)
// =============================================================================
const uint8_t TEST_KEY[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                              0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// =============================================================================
// AES-CCM MAC Calculation for Auth Command
// =============================================================================
/**
 * @brief Calculate AES-CCM MAC for ATAES132A Auth command
 *
 * @param key      16-byte AES key
 * @param nonce    12-byte nonce from chip
 * @param man_id   2-byte ManufacturingID
 * @param op       Opcode (0x03 for Auth)
 * @param mode     Mode byte
 * @param p1       Param1 (KeyID)
 * @param p2       Param2 (Usage flags)
 * @param out_mac  Output: 16-byte MAC
 * @return true on success, false on failure
 *
 * CCM Nonce Structure (13 bytes):
 *   [0:11] Nonce from chip
 *   [12]   MacCount (0x01)
 *
 * AAD Structure (14 bytes):
 *   [0:1]  ManufacturingID
 *   [2]    Opcode
 *   [3]    Mode
 *   [4:5]  Param1 (Big-Endian)
 *   [6:7]  Param2 (Big-Endian)
 *   [8]    MacFlag (0x02 for Inbound Nonce)
 *   [9:13] Padding (zeros)
 */
bool calculate_auth_mac(const uint8_t *key, const uint8_t *nonce,
                        const uint8_t *man_id, uint8_t op, uint8_t mode,
                        uint16_t p1, uint16_t p2, uint8_t *out_mac) {
  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
  if (ret != 0)
    return false;

  // Build 13-byte CCM Nonce: Nonce[0:11] + MacCount
  uint8_t ccm_nonce[13];
  memcpy(ccm_nonce, nonce, 12);
  ccm_nonce[12] = 0x01; // MacCount = 1

  // Build 14-byte AAD
  uint8_t aad[14] = {0};
  aad[0] = man_id[0];
  aad[1] = man_id[1];
  aad[2] = op;
  aad[3] = mode;
  aad[4] = (uint8_t)(p1 >> 8);
  aad[5] = (uint8_t)(p1 & 0xFF);
  aad[6] = (uint8_t)(p2 >> 8);
  aad[7] = (uint8_t)(p2 & 0xFF);
  aad[8] = 0x02; // MacFlag: Fixed Nonce (Inbound Mode)
  // [9:13] = 0x00 (padding)

  print_hex("-> AES-CCM Nonce (13 bytes): ", ccm_nonce, 13);
  print_hex("-> AES-CCM AAD: ", aad, 14);

  ret = mbedtls_ccm_encrypt_and_tag(&ctx, 0, ccm_nonce, 13, aad, 14, NULL, NULL,
                                    out_mac, 16);
  mbedtls_ccm_free(&ctx);
  return (ret == 0);
}

// =============================================================================
// Main Setup
// =============================================================================
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

  uint8_t ret;
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // =========================================================================
  // Step 0: Read ManufacturingID via BlockRead
  // =========================================================================
  Serial.println("Step 0: ManufacturingID 정확히 읽기 (BlockRead 사용)");

  ret = aes132m_execute(AES132_BLOCK_READ, 0x00, MANID_ADDR, 0x0002, 0, NULL, 0,
                        NULL, 0, NULL, 0, NULL, tx_buf, rx_buf);

  uint8_t man_id[2];
  if (ret == AES132_DEVICE_RETCODE_SUCCESS && rx_buf[1] == 0x00) {
    man_id[0] = rx_buf[2];
    man_id[1] = rx_buf[3];
    Serial.print("-> SUCCESS: Real ManufacturingID is: ");
    print_hex("", man_id, 2);
  } else {
    man_id[0] = 0x00;
    man_id[1] = 0xEE;
    Serial.println("-> FAIL: Using default 00 EE");
  }
  Serial.println();

  // =========================================================================
  // Step 1: Device Configuration
  // =========================================================================
  Serial.println("Step 1: 기기 설정 (BlockWrite 활용)");

  // ZoneConfig[1]: AuthRead=1, AuthWrite=1
  uint8_t zone_conf[4] = {0x03, 0x00, 0x00, 0x55};
  ret = aes132m_write_memory(4, ZONE1_CONFIG_ADDR, zone_conf);
  print_hex("-> Updating ZoneConfig[1] (0xF0C4): ", zone_conf, 4);
  print_result("ZoneConfig Update (BlockWrite)", ret);
  aes132c_wait_for_device_ready();

  // KeyConfig[0]: No restrictions (allows Inbound Nonce)
  uint8_t key_conf[4] = {0x00, 0x00, 0x00, 0x00};
  ret = aes132m_write_memory(4, KEY0_CONFIG_ADDR, key_conf);
  print_hex("-> Updating KeyConfig[0] (0xF080): ", key_conf, 4);
  print_result("KeyConfig Update (BlockWrite)", ret);
  aes132c_wait_for_device_ready();

  // KeyMemory[0]: Inject test key
  ret = aes132m_write_memory(16, KEY0_MEMORY_ADDR, (uint8_t *)TEST_KEY);
  print_hex("-> Writing KeyMemory[0] (0xF200): ", TEST_KEY, 16);
  print_result("KeyMemory Update (BlockWrite)", ret);
  aes132c_wait_for_device_ready();

  // Apply settings via Sleep/Wakeup
  Serial.println("-> 설정 적용을 위해 기기 재시작 (Sleep -> Wakeup)");
  aes132c_sleep();
  delay(100);
  aes132c_wakeup();
  delay(100);
  Serial.println();

  // =========================================================================
  // Step 2: Pre-Auth Write Attempt (Expected to Fail)
  // =========================================================================
  Serial.println("Step 2: 인증 전 Zone 1 쓰기 시도 (실패 예상)");

  uint8_t pre_auth_data[16] = "PRE_AUTH_TEST";
  ret = aes132m_write_memory(16, ZONE1_DATA_ADDR, pre_auth_data);
  print_result("Pre-Auth Write to Zone 1", ret);

  if (ret != 0) {
    Serial.println("-> 예상대로 실패! (인증 필요)");
  } else {
    Serial.println("-> 주의: 쓰기 성공 (ZoneConfig 미적용?)");
  }
  Serial.println();

  // =========================================================================
  // Step 3: Generate Nonce (Inbound Mode)
  // =========================================================================
  Serial.println("Step 3: Nonce 생성 (Inbound Mode - 0x00)");

  uint8_t seed[12] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                      0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};

  ret = aes132m_execute(AES132_NONCE, 0x00, 0, 0, 12, seed, 0, NULL, 0, NULL, 0,
                        NULL, tx_buf, rx_buf);

  if (ret != AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.printf("Nonce Generation Failed: 0x%02X\n", ret);
    return;
  }
  print_hex("-> Inbound Nonce (=seed): ", seed, 12);
  Serial.println();

  // =========================================================================
  // Step 4: Calculate MAC and Execute Auth
  // =========================================================================
  Serial.println(
      "Step 4: MAC 계산 (Inbound Nonce + Param2=0x0003 Read+Write) 및 "
      "Auth 명령 실행");

  uint8_t mac[16];
  if (!calculate_auth_mac(TEST_KEY, seed, man_id, AES132_AUTH, 0x01, 0x0000,
                          0x0003, mac)) { // 0x0003 = ReadOK + WriteOK
    Serial.println("MAC Calculation Failed");
    return;
  }
  print_hex("-> Host Calculated MAC (Param2=0x0003): ", mac, 16);

  // Execute Auth command with ReadOK + WriteOK permission
  ret = aes132m_execute(AES132_AUTH, 0x01, 0x0000, 0x0003, 16, mac, 0, NULL, 0,
                        NULL, 0, NULL, tx_buf, rx_buf);
  print_result("Authentication (Opcode 0x03, Param2=WriteOK)", ret);

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

  // =========================================================================
  // Step 5: Verify Zone 1 Access After Auth (BlockWrite/BlockRead)
  // =========================================================================
  Serial.println("Step 5: 인증 후 Zone 1 데이터 접근성 검증");

  // ZoneConfig.EncWrite=0 means BlockWrite can be used after Auth
  uint8_t write_data[16] = "AUTH_OK";
  print_hex("-> Write Attempt Data: ", write_data, 16);

  ret = aes132m_write_memory(16, ZONE1_DATA_ADDR, write_data);
  print_result("BlockWrite to Zone 1", ret);

  // Read back and verify
  uint8_t read_data[16] = {0};
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

    // Verify written data matches
    if (memcmp(write_data, read_data, 16) == 0) {
      Serial.println("-> DATA VERIFIED: Write/Read match!");
    } else {
      Serial.println("-> Warning: Data mismatch");
    }
  } else {
    Serial.printf("Read failed (ReturnCode: 0x%02X)\n", ret);
  }

  Serial.println("\n=== Example 32 Complete ===");
}

void loop() { delay(1000); }
