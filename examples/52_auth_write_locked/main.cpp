/**
 * @file main.cpp
 * @brief Example 52: Auth-Based Write (Locked Chip)
 *
 * Demonstrates ATAES132A authentication mechanism on a LOCKED chip:
 * - Read ManufacturingID via BlockRead
 * - Generate Nonce (Inbound Mode - 0x00)
 * - Calculate AES-CCM MAC with correct 13-byte nonce
 * - Execute Auth command with ReadOK + WriteOK permissions
 * - Verify Zone 1 access after authentication
 *
 * Prerequisites:
 * - Chip must be locked using Example 99
 * - ZoneConfig[1] must have AuthRead=1, AuthWrite=1
 * - KeyMemory[0] must contain MASTER_KEY
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <mbedtls/ccm.h>

// =============================================================================
// Configuration
// =============================================================================
#define MANID_ADDR 0xF02B      // ManufacturingID address (MSB first)
#define ZONE1_DATA_ADDR 0x0100 // Zone 1 data address

// Master Keys to try (Handle already locked chips with old keys)
const uint8_t MASTER_KEY_NEW[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                    0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB,
                                    0xCC, 0xDD, 0xEE, 0xFF};
const uint8_t MASTER_KEY_OLD[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5,
                                    0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
                                    0xAC, 0xAD, 0xAE, 0xAF};

// =============================================================================
// AES-CCM MAC Calculation
// =============================================================================
/**
 * @brief Calculate AES-CCM MAC for Auth command
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
  Serial.println("Example 52: Auth-Based Write (Locked Chip)");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 Init Failed");
    return;
  }

  uint8_t ret;
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // =========================================================================
  // Step 0: Read ManufacturingID
  // =========================================================================
  Serial.println("Step 0: ManufacturingID 읽기 (BlockRead)");

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
  // Step 1: Pre-Auth Write Attempt (Expected to Fail)
  // =========================================================================
  Serial.println("Step 1: 인증 전 Zone 1 쓰기 시도 (실패 예상)");

  uint8_t dummy[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                       0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};
  ret = aes132m_write_memory(16, ZONE1_DATA_ADDR, dummy);
  print_result("Pre-Auth Write to Zone 1", ret);
  if (ret != 0) {
    Serial.println("-> 예상대로 실패! (인증 필요)");
  }
  Serial.println();

  // =========================================================================
  // Step 2: Generate Nonce (Inbound Mode - 0x00)
  // =========================================================================
  Serial.println("Step 2: Nonce 생성 (Inbound Mode - 0x00)");

  uint8_t seed[12] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                      0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
  uint8_t nonce[12];
  memcpy(nonce, seed, 12); // User solution: Use seed directly as nonce

  ret = aes132m_execute(AES132_NONCE, 0x00, 0, 0, 12, seed, 0, NULL, 0, NULL, 0,
                        NULL, tx_buf, rx_buf);

  if (ret != AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.printf("Nonce Generation Failed: 0x%02X\n", ret);
    return;
  }
  print_hex("-> Inbound Nonce (=seed): ", nonce, 12);
  Serial.println();

  // =========================================================================
  // Step 3: Calculate MAC and Execute Auth
  // =========================================================================
  Serial.println("Step 3: MAC 계산 및 Auth 명령 실행");

  uint8_t mac[16];
  const uint8_t *keys_to_try[] = {MASTER_KEY_NEW, MASTER_KEY_OLD};
  const char *key_names[] = {"MASTER_KEY_NEW (0x00...)",
                             "MASTER_KEY_OLD (0xA0...)"};
  bool auth_success = false;

  for (int i = 0; i < 2; i++) {
    Serial.printf("-> Attempting Auth with %s\n", key_names[i]);

    // Param2 = 0x0003 (ReadOK + WriteOK), MacFlag = 0x02 (Fixed Nonce)
    if (!calculate_auth_mac(keys_to_try[i], nonce, man_id, AES132_AUTH, 0x01,
                            0x0000, 0x0003, mac)) {
      Serial.println("MAC Calculation Failed");
      continue;
    }
    print_hex("-> Host Calculated MAC: ", mac, 16);

    // Execute Auth command
    ret = aes132m_execute(AES132_AUTH, 0x01, 0x0000, 0x0003, 16, mac, 0, NULL,
                          0, NULL, 0, NULL, tx_buf, rx_buf);

    if (ret == 0x00 && rx_buf[1] == 0x00) {
      auth_success = true;
      Serial.println("[Authentication] SUCCESS");
      break;
    } else {
      Serial.printf("-> Auth Attempt Failed: 0x%02X (Device Ret: 0x%02X)\n",
                    ret, rx_buf[1]);
      if (ret != 0x40 && rx_buf[1] != 0x40) {
        // If not a MAC error, something else is wrong, stop trying keys
        break;
      }
    }
  }

  uint8_t status = 0;
  aes132c_read_device_status_register(&status);
  Serial.printf("Device Status: 0x%02X\n", status);

  if (auth_success &&
      !(status &
        (1 << 5))) { // Bit 5 is Auth fail bit or similar in some status regs
    Serial.println("-> AUTH SUCCESS! Read+Write access granted.");
  } else if (!auth_success) {
    Serial.println("-> ALL AUTH ATTEMPTS FAILED.");
    return;
  }
  Serial.println();

  // =========================================================================
  // Step 4: Post-Auth Write and Verify
  // =========================================================================
  Serial.println("Step 4: 인증 후 Zone 1 쓰기/읽기 검증");

  uint8_t write_data[16] = "LOCKED_AUTH_OK";
  print_hex("-> Write Data: ", write_data, 16);

  ret = aes132m_write_memory(16, ZONE1_DATA_ADDR, write_data);
  print_result("BlockWrite to Zone 1", ret);

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

    if (memcmp(write_data, read_data, 16) == 0) {
      Serial.println("-> DATA VERIFIED: Write/Read match!");
    } else {
      Serial.println("-> Warning: Data mismatch");
    }
  }

  Serial.println("\n=== Example 52 Complete ===");
}

void loop() { delay(1000); }
