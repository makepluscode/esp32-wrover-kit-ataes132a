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

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"

// --- Configuration ---
#define KEY_SLOT_ID   0   // Uses Slot 0
#define BLOCK_SIZE    16

// --- Helper: Generate Nonce ---
bool generate_nonce() {
    Serial.println("Generating Nonce...");
    uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];
    uint8_t seed[12] = {0}; 

    uint8_t ret = aes132m_execute(
        AES132_NONCE, 1, 0, 0,
        12, seed, 0, NULL, 0, NULL, 0, NULL,
        tx_buf, rx_buf
    );

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        Serial.println("-> Nonce Success.");
        return true;
    }
    Serial.print("-> Nonce Failed: 0x"); Serial.println(ret, HEX);
    return false;
}

// --- Helper: Encrypt Block (Needed for Self-Test) ---
bool encrypt_block(uint8_t key_id, const uint8_t* plaintext, uint8_t* out_mac, uint8_t* out_ciphertext) {
    uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

    uint8_t ret = aes132m_execute(
        AES132_ENCRYPT, 0, key_id, BLOCK_SIZE,
        BLOCK_SIZE, (uint8_t*)plaintext,
        0, NULL, 0, NULL, 0, NULL,
        tx_buf, rx_buf
    );

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
bool decrypt_block(uint8_t key_id, const uint8_t* mac, const uint8_t* ciphertext, uint8_t* out_plaintext) {
    uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

    // Decrypt Command
    // Data 1: MAC (16 bytes)
    // Data 2: Ciphertext (16 bytes)
    // Param 2: 16 (Result Length)
    uint8_t ret = aes132m_execute(
        AES132_DECRYPT, 0, key_id, BLOCK_SIZE,
        BLOCK_SIZE, (uint8_t*)mac,          // Data 1: Input MAC
        BLOCK_SIZE, (uint8_t*)ciphertext,   // Data 2: Input Ciphertext
        0, NULL, 0, NULL,
        tx_buf, rx_buf
    );

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
    while (!Serial) delay(10);

    Serial.println("\n=== Example 07: AES Decryption (AES-CCM) ===");

    if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.println("I2C Init Failed.");
        return;
    }

    // 1. Generate Nonce (Essential)
    if (!generate_nonce()) return;

    // 2. Prepare Data (Encrypt First to get valid MAC/Cipher)
    Serial.println("\n--- Step 1: Encrypting Data internally for test ---");
    uint8_t original[16] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x41, 0x45, 0x53, 0x31, 0x33, 0x32, 0x21, 0x00, 0x00, 0x00}; // "Hello AES132!..."
    uint8_t mac[16];
    uint8_t ciphertext[16];
    uint8_t decrypted[16];

    Serial.print("Original: "); print_hex("", original, 16);

    if (!encrypt_block(KEY_SLOT_ID, original, mac, ciphertext)) {
        Serial.println("Encrypt failed. Aborting.");
        return;
    }

    Serial.print("Generated MAC:    "); print_hex("", mac, 16);
    Serial.print("Generated Cipher: "); print_hex("", ciphertext, 16);

    // 3. Decrypt Data
    Serial.println("\n--- Step 2: Decrypting Data ---");
    Serial.println("Sending Decrypt Command (MAC + Cipher)...");

    if (decrypt_block(KEY_SLOT_ID, mac, ciphertext, decrypted)) {
        Serial.println("-> Decrypt SUCCESS!");
        Serial.print("Decrypted: "); print_hex("", decrypted, 16);

        // Verify
        if (memcmp(original, decrypted, 16) == 0) {
             Serial.println("-> VERIFICATION PASS: Data matches original.");
        } else {
             Serial.println("-> VERIFICATION FAIL: Data mismatch.");
        }
    } else {
        Serial.println("-> Decrypt FAILED.");
    }
}

void loop() {
    delay(1000);
}
