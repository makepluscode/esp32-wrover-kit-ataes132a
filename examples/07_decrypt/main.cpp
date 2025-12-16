/**
 * @file main.cpp
 * @brief 예제 7: AES132 복호화 (Decrypt) - AES-CCM Mode Fixed
 *
 * AES-CCM 모드 복호화 예제입니다.
 * 복호화를 위해서는 암호화 시 생성된 MAC(Tag)과 Ciphertext가 모두 필요합니다.
 *
 * Flow:
 * 1. Setup Chip (Init)
 * 2. Generate Nonce (Random)
 * 3. Encrypt Data (Internal or Previous) -> Get MAC & Ciphertext
 * 4. Decrypt Data -> Verify Plaintext
 */

#include "aes132_comm_marshaling.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>

// --- Configuration ---
#define KEY_SLOT_ID 0 // Uses Slot 0
#define BLOCK_SIZE 16

// --- Global: Stored Nonce (Buffer) ---
// Step 1에서 사용한 Nonce를 저장해 두었다가 Step 2에서 재사용하기 위한
// 버퍼입니다. 암호화 시 Nonce가 무효화되므로, 복호화 전에 반드시 동일한 Nonce를
// 다시 로드해야 합니다.
uint8_t stored_nonce[12];

// --- Helper: Load Nonce (Fixed Mode 0) ---
// 저장된(stored_nonce) 값을 Chip의 TempKey로 로드합니다.
bool load_nonce_fixed() {
  Serial.println("Loading Stored Nonce (Fixed Mode)...");
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // OpCode: 0x01 (Nonce), Mode: 0 (Fixed), Param1: 0 (Update Seed), Param2: 0
  uint8_t ret = aes132m_execute(AES132_NONCE, 0, 0, 0, 12, stored_nonce, 0,
                                NULL, 0, NULL, 0, NULL, tx_buf, rx_buf);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.println("-> Nonce Load Success.");
    return true;
  }
  Serial.print("-> Nonce Load Failed: 0x");
  Serial.println(ret, HEX);
  return false;
}

// --- Helper: Encrypt Block (Needed for Self-Test) ---
bool encrypt_block(uint8_t key_id, const uint8_t *plaintext, uint8_t *out_mac,
                   uint8_t *out_ciphertext) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  uint8_t ret = aes132m_execute(AES132_ENCRYPT, 0, key_id, BLOCK_SIZE,
                                BLOCK_SIZE, (uint8_t *)plaintext, 0, NULL, 0,
                                NULL, 0, NULL, tx_buf, rx_buf);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    // [Count][Status][MAC(16)][Cipher(16)][CRC]
    memcpy(out_mac, &rx_buf[AES132_RESPONSE_INDEX_DATA], 16);
    memcpy(out_ciphertext, &rx_buf[AES132_RESPONSE_INDEX_DATA + 16], 16);
    return true;
  }
  return false;
}

// --- Helper: Decrypt Block (Corrected for AES-CCM) ---
// Requires: KeyID, MAC(16), Ciphertext(16)
// Returns: Plaintext(16)
bool decrypt_block(uint8_t key_id, const uint8_t *mac,
                   const uint8_t *ciphertext, uint8_t *out_plaintext) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // Decrypt Command
  // Data 1: Ciphertext (16 bytes)
  // Data 2: MAC (16 bytes)
  // Param 2: 16 (Result Length)
  uint8_t ret =
      aes132m_execute(AES132_DECRYPT, 0, key_id, BLOCK_SIZE, BLOCK_SIZE,
                      (uint8_t *)ciphertext,      // Data 1: Input Ciphertext
                      BLOCK_SIZE, (uint8_t *)mac, // Data 2: Input MAC
                      0, NULL, 0, NULL, tx_buf, rx_buf);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    uint8_t count = rx_buf[AES132_RESPONSE_INDEX_COUNT];
    if (count >= 20) { // Count(1)+Status(1)+Data(16)+CRC(2)
      memcpy(out_plaintext, &rx_buf[AES132_RESPONSE_INDEX_DATA], 16);
      return true;
    }
  }

  Serial.print("[Decrypt] Failed. Code: 0x");
  Serial.println(ret, HEX);
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n=== Example 07: AES Decryption (AES-CCM) ===");

  // Force Wakeup
  i2c_enable_phys();
  aes132c_wakeup();
  delay(200);

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("Warning: Init failed, retrying...");
    aes132c_wakeup();
    if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
      Serial.println("Error: I2C Init Failed.");
      return;
    }
  }

  // 0. Prepare Stored Nonce (Fixed Pattern)
  for (int i = 0; i < 12; i++)
    stored_nonce[i] = i;

  // 1. Load Nonce for Encryption
  if (!load_nonce_fixed())
    return;

  // 2. Encrypt
  Serial.println("\n--- Step 1: Encrypting Data ---");
  uint8_t original[16] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x41, 0x45,
                          0x53, 0x31, 0x33, 0x32, 0x21, 0x00, 0x00, 0x00};
  uint8_t mac[16];
  uint8_t ciphertext[16];
  uint8_t decrypted[16];

  Serial.print("Original: ");
  print_hex("", original, 16);

  if (!encrypt_block(KEY_SLOT_ID, original, mac, ciphertext)) {
    Serial.println("Encrypt failed.");
    return;
  }
  Serial.print("MAC:    ");
  print_hex("", mac, 16);
  Serial.print("Cipher: ");
  print_hex("", ciphertext, 16);

  // 3. Load Nonce for Decryption (MUST be same as above)
  Serial.println("\n--- Step 2: Decrypting Data ---");
  // [CRITICAL] Encrypt invalidated the nonce. Reload it.
  if (!load_nonce_fixed())
    return;

  Serial.println("Sending Decrypt Command (Cipher + MAC)...");

  // Data Order: Cipher then MAC (per User Instruction)
  if (decrypt_block(KEY_SLOT_ID, mac, ciphertext, decrypted)) {
    Serial.println("-> Decrypt SUCCESS!");
    Serial.print("Decrypted: ");
    print_hex("", decrypted, 16);
    if (memcmp(original, decrypted, 16) == 0)
      Serial.println("MATCH!");
    else
      Serial.println("MISMATCH!");
  } else {
    Serial.println("-> Decrypt FAILED.");
  }
}

void loop() { delay(1000); }
