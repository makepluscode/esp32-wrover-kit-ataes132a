/**
 * @file main.cpp
 * @brief 예제 8: 키 로드 (Key Load)
 *
 * 이 예제는 AES132 디바이스의 Key Memory에 키를 저장하는 방법을 보여줍니다.
 * 1. Key Configuration 설정 (암호화/복호화 활성화)
 * 2. Key Load 명령어를 사용하여 키 저장
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"
#include "aes132_config.h"
#include <string.h>

// AES132 메모리 주소 정의
// AES132 메모리 주소 정의 (Corrected)
#define AES132_CONFIG_ADDR_BASE      0xF000
#define AES132_LOCK_CONFIG_ADDR      0xF020 // Lock Config Base
#define AES132_CHIP_CONFIG_ADDR      0xF040 // Chip Config Base
#define AES132_KEY_CONFIG_ADDR_BASE  0xF080 // Key Config Base (Key 0)

/**
 * @brief Key Configuration 설정 (Config Memory 상세 설정)
 *
 * 키 슬롯의 권한을 설정합니다.
 *
 * @param key_id 설정할 Key ID (0-15)
 * @return 0x00=성공
 */
uint8_t configure_key_slot(uint8_t key_id) {
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // KeyConfig 주소 계산 (슬롯당 2바이트)
    uint16_t config_addr = AES132_KEY_CONFIG_ADDR_BASE + (key_id * 2);

    // KeyConfig 값 설정
    // Data (2 bytes):
    // [0] 0x1C (External Crypto Enable, Key Usage Enable...)
    // [1] 0x00 (Authorization requirements, 0x00 = Free access for simplicity)
    uint8_t config_data[4] = {0x1C, 0x00, 0x00, 0x00}; // 4바이트 단위 쓰기 권장 (Word Align)

    // 쓰기 전 락 해제 상태인지 확인 필요하지만, 예제에서는 Unlocked 가정
    // aes132m_write_memory 사용 (BlockWrite Wrapper)
    uint8_t ret = aes132m_write_memory(4, config_addr, config_data);

    return ret;
}

/**
 * @brief Key Load 명령어를 사용하여 키 저장
 *
 * @param key_id 저장할 Key ID (0-15)
 * @param key_data 저장할 키 데이터 (16바이트)
 * @return 0x00=성공
 */
uint8_t load_key(uint8_t key_id, const uint8_t* key_data) {
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // KeyLoad 명령어 실행
    // Mode: 1 (Key Memory Write)
    // Param1: Key ID
    // Param2: 0
    // Data1: Key Data (16 bytes)
    uint8_t ret = aes132m_execute(
        AES132_KEY_LOAD,    // Op-Code: 0x09
        1,                  // Mode: 1 (Load Key Memory)
        key_id,             // Param1: Key ID
        0,                  // Param2: 0
        16, (uint8_t*)key_data, // Data 1: Key Data
        0, NULL, 0, NULL, 0, NULL,
        tx_buffer, rx_buffer
    );

    return ret;
}

