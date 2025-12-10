/**
 * @file main.cpp
 * @brief 예제 6: AES132 암호화 (Encrypt)
 * 
 * 이 예제는 AES132 디바이스의 AES-128 암호화 기능을 사용하는 방법을 보여줍니다.
 * Encrypt 명령어를 사용하여 평문 데이터를 암호화합니다.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"
#include "aes132_config.h"
#include <string.h> // for memcpy, memcmp

/**
 * @brief Encrypt 명령어를 사용하여 데이터 암호화
 * 
 * Encrypt 명령어는 AES-128 암호화를 수행합니다.
 * 16바이트 블록 단위로 암호화됩니다.
 * 
 * @param key_id 사용할 키 슬롯 ID (0-15)
 * @param plaintext 암호화할 평문 데이터 (16바이트)
 * @param ciphertext 암호화된 데이터를 저장할 버퍼 (16바이트)
 * @return 1 성공 시, 0 실패 시
 */
uint8_t encrypt_data(uint8_t key_id, const uint8_t* plaintext, uint8_t* ciphertext) {
    if (key_id > 15) {
        Serial.println("Error: Key ID must be between 0 and 15.");
        return 0;
    }

    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // Encrypt 명령어 파라미터 구성
    // Mode: 0 (일반 암호화)
    // Param1: Key ID (0-15)
    // Param2: 0 (사용 안 함)
    // Data1: 평문 데이터 (16바이트)
    uint16_t param1 = key_id;
    uint16_t param2 = 0;

    // Encrypt 명령어 실행
    uint8_t ret = aes132m_execute(
        AES132_ENCRYPT,     // Op-Code: 0x06
        0,                  // Mode: 0 (일반 암호화)
        param1,             // Parameter 1: Key ID
        param2,             // Parameter 2: 0
        16, (uint8_t*)plaintext,  // Data 1: 평문 데이터 (16바이트)
        0, NULL,            // Data 2: 없음
        0, NULL,            // Data 3: 없음
        0, NULL,            // Data 4: 없음
        tx_buffer,
        rx_buffer
    );

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        // 응답에서 암호화된 데이터 추출
        // rx_buffer[0]: Count
        // rx_buffer[1]: Status (Return Code)
        // rx_buffer[2-17]: 암호화된 데이터 (16바이트)
        uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
        
        // Count가 최소 20이어야 함 (Count + Status + Data(16) + Checksum(2))
        if (count >= 20) {
            // 암호화된 데이터 복사
            memcpy(ciphertext, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
            return 1; // 성공
        } else {
            Serial.print("Error: Invalid response count: ");
            Serial.print(count);
            Serial.print(" (expected >= 20)");
            Serial.println();
        }
    } else {
        Serial.print("Error: Encrypt command failed with code 0x");
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
    Serial.println("Example 06: Encryption (AES-128)");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }

    Serial.println("AES132 initialized successfully\n");

    // 주의: 실제 암호화를 위해서는 키가 키 슬롯에 로드되어 있어야 합니다
    // 이 예제는 키 슬롯 0에 키가 이미 로드되어 있다고 가정합니다
    Serial.println("Note: This example assumes a key is already loaded in Key Slot 0.");
    Serial.println("      Use KeyLoad command (Example 8) to load a key first.\n");

    // 예제 1: 간단한 데이터 암호화
    Serial.println("=== Example 1: Encrypt Simple Data ===");
    uint8_t plaintext1[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    uint8_t ciphertext1[16] = {0};

    Serial.println("Plaintext (16 bytes):");
    print_hex("  ", plaintext1, 16);
    Serial.println();

    Serial.println("Encrypting with Key Slot 0...");
    uint8_t encrypt_ret = encrypt_data(0, plaintext1, ciphertext1);
    print_result("Encrypt", encrypt_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);

    if (encrypt_ret) {
        Serial.println("Ciphertext (16 bytes):");
        print_hex("  ", ciphertext1, 16);
        Serial.println();
    }
    Serial.println();

    // 예제 2: 텍스트 데이터 암호화
    Serial.println("=== Example 2: Encrypt Text Data ===");
    // "Hello AES132!" 문자열을 16바이트 블록으로 패딩
    uint8_t plaintext2[16] = {
        'H', 'e', 'l', 'l', 'o', ' ', 'A', 'E',
        'S', '1', '3', '2', '!', 0x00, 0x00, 0x00
    };
    uint8_t ciphertext2[16] = {0};

    Serial.print("Plaintext: \"");
    for (uint8_t i = 0; i < 13; i++) {
        Serial.print((char)plaintext2[i]);
    }
    Serial.println("\"");
    print_hex("Plaintext (hex): ", plaintext2, 16);
    Serial.println();

    Serial.println("Encrypting with Key Slot 0...");
    encrypt_ret = encrypt_data(0, plaintext2, ciphertext2);
    print_result("Encrypt", encrypt_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);

    if (encrypt_ret) {
        Serial.println("Ciphertext (16 bytes):");
        print_hex("  ", ciphertext2, 16);
        Serial.println();
    }
    Serial.println();

    // 예제 3: 여러 블록 암호화 (시뮬레이션)
    Serial.println("=== Example 3: Encrypt Multiple Blocks ===");
    Serial.println("Note: AES-128 encrypts one 16-byte block at a time.");
    Serial.println("      For multiple blocks, call Encrypt multiple times.\n");

    uint8_t plaintext3_block1[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    uint8_t plaintext3_block2[16] = {
        0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
        0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
    };
    uint8_t ciphertext3_block1[16] = {0};
    uint8_t ciphertext3_block2[16] = {0};

    Serial.println("Block 1:");
    print_hex("  Plaintext:  ", plaintext3_block1, 16);
    encrypt_ret = encrypt_data(0, plaintext3_block1, ciphertext3_block1);
    if (encrypt_ret) {
        print_hex("  Ciphertext: ", ciphertext3_block1, 16);
    }
    Serial.println();

    Serial.println("Block 2:");
    print_hex("  Plaintext:  ", plaintext3_block2, 16);
    encrypt_ret = encrypt_data(0, plaintext3_block2, ciphertext3_block2);
    if (encrypt_ret) {
        print_hex("  Ciphertext: ", ciphertext3_block2, 16);
    }
    Serial.println();

    // 예제 4: 암호화 결과 분석
    Serial.println("=== Example 4: Encryption Analysis ===");
    Serial.println("Characteristics of AES encryption:");
    Serial.println("1. Same plaintext always produces same ciphertext (with same key)");
    Serial.println("2. Small change in plaintext causes large change in ciphertext");
    Serial.println("3. Ciphertext appears random and unpredictable");
    Serial.println("4. Ciphertext length equals plaintext length (16 bytes)");
    Serial.println();

    Serial.println("\n=== Encryption Examples Complete ===");
    Serial.println("\nNote: To decrypt this data, use Decrypt command (Example 7)");
    Serial.println("      with the same key slot.");
}

void loop(void) {
    delay(1000);
}

