/**
 * @file main.cpp
 * @brief 예제: 51_readonly_test_locked - 잠금 후 Zone 0 읽기 전용 테스트
 *
 * 이 예제는 99번 예제로 칩이 잠긴 후, Zone 0(0x0000)이 읽기 전용으로 정확히
 * 동작하는지 확인합니다.
 * 1. Zone 0에서 16바이트를 읽습니다. (성공 기대)
 * 2. Zone 0에 16바이트 쓰기를 시도합니다. (실패 기대)
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>

void setup() {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial)
    delay(10);

  Serial.println("\n========================================");
  Serial.println("예제 51: 잠금 후 Zone 0 읽기 전용 테스트");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 초기화 실패");
    return;
  }

  uint8_t ret;
  uint8_t data[16];

  // 1. Zone 0 읽기 테스트
  Serial.println("[Step 1] Zone 0 (0x0000) 16바이트 읽기 시도...");
  ret = aes132m_read_memory(16, 0x0000, data);
  if (ret == 0) {
    Serial.print("-> 읽기 성공: ");
    print_hex("", data, 16);
  } else {
    Serial.printf("-> 읽기 실패: 0x%02X\n", ret);
  }

  Serial.println();

  // 2. Zone 0 쓰기 테스트
  Serial.println("[Step 2] Zone 0 (0x0000) 16바이트 쓰기 시도 (실패 기대)...");
  uint8_t dummy[16];
  for (int i = 0; i < 16; i++)
    dummy[i] = 0xEE;

  ret = aes132m_write_memory(16, 0x0000, dummy);
  if (ret != 0) {
    Serial.printf("-> 쓰기 거부됨 (정상): 0x%02X\n", ret);
    if (ret == 0x04 || ret == 0x08) {
      Serial.println("   (결과 분석: 칩이 잠겨있고 Read-only 정책이 잘 "
                     "작동하고 있습니다.)");
    }
  } else {
    Serial.println("-> !!! 오류: 쓰기가 성공했습니다. (칩이 잠기지 않았거나 "
                   "설정 오류 가능성) !!!");
  }

  Serial.println("\n테스트 종료.");
}

void loop() { delay(1000); }
