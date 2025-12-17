/**
 * @file main.cpp
 * @brief 예제 9: 인증 (Authentication) - FIXED
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <string.h>

// Example 08에서 설정한 Key ID (Slot 2)
#define AUTH_KEY_ID 2

// Key Value (Example 08에서 사용한 값과 동일해야 함)
const uint8_t KEY_VALUE[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                               0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

/**
 * @brief AES132 Nonce 명령 전송 (Random Mode)
 * [FIXED] 20바이트 InSeed 추가
 */
uint8_t generateNonce(uint8_t *nonce_out) {
  uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

  // [FIX] Try 12 bytes seed (similar to Example 06)
  uint8_t in_seed[12];
  memset(in_seed, 0x00, 12);

  // [AES132 Datasheet 7.11 Nonce Command]
  // OpCode: 0x11 (Nonce)
  // Mode: 0x00 (Random Nonce)
  // Param1: 0x0000
  // Param2: 0x0000
  // Data: in_seed (20 bytes) <-- 필수 파라미터 추가됨

  // 라이브러리 실행 함수 호출
  // (DataLength=12)
  uint8_t ret =
      aes132m_execute(AES132_NONCE, 0x00, 0x0000, 0x0000, 12,
                      in_seed, // DataLength=12
                      0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

  // [DEBUG] 칩이 뭐라고 대답했는지 원본 데이터를 찍어봅시다.
  Serial.print("DEBUG RX: ");
  for (int i = 0; i < 8; i++) { // 앞부분 8바이트만 확인
    Serial.print(rx_buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    // Response: Count(1) + Status(1) + Random(12) + CRC(2)
    // Data Index: 2
    if (rx_buffer[AES132_RESPONSE_INDEX_COUNT] >= 16) {
      memcpy(nonce_out, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 12);
      return AES132_DEVICE_RETCODE_SUCCESS;
    }
  }
  return ret;
}

/**
 * @brief Auth 명령 수행 (Inbound Authentication)
 */
uint8_t performInboundAuth(uint8_t key_id) {
  uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

  Serial.println("Performing Inbound Authentication (Placeholder)...");

  // [추가] 장치가 자고 있을 수 있으므로 깨웁니다.
  // 사용 중인 라이브러리에 맞는 Wake 함수를 호출하세요.
  if (aes132c_wakeup() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("Warning: Device Wakeup failed or already awake.");
  }

  // 1. Generate Nonce first to setup TempKey
  uint8_t device_nonce[12];
  if (generateNonce(device_nonce) != AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.println("Error: Failed to generate Nonce");
    return 0xFF; // Fail
  }

  Serial.print("Device Nonce: ");
  for (int i = 0; i < 12; i++) {
    Serial.printf("%02X ", device_nonce[i]);
  }
  Serial.println();

  // 2. Calculate MAC (Host side)
  // 실제로는 여기서 AES-CCM 계산을 해야 하지만, 테스트를 위해 Dummy MAC 사용
  uint8_t host_mac[16];
  memset(host_mac, 0xAA, 16); // Dummy MAC

  // 3. Send Auth Command
  // OpCode: 0x03 (Auth)
  // Mode: 0x01 (Inbound Authentication)
  // Param1: Key ID
  // Param2: Usage (0x0000)
  // Data: MAC (16 bytes)
  uint8_t ret =
      aes132m_execute(AES132_AUTH, 0x01, key_id, 0x0000, 16,
                      host_mac, // DataLength=16, DataPtr=host_mac
                      0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

  return ret;
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n========================================");
  Serial.println("ESP32 AES132 CryptoAuth Example");
  Serial.println("Example 09: Authentication (Fixed)");
  Serial.println("========================================\n");

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("AES132 initialization failed");
    return;
  }
  Serial.println("AES132 initialized successfully");

  // 인증 시도
  Serial.print("\n=== Authenticating Key Slot ");
  Serial.print(AUTH_KEY_ID);
  Serial.println(" ===");

  uint8_t ret = performInboundAuth(AUTH_KEY_ID);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.println("\nSUCCESS: Authentication Passed!");
  } else {
    Serial.print("\nFAILED: Authentication Failed. RetCode: 0x");
    Serial.println(ret, HEX);

    if (ret == 0x40) {
      Serial.println("--------------------------------------------------");
      Serial.println("[OK] Expected Result: 0x40 (MacError)");
      Serial.println("The Nonce command succeeded, but Auth failed");
      Serial.println(
          "because we sent a dummy MAC. This confirms communication works.");
      Serial.println("--------------------------------------------------");
    } else if (ret == 0xFF) {
      Serial.println(
          "[Error] 0xFF: Still getting Parameter Error. Check Nonce Seed.");
    }
  }
}

void loop(void) { delay(1000); }
