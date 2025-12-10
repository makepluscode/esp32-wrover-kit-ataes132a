/**
 * @file main.cpp
 * @brief 예제 5: AES132 카운터 (Counter)
 * 
 * 이 예제는 AES132 디바이스의 카운터 기능을 사용하는 방법을 보여줍니다.
 * 카운터 값 읽기 및 증가 기능을 학습합니다.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"
#include "aes132_config.h"

/**
 * @brief Counter 명령어를 사용하여 카운터 값 읽기
 * 
 * Counter 명령어의 Mode 0을 사용하여 카운터의 현재 값을 읽습니다.
 * 
 * @param counter_id 읽을 카운터 ID (0-3, 총 4개의 카운터 슬롯)
 * @param counter_value 읽은 카운터 값을 저장할 포인터 (4바이트, 빅엔디안)
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t read_counter(uint8_t counter_id, uint32_t* counter_value) {
    if (counter_id > 3) {
        Serial.println("Error: Counter ID must be between 0 and 3.");
        return 0;
    }

    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // Counter 명령어 파라미터 구성
    // Mode: 0 (카운터 읽기)
    // Param1: Counter ID (0-3)
    // Param2: 0 (읽기 모드에서는 사용 안 함)
    uint16_t param1 = counter_id;
    uint16_t param2 = 0;

    // Counter 명령어 실행 (읽기 모드)
    uint8_t ret = aes132m_execute(
        AES132_COUNTER,     // Op-Code: 0x0A
        0,                  // Mode: 0 (카운터 읽기)
        param1,             // Parameter 1: Counter ID
        param2,             // Parameter 2: 0 (읽기 모드)
        0, NULL,            // Data 1: 없음
        0, NULL,            // Data 2: 없음
        0, NULL,            // Data 3: 없음
        0, NULL,            // Data 4: 없음
        tx_buffer,
        rx_buffer
    );

    // 디버깅: 응답 패킷 전체 출력
    Serial.print("Debug Counter ");
    Serial.print(counter_id);
    Serial.print(": ret=0x");
    Serial.print(ret, HEX);
    Serial.print(", Count=");
    Serial.print(rx_buffer[AES132_RESPONSE_INDEX_COUNT]);
    Serial.print(", Status=0x");
    Serial.print(rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE], HEX);
    Serial.print(", Data=");
    uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
    for (uint8_t i = 0; i < count && i < 10; i++) {
        Serial.print("0x");
        if (rx_buffer[i] < 0x10) Serial.print("0");
        Serial.print(rx_buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        // 응답에서 카운터 값 추출
        // rx_buffer[0]: Count (전체 패킷 크기)
        // rx_buffer[1]: Status (Return Code)
        // rx_buffer[2-5]: 카운터 값 (4바이트, 빅엔디안)
        // rx_buffer[6-7]: Checksum (2바이트)
        // Count = Count(1) + Status(1) + Data(4) + Checksum(2) = 8
        
        // Count가 최소 8이어야 함 (Count + Status + Data(4) + Checksum(2))
        if (count >= 8) {
            // 빅엔디안으로 4바이트를 32비트 값으로 변환
            *counter_value = ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA] << 24) |
                            ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA + 1] << 16) |
                            ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA + 2] << 8) |
                            ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA + 3]);
            return 1; // 성공
        } else {
            Serial.print("Error: Invalid response count: ");
            Serial.print(count);
            Serial.print(" (expected >= 8)");
            Serial.println();
        }
    } else {
        Serial.print("Error: Counter command failed with code 0x");
        Serial.println(ret, HEX);
    }

    return 0; // 실패
}

/**
 * @brief Counter 명령어를 사용하여 카운터 값 증가
 * 
 * Counter 명령어의 Mode 1을 사용하여 카운터 값을 증가시킵니다.
 * 
 * @param counter_id 증가시킬 카운터 ID (0-3)
 * @param increment_value 증가시킬 값 (1-255)
 * @param new_value 증가 후 새로운 카운터 값을 저장할 포인터 (선택사항, NULL 가능)
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t increment_counter(uint8_t counter_id, uint8_t increment_value, uint32_t* new_value) {
    if (counter_id > 3) {
        Serial.println("Error: Counter ID must be between 0 and 3.");
        return 0;
    }

    if (increment_value == 0 || increment_value > 255) {
        Serial.println("Error: Increment value must be between 1 and 255.");
        return 0;
    }

    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    // Counter 명령어 파라미터 구성
    // Mode: 1 (카운터 증가)
    // Param1: Counter ID (0-3)
    // Param2: 증가시킬 값 (1-255)
    uint16_t param1 = counter_id;
    uint16_t param2 = increment_value;

    // Counter 명령어 실행 (증가 모드)
    uint8_t ret = aes132m_execute(
        AES132_COUNTER,     // Op-Code: 0x0A
        1,                  // Mode: 1 (카운터 증가)
        param1,             // Parameter 1: Counter ID
        param2,             // Parameter 2: 증가값
        0, NULL,            // Data 1: 없음
        0, NULL,            // Data 2: 없음
        0, NULL,            // Data 3: 없음
        0, NULL,            // Data 4: 없음
        tx_buffer,
        rx_buffer
    );

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        // 응답에서 새로운 카운터 값 추출 (선택사항)
        if (new_value != NULL) {
            uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
            // Count가 최소 8이어야 함 (Count + Status + Data(4) + Checksum(2))
            if (count >= 8) {
                *new_value = ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA] << 24) |
                            ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA + 1] << 16) |
                            ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA + 2] << 8) |
                            ((uint32_t)rx_buffer[AES132_RESPONSE_INDEX_DATA + 3]);
            }
        }
        return 1; // 성공
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
    Serial.println("Example 05: Counter");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }

    Serial.println("AES132 initialized successfully\n");

    // 예제 1: 모든 카운터 값 읽기
    Serial.println("=== Example 1: Read All Counter Values ===");
    for (uint8_t i = 0; i < 4; i++) {
        uint32_t counter_value = 0;
        uint8_t success = read_counter(i, &counter_value);
        
        if (success) {
            Serial.print("Counter ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(counter_value);
            Serial.print(" (0x");
            Serial.print(counter_value, HEX);
            Serial.println(")");
        } else {
            Serial.print("Failed to read Counter ");
            Serial.println(i);
            // ret는 이전 초기화 값이므로 의미 없음, 디버깅은 read_counter 내부에서 출력됨
        }
    }
    Serial.println();

    // 예제 2: 특정 카운터 증가
    Serial.println("=== Example 2: Increment Counter 0 ===");
    uint32_t initial_value = 0;
    uint32_t new_value = 0;
    
    // 현재 값 읽기
    if (read_counter(0, &initial_value)) {
        Serial.print("Initial Counter 0 value: ");
        Serial.println(initial_value);
        
        // 카운터 증가 (1씩 증가)
        Serial.println("Incrementing Counter 0 by 1...");
        uint8_t inc_ret = increment_counter(0, 1, &new_value);
        print_result("Increment Counter", inc_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);
        
        if (inc_ret) {
            Serial.print("New Counter 0 value: ");
            Serial.println(new_value);
            Serial.print("Difference: ");
            Serial.println(new_value - initial_value);
        }
    } else {
        Serial.println("Failed to read initial counter value");
    }
    Serial.println();

    // 예제 3: 여러 번 증가
    Serial.println("=== Example 3: Increment Counter 1 Multiple Times ===");
    uint32_t start_value = 0;
    uint32_t current_value = 0;
    
    if (read_counter(1, &start_value)) {
        Serial.print("Starting Counter 1 value: ");
        Serial.println(start_value);
        
        // 5번 증가
        for (uint8_t i = 0; i < 5; i++) {
            uint8_t inc_ret = increment_counter(1, 1, &current_value);
            if (inc_ret) {
                Serial.print("After increment ");
                Serial.print(i + 1);
                Serial.print(": ");
                Serial.println(current_value);
                delay(100); // 약간의 지연
            } else {
                Serial.print("Failed to increment at step ");
                Serial.println(i + 1);
                break;
            }
        }
        
        Serial.print("Final Counter 1 value: ");
        Serial.println(current_value);
        Serial.print("Total increments: ");
        Serial.println(current_value - start_value);
    } else {
        Serial.println("Failed to read starting counter value");
    }
    Serial.println();

    // 예제 4: 큰 값으로 증가
    Serial.println("=== Example 4: Increment Counter 2 by Large Value ===");
    uint32_t before_value = 0;
    uint32_t after_value = 0;
    
    if (read_counter(2, &before_value)) {
        Serial.print("Counter 2 before increment: ");
        Serial.println(before_value);
        
        // 10씩 증가
        Serial.println("Incrementing Counter 2 by 10...");
        uint8_t inc_ret = increment_counter(2, 10, &after_value);
        print_result("Increment Counter", inc_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);
        
        if (inc_ret) {
            Serial.print("Counter 2 after increment: ");
            Serial.println(after_value);
            Serial.print("Difference: ");
            Serial.println(after_value - before_value);
        }
    } else {
        Serial.println("Failed to read counter value");
    }
    Serial.println();

    // 예제 5: 최종 카운터 상태 확인
    Serial.println("=== Example 5: Final Counter States ===");
    for (uint8_t i = 0; i < 4; i++) {
        uint32_t final_value = 0;
        if (read_counter(i, &final_value)) {
            Serial.print("Counter ");
            Serial.print(i);
            Serial.print(" final value: ");
            Serial.print(final_value);
            Serial.print(" (0x");
            Serial.print(final_value, HEX);
            Serial.println(")");
        }
    }

    Serial.println("\n=== Counter Examples Complete ===");
    Serial.println("\nNote: Counter values are stored in non-volatile memory");
    Serial.println("      and persist across power cycles.");
}

void loop(void) {
    delay(1000);
}

