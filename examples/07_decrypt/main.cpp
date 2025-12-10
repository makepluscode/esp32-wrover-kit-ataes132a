/**
 * @file main.cpp
 * @brief 예제 7: AES132 복호화 (Decrypt)
 * 
 * 이 예제는 AES132 디바이스의 AES-128 복호화 기능을 사용하는 방법을 보여줍니다.
 * Decrypt 명령어를 사용하여 암호문을 평문으로 복호화합니다.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"
#include "aes132_config.h"
#include <string.h> // for memcpy, memcmp

/**
 * @brief Decrypt 명령어를 사용하여 데이터 복호화
 * 
 * Decrypt 명령어는 AES-128 복호화를 수행합니다.
 * 16바이트 블록 단위로 복호화됩니다.
 * 
 * @param key_id 사용할 키 슬롯 ID (0-15, 암호화에 사용한 것과 동일해야 함)
 * @param ciphertext 복호화할 암호문 데이터 (16바이트)
 * @param plaintext 복호화된 평문을 저장할 버퍼 (16바이트)
 * @return 1 성공 시, 0 실패 시
 */
uint8_t decrypt_data(uint8_t key_id, const uint8_t* ciphertext, uint8_t* plaintext) {
    if (key_id > 15) {
        Serial.println("Error: Key ID must be between 0 and 15.");
        return 0;
    }

    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // Decrypt 명령어 파라미터 구성
    // Mode: 0 (일반 복호화)
    // Param1: Key ID (0-15, 암호화에 사용한 것과 동일)
    // Param2: 0 (사용 안 함)
    // Data1: 암호문 데이터 (16바이트)
    uint16_t param1 = key_id;
    uint16_t param2 = 0;

    // Decrypt 명령어 실행
    uint8_t ret = aes132m_execute(
        AES132_DECRYPT,     // Op-Code: 0x07
        0,                  // Mode: 0 (일반 복호화)
        param1,             // Parameter 1: Key ID
        param2,             // Parameter 2: 0
        16, (uint8_t*)ciphertext,  // Data 1: 암호문 데이터 (16바이트)
        0, NULL,            // Data 2: 없음
        0, NULL,            // Data 3: 없음
        0, NULL,            // Data 4: 없음
        tx_buffer,
        rx_buffer
    );

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        // 응답에서 복호화된 데이터 추출
        // rx_buffer[0]: Count
        // rx_buffer[1]: Status (Return Code)
        // rx_buffer[2-17]: 복호화된 평문 데이터 (16바이트)
        uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
        
        // Count가 최소 20이어야 함 (Count + Status + Data(16) + Checksum(2))
        if (count >= 20) {
            // 복호화된 데이터 복사
            memcpy(plaintext, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
            return 1; // 성공
        } else {
            Serial.print("Error: Invalid response count: ");
            Serial.print(count);
            Serial.print(" (expected >= 20)");
            Serial.println();
        }
    } else {
        Serial.print("Error: Decrypt command failed with code 0x");
        Serial.println(ret, HEX);
    }

    return 0; // 실패
}

