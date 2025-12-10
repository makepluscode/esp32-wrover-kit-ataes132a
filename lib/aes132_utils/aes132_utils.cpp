/**
 * @file aes132_utils.cpp
 * @brief AES132 공통 유틸리티 함수 구현
 */

#include "aes132_utils.h"
#include "aes132_comm_marshaling.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <stdio.h>

// ============================================================================
// 초기화 함수
// ============================================================================

uint8_t aes132_init(void) {
    uint8_t ret;

    // I2C 핀 설정
    i2c_set_pins(AES132_SDA_PIN, AES132_SCL_PIN);

    // I2C 인터페이스 활성화
    aes132p_enable_interface();

    // AES132 디바이스 선택
    ret = aes132p_select_device(AES132_I2C_ADDRESS);
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        return ret;
    }

    // 디바이스 웨이크업
    ret = aes132c_wakeup();
    if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
        return ret;
    }

    return AES132_FUNCTION_RETCODE_SUCCESS;
}

// ============================================================================
// 출력 유틸리티 함수
// ============================================================================

void print_hex(const char* label, const uint8_t* data, uint8_t length) {
    if (label != NULL) {
        Serial.print(label);
    }

    for (uint8_t i = 0; i < length; i++) {
        if (data[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(data[i], HEX);
        if (i < length - 1) {
            Serial.print(" ");
        }
    }
    Serial.println();
}

void print_response(const uint8_t* rx_buffer, uint8_t count) {
    Serial.println("\n=== Response Packet ===");
    Serial.print("Count: ");
    Serial.println(count);

    if (count > 0) {
        Serial.print("Status: 0x");
        if (rx_buffer[1] < 0x10) {
            Serial.print("0");
        }
        Serial.print(rx_buffer[1], HEX);
        Serial.print(" (");
        Serial.print(get_error_string(rx_buffer[1]));
        Serial.println(")");

        if (count > 2) {
            Serial.print("Data: ");
            print_hex(NULL, &rx_buffer[2], count - 2);
        }
    }
    Serial.println("======================\n");
}

const char* get_error_string(uint8_t error_code) {
    switch (error_code) {
        case AES132_DEVICE_RETCODE_SUCCESS:
            return "Success";
        case AES132_DEVICE_RETCODE_BOUNDARY_ERROR:
            return "Boundary Error";
        case AES132_DEVICE_RETCODE_RW_CONFIG:
            return "Read/Write Config Error";
        case AES132_DEVICE_RETCODE_BAD_ADDR:
            return "Bad Address";
        case AES132_DEVICE_RETCODE_COUNT_ERROR:
            return "Count Error";
        case AES132_DEVICE_RETCODE_NONCE_ERROR:
            return "Nonce Error";
        case AES132_DEVICE_RETCODE_MAC_ERROR:
            return "MAC Error";
        case AES132_DEVICE_RETCODE_PARSE_ERROR:
            return "Parse Error";
        case AES132_DEVICE_RETCODE_DATA_MISMATCH:
            return "Data Mismatch";
        case AES132_DEVICE_RETCODE_LOCK_ERROR:
            return "Lock Error";
        case AES132_DEVICE_RETCODE_KEY_ERROR:
            return "Key Error";
        case AES132_DEVICE_RETCODE_TEMP_SENSE_ERROR:
            return "Temp Sense Error";
        default:
            return "Unknown Error";
    }
}

void print_result(const char* operation, uint8_t result) {
    Serial.print("[");
    Serial.print(operation);
    Serial.print("] ");

    if (result == AES132_DEVICE_RETCODE_SUCCESS) {
        Serial.print("SUCCESS");
    } else {
        Serial.print("FAILED: 0x");
        if (result < 0x10) {
            Serial.print("0");
        }
        Serial.print(result, HEX);
        Serial.print(" (");
        Serial.print(get_error_string(result));
        Serial.print(")");
    }
    Serial.println();
}

// ============================================================================
// 데이터 비교 함수
// ============================================================================

int compare_data(const uint8_t* data1, const uint8_t* data2, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        if (data1[i] != data2[i]) {
            return 1;  // 다름
        }
    }
    return 0;  // 같음
}

