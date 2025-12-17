/**
 * @file main.cpp
 * @brief 예제 4: AES132 랜덤 숫자 생성 (Random)
 *
 * 이 예제는 AES132 디바이스의 하드웨어 RNG(Random Number Generator)를 사용하여
 * 암호학적으로 안전한 랜덤 숫자를 생성하는 방법을 보여줍니다.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>

/**
 * @brief Random 명령어를 사용하여 랜덤 바이트 생성
 *
 * Random 명령어는 AES132의 하드웨어 RNG를 사용하여 암호학적으로 안전한
 * 랜덤 바이트를 생성합니다.
 *
 * @param random_data 생성된 랜덤 데이터를 저장할 버퍼
 * @param length 생성할 랜덤 바이트 수 (최대 32바이트)
 * @return AES132_DEVICE_RETCODE_SUCCESS 성공 시
 */
uint8_t generateRandom(uint8_t *random_data, uint8_t length) {
  if (length == 0 || length > 32) {
    Serial.println("Error: Random length must be between 1 and 32 bytes.");
    return 0; // Return 0 on error (same pattern as read_memory_block)
  }

  uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];
  memset(rx_buffer, 0, AES132_RESPONSE_SIZE_MAX); // Initialize buffer

  // Random 명령어 파라미터 구성
  // Mode: 0 (일반 랜덤 생성)
  // Param1: 0 (Mode 0에서는 무시됨, 0으로 설정)
  // Param2: 0 (사용 안 함)
  uint16_t param1 = 0;
  uint16_t param2 = 0;

  // Random 명령어 실행
  // [AES132 Datasheet 8.12 Random Command]
  // OpCode: 0x1B (AES132_RANDOM) -> 랜덤 생성
  // Mode: 0 -> Seed Update 없이 난수 생성
  // Param1: 0
  // Param2: 0
  uint8_t ret = aes132m_execute(
      AES132_RANDOM, // [OpCode] Random (0x1B): 랜덤 데이터 생성 명령
      0,             // [Mode] 0: Seed Update 없이 난수 생성
      param1,        // [Param1] 0: Mode 0에서는 무시됨
      param2,        // [Param2] 0: Mode 0에서는 무시됨
      0,             // [Data1 Length] 0: 입력 데이터 없음
      NULL,          // [Data1 Pointer] NULL: 입력 데이터 없음
      0,             // [Data2 Length] 0: 입력 데이터 없음
      NULL,          // [Data2 Pointer] NULL: 입력 데이터 없음
      0,             // [Data3 Length] 0: 입력 데이터 없음
      NULL,          // [Data3 Pointer] NULL: 입력 데이터 없음
      0,             // [Data4 Length] 0: 입력 데이터 없음
      NULL,          // [Data4 Pointer] NULL: 입력 데이터 없음
      tx_buffer,     // [TX Buffer] 송신 버퍼 포인터
      rx_buffer      // [RX Buffer] 수신 버퍼 포인터
  );

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    // 응답에서 랜덤 데이터 추출
    // rx_buffer[0]: Count
    // rx_buffer[1]: Status (Return Code)
    // rx_buffer[2]부터: 생성된 랜덤 데이터
    // Count includes: Count(1) + Status(1) + Data(N) + Checksum(2)
    // So Data length = Count - 4
    uint8_t data_length = rx_buffer[AES132_RESPONSE_INDEX_COUNT] -
                          AES132_RESPONSE_INDEX_DATA - AES132_CRC_SIZE;
    uint8_t copy_length = (data_length < length) ? data_length : length;

    for (uint8_t i = 0; i < copy_length; i++) {
      random_data[i] = rx_buffer[AES132_RESPONSE_INDEX_DATA + i];
    }

    return copy_length;
  }

  return 0;
}

/**
 * @brief 랜덤 바이트 배열의 통계적 분석
 *
 * 생성된 랜덤 데이터의 분포를 간단히 분석합니다.
 *
 * @param data 분석할 데이터
 * @param length 데이터 길이
 */
void analyzeRandomDistribution(const uint8_t *data, uint8_t length) {
  uint16_t sum = 0;
  uint8_t min_val = 0xFF;
  uint8_t max_val = 0x00;
  uint16_t even_count = 0;
  uint16_t odd_count = 0;

  for (uint8_t i = 0; i < length; i++) {
    sum += data[i];
    if (data[i] < min_val)
      min_val = data[i];
    if (data[i] > max_val)
      max_val = data[i];
    if (data[i] % 2 == 0) {
      even_count++;
    } else {
      odd_count++;
    }
  }

  Serial.println("=== Random Distribution Analysis ===");
  Serial.print("Length: ");
  Serial.println(length);
  Serial.print("Average: ");
  Serial.println((float)sum / length, 2);
  Serial.print("Min: 0x");
  Serial.println(min_val, HEX);
  Serial.print("Max: 0x");
  Serial.println(max_val, HEX);
  Serial.print("Even bytes: ");
  Serial.print(even_count);
  Serial.print(" (");
  Serial.print((float)even_count * 100 / length, 1);
  Serial.println("%)");
  Serial.print("Odd bytes: ");
  Serial.print(odd_count);
  Serial.print(" (");
  Serial.print((float)odd_count * 100 / length, 1);
  Serial.println("%)");
  Serial.println();
}

/**
 * @brief 디바이스 Lock 상태 확인
 *
 * Config Zone의 LockConfig 레지스터를 읽어 디바이스의 잠금 상태를 확인합니다.
 */
