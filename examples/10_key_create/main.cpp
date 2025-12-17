/**
 * @file main.cpp
 * @brief 예제 10: 키 생성 (Key Create) - FIXED
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <string.h>

// 테스트할 대상 키 슬롯 (4번)
#define TARGET_KEY_ID 4

// KeyConfig 레지스터 시작 주소 (0xF020)
#define KEY_CONFIG_ADDR_BASE 0xF020

/**
 * @brief KeyConfig 설정 함수 (슬롯 권한 부여)
 */
uint8_t configureKeySlot(uint8_t key_id) {
  Serial.print("Configuring KeyConfig for Slot ");
  Serial.println(key_id);

  // 설정값: Permissive (누구나 쓰고 읽기 가능, 테스트용)
  // Byte 0-1: 0x5555 (R/W 허용)
  // Byte 2:   0x55   (기타 옵션 허용, Create 허용 포함)
  // Byte 3:   0xFF   (링크 없음)
  uint8_t config_data[4] = {0x55, 0x55, 0x55, 0xFF};

  // 주소 계산: Base + (Slot * 4)
  uint16_t addr = KEY_CONFIG_ADDR_BASE + (key_id * 4);

  // Use aes132m_write_memory helper which maps to BlockWrite or EncWrite
  // This avoids defining OpCodes manually and handles packet formatting
  uint8_t ret = aes132m_write_memory(4, addr, config_data);

  return ret;
}

/**
 * @brief KeyCreate 명령 수행
 */
uint8_t createNewKey(uint8_t key_id) {
  uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];
  uint8_t ret_code;

  Serial.println("\n=== Step: Create New Key ===");
  Serial.print("Target Key Slot: ");
  Serial.println(key_id);

  // [AES132 Datasheet 7.9 KeyCreate Command]
  // OpCode: 0x08 (AES132_KEY_CREATE)
  // Mode: 0x00 (Create random key using internal RNG)
  ret_code = aes132m_execute(AES132_KEY_CREATE,
                             0x00,    // Mode: Random Key Generation
                             key_id,  // Param1: KeyID
                             0x0000,  // Param2: Usage Counter (0)
                             0, NULL, // Data: None for Mode 0
                             0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

  if (ret_code == AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.println("SUCCESS: Key Created Successfully!");
  } else {
    Serial.print("FAILED: Key Create Error. RetCode: 0x");
    Serial.println(ret_code, HEX);

    // 추가 디버깅
    if (ret_code == 0x50) {
      Serial.println("Error 0x50: Parse Error. Check Slot Configuration.");
    }
  }

  return ret_code;
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n========================================");
  Serial.println("ESP32 AES132 CryptoAuth Example");
  Serial.println("Example 10: Key Create (Fixed)");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 initialization failed");
    return;
  }
  Serial.println("AES132 initialized successfully");

  // [Step 1] 슬롯 4번 설정 (먼저 권한을 줘야 함)
  uint8_t cfg_ret = configureKeySlot(TARGET_KEY_ID);
  if (cfg_ret != AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.print("Config Write Failed: 0x");
    Serial.println(cfg_ret, HEX);
  } else {
    Serial.println("Key Slot Configured.");
  }

  delay(100); // 처리 시간 대기

  // [Step 2] 키 생성 시도
  createNewKey(TARGET_KEY_ID);
}

void loop(void) { delay(1000); }