void setup(void) {
    Serial.begin(AES132_SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n========================================");
    Serial.println("ESP32 AES132 CryptoAuth Example");
    Serial.println("Example 07: Decryption (AES-128)");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }

    Serial.println("AES132 initialized successfully\n");

    // 주의: 실제 복호화를 위해서는 키가 키 슬롯에 로드되어 있어야 합니다
    // 이 예제는 키 슬롯 0에 키가 이미 로드되어 있다고 가정합니다
    Serial.println("Note: This example assumes a key is already loaded in Key Slot 0.");
    Serial.println("      Use KeyLoad command (Example 8) to load a key first.\n");

    // 예제 1: 암호문 복호화
    Serial.println("=== Example 1: Decrypt Ciphertext ===");
    // 이 암호문은 예제 6에서 암호화된 것으로 가정합니다
    // 실제로는 예제 6의 출력을 사용해야 합니다
    uint8_t ciphertext1[16] = {
        0x3A, 0xD7, 0x7B, 0xB4, 0x0D, 0x7A, 0x36, 0x60,
        0xA8, 0x9E, 0xCA, 0xF3, 0x24, 0x66, 0xEF, 0x5A
    };
    uint8_t plaintext1[16] = {0};

    Serial.println("Ciphertext (16 bytes):");
    print_hex("  ", ciphertext1, 16);
    Serial.println();

    Serial.println("Decrypting with Key Slot 0...");
    uint8_t decrypt_ret = decrypt_data(0, ciphertext1, plaintext1);
    print_result("Decrypt", decrypt_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);

    if (decrypt_ret) {
        Serial.println("Plaintext (16 bytes):");
        print_hex("  ", plaintext1, 16);
        Serial.println();
    }
    Serial.println();

    // 예제 2: 암호화-복호화 전체 사이클
    Serial.println("=== Example 2: Encrypt-Decrypt Cycle ===");
    uint8_t original_plaintext[16] = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x41, 0x45,
        0x53, 0x31, 0x33, 0x32, 0x21, 0x00, 0x00, 0x00
    };
    uint8_t encrypted[16] = {0};
    uint8_t decrypted[16] = {0};

    Serial.println("Step 1: Encrypt plaintext");
    print_hex("Original plaintext: ", original_plaintext, 16);
    
    // 암호화 (Encrypt 명령어 사용)
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];
    uint8_t encrypt_ret = aes132m_execute(
        AES132_ENCRYPT, 0, 0, 0,
        16, original_plaintext, 0, NULL, 0, NULL, 0, NULL,
        tx_buffer, rx_buffer
    );
    
    if (encrypt_ret == AES132_DEVICE_RETCODE_SUCCESS) {
        uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
        if (count >= 20) {
            memcpy(encrypted, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
            print_hex("Encrypted ciphertext: ", encrypted, 16);
            Serial.println();
        }
    } else {
        Serial.print("Encryption failed: 0x");
        Serial.println(encrypt_ret, HEX);
        Serial.println("Skipping decryption...\n");
        return;
    }

    Serial.println("Step 2: Decrypt ciphertext");
    print_hex("Ciphertext to decrypt: ", encrypted, 16);
    
    decrypt_ret = decrypt_data(0, encrypted, decrypted);
    print_result("Decrypt", decrypt_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);

    if (decrypt_ret) {
        print_hex("Decrypted plaintext: ", decrypted, 16);
        Serial.println();

        // 원본과 복호화된 데이터 비교
        Serial.println("Step 3: Verify decryption");
        if (memcmp(original_plaintext, decrypted, 16) == 0) {
            Serial.println("✓ Verification successful! Decrypted data matches original.");
        } else {
            Serial.println("✗ Verification failed! Decrypted data does not match original.");
            Serial.println("Differences:");
            for (uint8_t i = 0; i < 16; i++) {
                if (original_plaintext[i] != decrypted[i]) {
                    Serial.print("  Byte ");
                    Serial.print(i);
                    Serial.print(": Original=0x");
                    Serial.print(original_plaintext[i], HEX);
                    Serial.print(", Decrypted=0x");
                    Serial.println(decrypted[i], HEX);
                }
            }
        }
    }
    Serial.println();

    // 예제 3: 여러 블록 복호화
    Serial.println("=== Example 3: Decrypt Multiple Blocks ===");
    Serial.println("Note: AES-128 decrypts one 16-byte block at a time.");
    Serial.println("      For multiple blocks, call Decrypt multiple times.\n");

    uint8_t ciphertext3_block1[16] = {
        0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
        0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
    };
    uint8_t ciphertext3_block2[16] = {
        0x4A, 0x5B, 0x2E, 0x1F, 0x3D, 0x6A, 0x8B, 0x4C,
        0x7D, 0x9E, 0x2F, 0x1A, 0x3B, 0x4C, 0x5D, 0x6E
    };
    uint8_t plaintext3_block1[16] = {0};
    uint8_t plaintext3_block2[16] = {0};

    Serial.println("Block 1:");
    print_hex("  Ciphertext: ", ciphertext3_block1, 16);
    decrypt_ret = decrypt_data(0, ciphertext3_block1, plaintext3_block1);
    if (decrypt_ret) {
        print_hex("  Plaintext:  ", plaintext3_block1, 16);
    }
    Serial.println();

    Serial.println("Block 2:");
    print_hex("  Ciphertext: ", ciphertext3_block2, 16);
    decrypt_ret = decrypt_data(0, ciphertext3_block2, plaintext3_block2);
    if (decrypt_ret) {
        print_hex("  Plaintext:  ", plaintext3_block2, 16);
    }
    Serial.println();

    // 예제 4: 복호화 특성 분석
    Serial.println("=== Example 4: Decryption Analysis ===");
    Serial.println("Characteristics of AES decryption:");
    Serial.println("1. Decryption is the inverse operation of encryption");
    Serial.println("2. Same ciphertext and key always produce same plaintext");
    Serial.println("3. Decryption restores the original data exactly");
    Serial.println("4. Wrong key produces garbage data (not original plaintext)");
    Serial.println();

    Serial.println("\n=== Decryption Examples Complete ===");
    Serial.println("\nNote: Decryption requires the same key used for encryption.");
    Serial.println("      Key must be loaded in the same key slot.");
}

void loop(void) {
    delay(1000);
}