void checkDeviceLockStatus() {
  Serial.println("=== Checking Device Lock Status ===");
  uint8_t config_data[4] = {0};
  uint8_t ret = aes132m_read_memory(4, 0xF020, config_data);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    uint8_t lock_config = config_data[0];

    Serial.print("LockConfig (0xF020): 0x");
    Serial.print(lock_config, HEX);

    if (lock_config == 0x55) {
      Serial.println(" (Unlocked - Development Mode)");
      Serial.println(
          "Warning: RNG may produce fixed patterns (0xA5) in this state.");
    } else if (lock_config == 0x00) {
      Serial.println(" (Locked - Production Mode)");
    } else {
      Serial.println(" (Unknown State)");
    }

    Serial.print("Raw Config Bytes: ");
    print_hex(NULL, config_data, 4);
  } else {
    Serial.print("Failed to read Lock Status. RetCode: 0x");
    Serial.println(ret, HEX);
  }
  Serial.println();
}

void setup(void) {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial) {
    delay(10);
  }

  // Allow time for serial monitor to connect
  delay(3000);

  Serial.println("\n========================================");
  Serial.println("ESP32 AES132 CryptoAuth Example");
  Serial.println("Example 04: Random Number Generation");
  Serial.println("========================================\n");

  // AES132 초기화
  uint8_t ret = aes132_init();
  if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.print("Failed to initialize AES132: 0x");
    Serial.println(ret, HEX);
    return;
  }

  Serial.println("AES132 initialized successfully\n");

  // Lock 상태 확인
  checkDeviceLockStatus();

  // 예제 1: 단일 랜덤 바이트 생성
  Serial.println("=== Example 1: Generate Single Random Byte ===");
  uint8_t random_byte = 0;
  uint8_t bytes_generated = generateRandom(&random_byte, 1);

  if (bytes_generated > 0) {
    Serial.print("Random byte: 0x");
    Serial.println(random_byte, HEX);
    Serial.print("Decimal: ");
    Serial.println(random_byte);
    Serial.print("Binary: ");
    for (int8_t i = 7; i >= 0; i--) {
      Serial.print((random_byte >> i) & 1);
    }
    Serial.println();
  } else {
    Serial.println("Failed to generate random byte");
    print_result("Random", ret);
  }
  Serial.println();

  // 예제 2: 여러 랜덤 바이트 생성 (16바이트)
  Serial.println("=== Example 2: Generate 16 Random Bytes ===");
  uint8_t random_data[16] = {0};
  bytes_generated = generateRandom(random_data, 16);

  if (bytes_generated > 0) {
    Serial.print("Successfully generated ");
    Serial.print(bytes_generated);
    Serial.println(" random bytes:");
    print_hex("Random data: ", random_data, bytes_generated);
    Serial.println();
  } else {
    Serial.println("Failed to generate random data");
    print_result("Random", ret);
  }
  Serial.println();

  // 예제 3: 연속 랜덤 생성 및 통계 분석
  Serial.println("=== Example 3: Generate Multiple Random Sequences ===");
  uint8_t random_seq1[32] = {0};
  uint8_t random_seq2[32] = {0};
  uint8_t random_seq3[32] = {0};

  Serial.println("Generating 3 sequences of 32 random bytes each...\n");

  bytes_generated = generateRandom(random_seq1, 32);
  if (bytes_generated > 0) {
    Serial.println("Sequence 1:");
    print_hex("  ", random_seq1, bytes_generated);
    Serial.println();
  }

  delay(100); // 약간의 지연

  bytes_generated = generateRandom(random_seq2, 32);
  if (bytes_generated > 0) {
    Serial.println("Sequence 2:");
    print_hex("  ", random_seq2, bytes_generated);
    Serial.println();
  }

  delay(100); // 약간의 지연

  bytes_generated = generateRandom(random_seq3, 32);
  if (bytes_generated > 0) {
    Serial.println("Sequence 3:");
    print_hex("  ", random_seq3, bytes_generated);
    Serial.println();
  }

  // 세 시퀀스가 모두 다른지 확인
  bool all_different = true;
  for (uint8_t i = 0; i < 32; i++) {
    if (random_seq1[i] == random_seq2[i] && random_seq2[i] == random_seq3[i]) {
      all_different = false;
      break;
    }
  }

  if (all_different) {
    Serial.println("✓ All sequences are different (good randomness)");
  } else {
    Serial.println("⚠ Warning: Some sequences have identical bytes");
  }
  Serial.println();

  // 예제 4: 통계적 분석
  Serial.println("=== Example 4: Statistical Analysis ===");
  uint8_t large_random[64] = {0};
  bytes_generated = generateRandom(large_random, 32);
  if (bytes_generated > 0) {
    // 두 번 생성하여 64바이트 확보
    delay(50);
    uint8_t temp[32] = {0};
    uint8_t bytes2 = generateRandom(temp, 32);
    if (bytes2 > 0) {
      for (uint8_t i = 0; i < 32; i++) {
        large_random[32 + i] = temp[i];
      }
      bytes_generated = 64;
    }
  }

  if (bytes_generated >= 32) {
    print_hex("Random data (64 bytes): ", large_random, bytes_generated);
    Serial.println();
    analyzeRandomDistribution(large_random, bytes_generated);
  }

  Serial.println("=== Random Generation Examples Complete ===");
  Serial.println(
      "\nNote: Hardware RNG provides cryptographically secure random numbers");
  Serial.println(
      "      suitable for encryption keys, nonces, and authentication.");
}

void loop(void) { delay(1000); }
