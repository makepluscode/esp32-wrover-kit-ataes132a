/**
 * @file main.cpp
 * @brief 예제 1: AES132 Info 명령어
 * 
 * 이 예제는 AES132 디바이스의 기본 정보를 읽어오는 방법을 보여줍니다.
 * Info 명령어는 디바이스 버전, 제조사 ID 등의 정보를 반환합니다.
 */

#include <Arduino.h>
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include "aes132_utils.h"  // from lib/aes132_utils/
#include "aes132_config.h"

void setup(void) {
    Serial.begin(AES132_SERIAL_BAUD);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n========================================");
    Serial.println("ESP32 AES132 CryptoAuth Example");
    Serial.println("Example 01: Info Command");
    Serial.println("========================================\n");

    // AES132 초기화
    uint8_t ret = aes132_init();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        Serial.print("Failed to initialize AES132: 0x");
        Serial.println(ret, HEX);
        return;
    }

    Serial.println("AES132 initialized successfully\n");

    // Execute Info command
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

    ret = aes132m_execute(AES132_INFO, 0, 0, 0,
                          0, NULL, 0, NULL, 0, NULL, 0, NULL,
                          tx_buffer, rx_buffer);

    print_result("Info Command", ret);

    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
        uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
        print_response(rx_buffer, count);

        // 상세 정보 출력
        Serial.println("=== Detailed Response ===");
        Serial.print("Response count: ");
        Serial.println(count);

        Serial.println("\nResponse data (byte-by-byte):");
        for (uint8_t i = 0; i < count && i < AES132_RESPONSE_SIZE_MAX; i++) {
            Serial.print("  [");
            Serial.print(i);
            Serial.print("] = 0x");
            if (rx_buffer[i] < 0x10) {
                Serial.print("0");
            }
            Serial.println(rx_buffer[i], HEX);
        }
    }
}

void loop(void) {
    delay(1000);
}