void setup(void) {
    Serial.begin(AES132_SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n========================================");
    Serial.println("ESP32 AES132 CryptoAuth Example");
    Serial.println("Example 08: Key Load");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }
    Serial.println("AES132 initialized successfully");

    // DEBUG: 상태 읽기
    uint8_t lock_config = 0xFF;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // Read LockConfig (Address 0xF002) using BLOCK_READ (Command 0x10)
    // Direct Read produced 0xFF, trying BlockCmd for verification
    Serial.println("\n=== DEBUG: Checking Device State (BlockRead) ===");
    
    // Param1: Address (0xF000)
    // Param2: Length (4)
    uint8_t ret_read = aes132m_execute(
        AES132_BLOCK_READ,
        0, 0xF000, 4,
        0, NULL, 0, NULL, 0, NULL, 0, NULL,
        tx_buffer, rx_buffer
    );

    if (ret_read == AES132_DEVICE_RETCODE_SUCCESS) {
        // Response: [Count][Status][Data...][CRC]
        // Data starts at index 2
        uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
        if (count >= 4 + 2) {
            Serial.print("Config Memory [0xF000-0xF003]: ");
            print_hex("", &rx_buffer[AES132_RESPONSE_INDEX_DATA], 4);
            
            lock_config = rx_buffer[AES132_RESPONSE_INDEX_DATA + 2];
            Serial.print("LockConfig (0xF002): 0x");
            Serial.println(lock_config, HEX);
            
            if (lock_config == 0x55) Serial.println("-> Device is UNLOCKED");
            else if (lock_config == 0x00) Serial.println("-> Device is LOCKED");
            else Serial.println("-> Device Lock State Unknown (Likely LOCKED)");
        }
    } else {
        Serial.print("Failed to read Config Memory (BlockRead): 0x");
        Serial.println(ret_read, HEX);
    }

    // Read KeyConfig[0] (Address 0xF020) using BLOCK_READ
    Serial.println("\n=== DEBUG: Reading KeyConfig[0] (BlockRead) ===");
    // KeyConfig starts at 0xF020. Reading 4 bytes.
    ret_read = aes132m_execute(
        AES132_BLOCK_READ,
        0, 0xF020, 4,
        0, NULL, 0, NULL, 0, NULL, 0, NULL,
        tx_buffer, rx_buffer
    );

    if (ret_read == AES132_DEVICE_RETCODE_SUCCESS) {
        // [Count][Status][Data(4)][CRC]
        uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
        if (count >= 4 + 2) {
            Serial.print("KeyConfig[0] (0xF020): ");
            // Data index is 2
            uint8_t* key_cfg = &rx_buffer[AES132_RESPONSE_INDEX_DATA];
            print_hex("", key_cfg, 4);
            
            // Analyze Permissions
            // KeyConfig[0]: External Crypto Enable
            // KeyConfig[1]: Auth Requirements
            // KeyConfig[2]: TimeStamp/Counter / Link Pointer
            // KeyConfig[3]: Restrictions (Counter/Usage)

            // Analysis of Byte 2 (KeyConfig[1] or [2] depending on endianness/datasheet, usually Byte 1 index)
            // But let's print raw bytes first.
        }
    } else {
        Serial.println("Failed to read KeyConfig[0] via BlockRead");
    }

    // Dump ALL KeyConfigs (0-15)
    Serial.println("\n=== DEBUG: Dumping KeyConfig Table (0-15) ===");
    Serial.println("Slot | KeyConfig Bytes (4)");
    Serial.println("-----|---------------------");
    
    for (int i = 0; i < 16; i++) {
        uint16_t addr = 0xF020 + (i * 2); // KeyConfig Address
        // WRONG ADDR CALCULATION ABOVE: KeyConfig is 2 words (4 bytes) per key? No.
        // Datasheet: KeyConfig is 4 bytes per Key.
        // Base 0xF020. Key 0 -> F020. Key 1 -> F024.
        // Wait, User doc says "Address calculation" might differ.
        // Standard: 0xF020 + (KeyID * 4).
        // Let's verify standard addressing.
        addr = 0xF020 + (i * 4); // Corrected to 4 bytes stride
        
        ret_read = aes132m_execute(
            AES132_BLOCK_READ,
            0, addr, 4,
            0, NULL, 0, NULL, 0, NULL, 0, NULL,
            tx_buffer, rx_buffer
        );
        
        if (ret_read == AES132_DEVICE_RETCODE_SUCCESS) {
            uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
            if (count >= 6) {
                Serial.print(" ");
                if (i < 10) Serial.print(" ");
                Serial.print(i);
                Serial.print("  | ");
                print_hex("", &rx_buffer[AES132_RESPONSE_INDEX_DATA], 4);
            }
        } else {
            Serial.print("  ");
            Serial.print(i);
            Serial.println("  | Read Failed");
        }
        delay(50); // Small delay to prevent I2C saturation/timeouts
    }
    Serial.println("---------------------------");

    Serial.println("---------------------------");

    // === 키 슬롯 2 사용 시도 ===
    // Slot 1 (FF...) 실패. Slot 2 (0F... Enc enabled) 사용 시도.
    uint8_t target_key_id = 2;
    
    // 1. Key Configuration (권한 설정)
    // 칩이 Locked 상태라면 이 단계는 실패(0x70)할 수 있습니다.
    Serial.print("\n=== Step 1: Configure Key Slot ");
    Serial.print(target_key_id);
    Serial.println(" ===");
    Serial.println("Setting KeyConfig to allow Encryption/Decryption...");
    
    ret = configure_key_slot(target_key_id);
    print_result("Configure Slot", ret);
    
    if (ret != AES132_DEVICE_RETCODE_SUCCESS) {
        Serial.println("Note: Configuration might fail if device is Locked. Proceeding to Key Load...");
    }

    // 2. Key Load (키 데이터 저장)
    Serial.print("\n=== Step 2: Load Key into Slot ");
    Serial.print(target_key_id);
    Serial.println(" ===");
    
    uint8_t my_key[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };
    
    print_hex("Key to Load: ", my_key, 16);
    
    ret = load_key(target_key_id, my_key);
    print_result("Key Load", ret);

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        Serial.println("\nSUCCESS: Key loaded successfully!");
        Serial.println("IMPORTANT: You must update Example 6 to use Slot 2.");
    } else {
        Serial.println("\nFAILED: Could not load key.");
        Serial.println("Possible reasons:");
        Serial.println("- Config Memory is Locked and current config forbids KeyLoad");
    }
}

void loop(void) {
    delay(1000);
}
