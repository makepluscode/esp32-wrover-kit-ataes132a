/**
 * @file main.cpp
 * @brief 예제 5: AES132 카운터 (Counter)
 *
 * 이 예제는 AES132 디바이스의 카운터 기능을 사용하는 방법을 보여줍니다.
 * 카운터 값 읽기 및 증가 기능을 학습합니다.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>

/**
 * @brief 디바이스 정보 확인 (Revision 등)
 */
void checkDeviceInfo() {
  Serial.println("=== Checking Device Info ===");
  uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

  // INFO command (Opcode 0x0C), Mode 0 (DevRev)
  // [AES132 Datasheet 8.9 Info Command]
  // OpCode: 0x0C (AES132_INFO) -> 디바이스 정보 읽기
  // Mode: 0 (DevRev)
  uint8_t ret =
      aes132m_execute(AES132_INFO, // [OpCode] Info (0x0C): 디바이스 정보 읽기
                      0,           // [Mode] 0: DevRev (Device Revision) 읽기
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

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.print("Device Revision: 0x");
    // DevRev is 4 bytes starting at index 2 (Data index)
    for (int i = 0; i < 4; i++) {
      if (rx_buffer[AES132_RESPONSE_INDEX_DATA + i] < 0x10)
        Serial.print("0");
      Serial.print(rx_buffer[AES132_RESPONSE_INDEX_DATA + i], HEX);
    }
    Serial.println();
  } else {
    Serial.print("Failed to read device info. RetCode: 0x");
    Serial.println(ret, HEX);
  }
  Serial.println();
}

/**
 * @brief Counter 명령어를 사용하여 카운터 값 읽기
 *
 * Counter 명령어의 Mode 0을 사용하여 카운터의 현재 값을 읽습니다.
 *
 * @param counter_id 읽을 카운터 ID (0-3, 총 4개의 카운터 슬롯)
 * @param counter_value 읽은 카운터 값을 저장할 포인터 (4바이트, 빅엔디안)
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t readCounter(uint8_t counter_id, uint32_t *counter_value) {
  if (counter_id > 3) {
    Serial.println("Error: Counter ID must be between 0 and 3.");
    return 0;
  }

  // Calculate address for the counter (Counter 0 at 0xF060, 4 bytes each)
  uint16_t address = 0xF060 + (counter_id * 4);
  uint8_t data[4];

  // Direct memory read instead of using Counter command (which causes 0x50
  // error) aes132m_read_memory(size, address, buffer)
  uint8_t ret = aes132m_read_memory(4, address, data);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    // Convert 4 bytes (Big Endian) to 32-bit integer
    *counter_value = ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
                     ((uint32_t)data[2] << 8) | (uint32_t)data[3];

    // Debug output
    Serial.print("Debug Counter ");
    Serial.print(counter_id);
    Serial.print(": Direct Read Success. Addr=0x");
    Serial.print(address, HEX);
    Serial.print(", Value=");
    Serial.println(*counter_value);

    return 1; // Success
  } else {
    Serial.print("Error: Direct read failed for Counter ");
    Serial.print(counter_id);
    Serial.print(" at address 0x");
    Serial.print(address, HEX);
    Serial.print(". RetCode: 0x");
    Serial.println(ret, HEX);
    return 0; // Failure
  }
}

/**
 * @brief Rewrite all counters to 0 (Initialize)
 * Using Direct Memory Write (0xF060 - 0xF06C)
 */
void resetCounters() {
  Serial.println("=== Resetting All Counters to 0 ===");
  uint8_t zero_data[4] = {0, 0, 0, 0};

  for (int i = 0; i < 4; i++) {
    uint16_t addr = 0xF060 + (i * 4);
    uint8_t ret = aes132m_write_memory(4, addr, zero_data);
    if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
      Serial.print("Counter ");
      Serial.print(i);
      Serial.print(" reset success (Addr: 0x");
      Serial.print(addr, HEX);
      Serial.println(")");
    } else {
      Serial.print("Failed to reset Counter ");
      Serial.print(i);
      Serial.print(". Ret: 0x");
      Serial.println(ret, HEX);
    }
    delay(10); // Safe delay between writes
  }
  Serial.println();
}

