/**
 * @file main.cpp
 * @brief Example 06: AES132 Encrypt (AES-128)
 *
 * Demonstrates AES-128 encryption using ATAES132A.
 * Requires:
 * 1. Correct ChipConfig (0xC3) -> Enabled by Ex 99.
 * 2. Nonce Generation (Mode 1) -> Implemented here.
 * 3. Valid Key in Slot 0 -> Loaded by Ex 99.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"

// --- Configuration ---
#define KEY_SLOT_ID   0   // Uses Slot 0 (Configured by Tool 99 as 0x0D)
#define BLOCK_SIZE    16

// --- Helper: Encrypt Function ---
// Returns true on success
bool encrypt_block(uint8_t key_id, const uint8_t* plaintext, uint8_t* ciphertext) {
    uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

    // Encrypt OpCode: 0x06, Mode: 0 (Encrypt)
    // Param1: KeyID, Param2: Data Length (16)
    uint8_t ret = aes132m_execute(
        AES132_ENCRYPT, 0,
        key_id, BLOCK_SIZE,
        BLOCK_SIZE, (uint8_t*)plaintext,
        0, NULL, 0, NULL, 0, NULL,
        tx_buf, rx_buf
    );

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        uint8_t count = rx_buf[AES132_RESPONSE_INDEX_COUNT];
        if (count >= 20) { // Count(1) + Status(1) + Data(16) + CRC(2)
            memcpy(ciphertext, &rx_buf[AES132_RESPONSE_INDEX_DATA], BLOCK_SIZE);
            return true;
        }
    }

    Serial.print("[Encrypt] Failed. Code: 0x");
    Serial.println(ret, HEX);
    return false;
}

// --- Helper: Generate Nonce ---
// Required before Encryption in Unlocked/Testing mode
bool generate_nonce() {
    Serial.println("Generating Nonce...");
    uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];
    uint8_t seed[12] = {0}; // Zero seed for Random Mode

    // OpCode: 0x01 (Nonce)
    // Mode: 1 (Random Nonce, incompatible with KeyConfig 0x0D? No, Mode 1 is REQUIRED for 0x0D)
    // Param1: 0, Param2: 0
    uint8_t ret = aes132m_execute(
        0x01, 1, 0, 0,
        12, seed,
        0, NULL, 0, NULL, 0, NULL,
        tx_buf, rx_buf
    );

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        Serial.println("-> Success.");
        return true;
    }
    
    Serial.print("-> Failed: 0x");
    Serial.println(ret, HEX);
    return false;
}

// --- Main Setup ---
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    Serial.println("\n=== Example 06: AES Encryption ===");

    if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.println("I2C Init Failed.");
        return;
    }

    // Diagnostic Check: Verify Encryption is Enabled
    uint8_t tx_debug[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_debug[AES132_RESPONSE_SIZE_MAX];
    if (aes132m_execute(AES132_BLOCK_READ, 0, 0xF040, 4, 0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_debug, rx_debug) == 0) {
        uint8_t conf = rx_debug[AES132_RESPONSE_INDEX_DATA];
        Serial.print("ChipConfig Check (0xF040): 0x");
        Serial.print(conf, HEX);
        if (conf & 0x02) Serial.println(" (Enc/Dec Enabled)");
        else Serial.println(" (WARNING: Enc/Dec DISABLED - Run Ex 99)");
    }

    // 1. Generate Nonce
    if (!generate_nonce()) {
        Serial.println("Critical Error: Nonce generation failed.");
        return;
    }

    // 2. Encryption Test
    Serial.println("\n--- Test 1: Simple Data ---");
    uint8_t plaintext[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint8_t ciphertext[16];

    Serial.print("Plaintext: "); print_hex("", plaintext, 16);

    if (encrypt_block(KEY_SLOT_ID, plaintext, ciphertext)) {
        Serial.print("Ciphertext:"); print_hex("", ciphertext, 16);
        Serial.println("-> Encryption SUCCESS!");
    }

    Serial.println("\n--- Test 2: Text Data ---");
    char text_msg[] = "Hello AES132!   "; // 16 chars
    memcpy(plaintext, text_msg, 16);
    
    Serial.print("Text: \""); Serial.print(text_msg); Serial.println("\"");
    
    if (encrypt_block(KEY_SLOT_ID, plaintext, ciphertext)) {
        Serial.print("Ciphertext:"); print_hex("", ciphertext, 16);
    }

    Serial.println("\nFrom here, you can verify results using Example 7 (Decrypt).");
}

void loop() {
    delay(1000);
}
