/**
 * @file main.cpp
 * @brief 예제 1: AES132 Info 명령어
 *
 * 이 예제는 AES132 디바이스의 기본 정보를 읽어오는 방법을 보여줍니다.
 * Info 명령어는 디바이스 버전, 제조사 ID 등의 정보를 반환합니다.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h" // from lib/aes132_utils/
#include "i2c_phys.h"
#include <Arduino.h>

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

  // [AES132 Datasheet 8.9 Info Command]
  // OpCode: 0x0C (AES132_INFO) -> 디바이스 정보 읽기
  // Mode: 0 (DevRev - Device Revision)
  // Param1: 0
  // Param2: 0
  ret = aes132m_execute(
      AES132_INFO, // [OpCode] Info Command (0x0C): 디바이스 정보 읽기
      0,           // [Mode] 0x00: DevRev (Device Revision) 읽기
      0,           // [Param1] 0: 사용하지 않음
      0,           // [Param2] 0: 사용하지 않음
      0,           // [Data1 Length] 0: 입력 데이터 없음
      NULL,        // [Data1 Pointer] NULL: 입력 데이터 없음
      0,           // [Data2 Length] 0: 입력 데이터 없음
      NULL,        // [Data2 Pointer] NULL: 입력 데이터 없음
      0,           // [Data3 Length] 0: 입력 데이터 없음
      NULL,        // [Data3 Pointer] NULL: 입력 데이터 없음
      0,           // [Data4 Length] 0: 입력 데이터 없음
      NULL,        // [Data4 Pointer] NULL: 입력 데이터 없음
      tx_buffer,   // [TX Buffer] 송신 버퍼 포인터
      rx_buffer    // [RX Buffer] 수신 버퍼 포인터
  );

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

void loop(void) { delay(1000); }
