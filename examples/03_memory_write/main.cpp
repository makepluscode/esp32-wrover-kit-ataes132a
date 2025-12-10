/**
 * @file main.cpp
 * @brief 예제 3: AES132 메모리 쓰기
 * 
 * 이 예제는 AES132 디바이스의 메모리 영역에 데이터를 쓰는 방법을 보여줍니다.
 * 표준 EEPROM 쓰기 방식을 사용하여 메모리에 데이터를 저장합니다.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"
#include "aes132_config.h"

/**
 * @brief Zone ID와 Block ID를 Word Address로 변환
 * 
 * AES132의 메모리 주소는 Word Address 형식을 사용합니다.
 * Word Address = (Zone ID * 128) + Block ID
 * 
 * @param zone_id Zone ID (0-15)
 * @param block_id Block ID (0-127, 각 Zone당 128개 블록)
 * @return Word Address
 */
uint16_t calculate_word_address(uint8_t zone_id, uint8_t block_id) {
    // 각 Zone은 128개 블록 (2KB = 2048 bytes / 16 bytes per block = 128 blocks)
    return ((uint16_t)zone_id * 128) + block_id;
}

/**
 * @brief 메모리 블록에 데이터 쓰기
 * 
 * @param zone_id 쓰기할 Zone ID (0-15)
 * @param block_id 쓰기할 Block ID (0-127)
 * @param data 쓸 데이터
 * @param length 데이터 길이 (최대 16바이트, 블록 크기)
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t write_memory_block(uint8_t zone_id, uint8_t block_id, const uint8_t* data, uint8_t length) {
    // 블록 크기 제한 확인
    if (length > 16) {
        length = 16;
    }
    
    // Word Address 계산
    uint16_t word_address = calculate_word_address(zone_id, block_id);
    
    // 메모리 쓰기 실행
    uint8_t ret = aes132m_write_memory(length, word_address, (uint8_t*)data);
    
    return ret;
}

/**
 * @brief 메모리 블록 읽기 (검증용)
 * 
 * @param zone_id 읽을 Zone ID (0-15)
 * @param block_id 읽을 Block ID (0-127)
 * @param data 읽은 데이터를 저장할 버퍼
 * @param length 읽을 바이트 수
 * @return 읽은 바이트 수
 */
uint8_t read_memory_block(uint8_t zone_id, uint8_t block_id, uint8_t* data, uint8_t length) {
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];
    
    // BlockRead 명령어 파라미터 구성
    uint16_t param1 = ((uint16_t)zone_id << 8) | block_id;
    uint16_t param2 = length;
    
    // BlockRead 명령어 실행
    uint8_t ret = aes132m_execute(
        AES132_BLOCK_READ,
        0,
        param1,
        param2,
        0, NULL, 0, NULL, 0, NULL, 0, NULL,
        tx_buffer,
        rx_buffer
    );
    
    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        uint8_t data_length = rx_buffer[AES132_RESPONSE_INDEX_COUNT] - 3;
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
    Serial.println("Example 03: Memory Write");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }

    Serial.println("AES132 initialized successfully\n");

    // 예제 1: 간단한 데이터 쓰기 및 검증
    Serial.println("=== Example 1: Write and Verify Simple Data ===");
    uint8_t test_data1[16] = {
        0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x41, 0x45,
        0x53, 0x31, 0x33, 0x32, 0x21, 0x00, 0x00, 0x00
    }; // "Hello AES132!"
    
    Serial.println("Writing data to Zone 0, Block 0...");
    print_hex("Data to write: ", test_data1, 13);
    
    ret = write_memory_block(0, 0, test_data1, 13);
    print_result("Write Memory", ret);
    
    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        Serial.println("Write successful! Reading back to verify...");
        delay(100); // 쓰기 완료 대기
        
        uint8_t read_back1[16] = {0};
        uint8_t bytes_read = read_memory_block(0, 0, read_back1, 16);
        
        if (bytes_read > 0) {
            print_hex("Data read back: ", read_back1, bytes_read);
            
            // 데이터 비교
            bool match = true;
            for (uint8_t i = 0; i < 13; i++) {
                if (read_back1[i] != test_data1[i]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                Serial.println("✓ Verification successful! Data matches.");
            } else {
                Serial.println("✗ Verification failed! Data mismatch.");
            }
        }
    }
    Serial.println();

    // 예제 2: 여러 블록에 연속 데이터 쓰기
    Serial.println("=== Example 2: Write Multiple Blocks ===");
    uint8_t test_data2[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                               0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    
    for (uint8_t block = 1; block < 4; block++) {
        Serial.print("Writing to Zone 0, Block ");
        Serial.print(block);
        Serial.println("...");
        
        ret = write_memory_block(0, block, test_data2, 16);
        print_result("Write", ret);
        
        if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
            delay(100);
            
            uint8_t read_back2[16] = {0};
            uint8_t bytes_read = read_memory_block(0, block, read_back2, 16);
            
            if (bytes_read > 0) {
                Serial.print("Block ");
                Serial.print(block);
                Serial.print(": ");
                print_hex("", read_back2, bytes_read);
                Serial.println();
            }
        }
    }
    Serial.println();

    // 예제 3: 부분 데이터 쓰기 (블록의 일부만 쓰기)
    Serial.println("=== Example 3: Partial Block Write ===");
    uint8_t test_data3[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
    
    Serial.println("Writing 8 bytes to Zone 0, Block 5...");
    print_hex("Data to write: ", test_data3, 8);
    
    ret = write_memory_block(0, 5, test_data3, 8);
    print_result("Write", ret);
    
    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        delay(100);
        
        uint8_t read_back3[16] = {0};
        uint8_t bytes_read = read_memory_block(0, 5, read_back3, 16);
        
        if (bytes_read > 0) {
            Serial.println("Reading entire block (16 bytes):");
            print_hex("Data read back: ", read_back3, bytes_read);
            Serial.println("Note: Only first 8 bytes were written, rest remains 0xFF");
        }
    }

    Serial.println("\n=== Memory Write Examples Complete ===");
}

void loop(void) {
    delay(1000);
}

