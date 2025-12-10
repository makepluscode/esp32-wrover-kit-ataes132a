/**
 * @file main.cpp
 * @brief 예제 2: AES132 메모리 읽기 (BlockRead)
 * 
 * 이 예제는 AES132 디바이스의 메모리 영역에서 데이터를 읽는 방법을 보여줍니다.
 * BlockRead 명령어를 사용하여 다양한 메모리 영역을 읽어옵니다.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"
#include "aes132_config.h"

/**
 * @brief BlockRead 명령어를 사용하여 메모리 읽기
 * 
 * @param zone_id 읽을 영역 ID (0-15)
 * @param block_id 읽을 블록 ID (0-7, 각 영역은 8개 블록으로 구성)
 * @param length 읽을 바이트 수 (최대 16바이트)
 * @param data 읽은 데이터를 저장할 버퍼
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t read_memory_block(uint8_t zone_id, uint8_t block_id, uint8_t length, uint8_t* data) {
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];
    
    // BlockRead 명령어 파라미터 구성
    // param1: Zone ID (상위 4비트) + Block ID (하위 4비트)
    uint16_t param1 = ((uint16_t)zone_id << 8) | block_id;
    
    // param2: 읽을 바이트 수
    uint16_t param2 = length;
    
    // BlockRead 명령어 실행
    uint8_t ret = aes132m_execute(
        AES132_BLOCK_READ,  // Op-Code: 0x10
        0,                  // Mode: 0 (일반 읽기)
        param1,             // Parameter 1: Zone ID + Block ID
        param2,             // Parameter 2: Length
        0, NULL,            // Data 1: 없음
        0, NULL,            // Data 2: 없음
        0, NULL,            // Data 3: 없음
        0, NULL,            // Data 4: 없음
        tx_buffer,
        rx_buffer
    );
    
    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        // 응답에서 데이터 추출
        // rx_buffer[0]: Count
        // rx_buffer[1]: Status (Return Code)
        // rx_buffer[2]부터: 읽은 데이터
        uint8_t data_length = rx_buffer[AES132_RESPONSE_INDEX_COUNT] - 3; // Count - Status - Checksum(2)
        uint8_t copy_length = (data_length < length) ? data_length : length;
        
        for (uint8_t i = 0; i < copy_length; i++) {
            data[i] = rx_buffer[AES132_RESPONSE_INDEX_DATA + i];
        }
        
        return copy_length;
    }
    
    return 0;
}

void setup(void) {
    Serial.begin(AES132_SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n========================================");
    Serial.println("ESP32 AES132 CryptoAuth Example");
    Serial.println("Example 02: Memory Read (BlockRead)");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }

    Serial.println("AES132 initialized successfully\n");

    // 예제 1: Zone 0, Block 0에서 16바이트 읽기
    Serial.println("=== Example 1: Read Zone 0, Block 0 (16 bytes) ===");
    uint8_t data1[16] = {0};
    uint8_t bytes_read = read_memory_block(0, 0, 16, data1);
    
    if (bytes_read > 0) {
        Serial.print("Successfully read ");
        Serial.print(bytes_read);
        Serial.println(" bytes:");
        print_hex("Data: ", data1, bytes_read);
        Serial.println();
    } else {
        Serial.println("Failed to read memory");
        print_result("BlockRead", ret);
    }

    // 예제 2: Zone 0, Block 1에서 8바이트 읽기
    Serial.println("=== Example 2: Read Zone 0, Block 1 (8 bytes) ===");
    uint8_t data2[8] = {0};
    bytes_read = read_memory_block(0, 1, 8, data2);
    
    if (bytes_read > 0) {
        Serial.print("Successfully read ");
        Serial.print(bytes_read);
        Serial.println(" bytes:");
        print_hex("Data: ", data2, bytes_read);
        Serial.println();
    } else {
        Serial.println("Failed to read memory");
    }

    // 예제 3: 여러 블록 연속 읽기
    Serial.println("=== Example 3: Read Multiple Blocks (Zone 0, Blocks 0-2) ===");
    for (uint8_t block = 0; block < 3; block++) {
        uint8_t block_data[16] = {0};
        bytes_read = read_memory_block(0, block, 16, block_data);
        
        if (bytes_read > 0) {
            Serial.print("Block ");
            Serial.print(block);
            Serial.print(": ");
            print_hex("", block_data, bytes_read);
            Serial.println();
        } else {
            Serial.print("Failed to read Block ");
            Serial.println(block);
        }
    }

    Serial.println("\n=== Memory Read Examples Complete ===");
}

void loop(void) {
    delay(1000);
}