/**
 * @brief Counter 명령어를 사용하여 카운터 값 증가
 *
 * Counter 명령어의 Mode 1을 사용하여 카운터 값을 증가시킵니다.
 * (수정됨: Direct Read-Modify-Write 방식으로 변경)
 *
 * @param counter_id 증가시킬 카운터 ID (0-3)
 * @param increment_value 증가시킬 값 (1-255)
 * @param new_value 증가 후 새로운 카운터 값을 저장할 포인터 (선택사항, NULL
 * 가능)
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t incrementCounter(uint8_t counter_id, uint8_t increment_value,
                         uint32_t *new_value) {
  if (counter_id > 3) {
    Serial.println("Error: Counter ID must be between 0 and 3.");
    return 0;
  }

  // 1. Read current value (Direct Read)
  uint32_t current_val = 0;
  if (!readCounter(counter_id, &current_val)) {
    Serial.println(
        "Error: Failed to read current counter value for increment.");
    return 0;
  }

  // 2. Calculate new value
  uint32_t next_val = current_val + increment_value;

  // 3. Prepare data (Big Endian)
  uint8_t data[4];
  data[0] = (next_val >> 24) & 0xFF;
  data[1] = (next_val >> 16) & 0xFF;
  data[2] = (next_val >> 8) & 0xFF;
  data[3] = (next_val) & 0xFF;

  // 4. Write new value (Direct Write)
  uint16_t addr = 0xF060 + (counter_id * 4);
  uint8_t ret = aes132m_write_memory(4, addr, data);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    if (new_value != NULL) {
      *new_value = next_val;
    }
    return 1; // Success
  } else {
    Serial.print("Error: Failed to write incremented value. Ret: 0x");
    Serial.println(ret, HEX);
    return 0; // Failure
  }
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

  checkDeviceInfo();

  // Example 05 특화: 카운터 초기화
  // UNLOCKED 상태에서 카운터를 0으로 리셋하여 예제 실행 보장
  resetCounters();

  // 예제 1: 모든 카운터 값 읽기
  Serial.println("=== Example 1: Read All Counter Values ===");
  for (uint8_t i = 0; i < 4; i++) {
    uint32_t counter_value = 0;
    uint8_t success = readCounter(i, &counter_value);

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
      // ret는 이전 초기화 값이므로 의미 없음, 디버깅은 read_counter 내부에서
      // 출력됨
    }
  }
  Serial.println();

  // 예제 2: 특정 카운터 증가
  Serial.println("=== Example 2: Increment Counter 0 ===");
  uint32_t initial_value = 0;
  uint32_t new_value = 0;

  // 현재 값 읽기
  if (readCounter(0, &initial_value)) {
    Serial.print("Initial Counter 0 value: ");
    Serial.println(initial_value);

    // 카운터 증가 (1씩 증가)
    Serial.println("Incrementing Counter 0 by 1...");
    uint8_t inc_ret = incrementCounter(0, 1, &new_value);
    print_result("Increment Counter",
                 inc_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);

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

  if (readCounter(1, &start_value)) {
    Serial.print("Starting Counter 1 value: ");
    Serial.println(start_value);

    // 5번 증가
    for (uint8_t i = 0; i < 5; i++) {
      uint8_t inc_ret = incrementCounter(1, 1, &current_value);
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

  if (readCounter(2, &before_value)) {
    Serial.print("Counter 2 before increment: ");
    Serial.println(before_value);

    // 10씩 증가
    Serial.println("Incrementing Counter 2 by 10...");
    uint8_t inc_ret = incrementCounter(2, 10, &after_value);
    print_result("Increment Counter",
                 inc_ret ? AES132_DEVICE_RETCODE_SUCCESS : 0xFF);

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
    if (readCounter(i, &final_value)) {
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

void loop(void) { delay(1000); }
