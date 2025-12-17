/**
 * @file main.cpp
 * @brief 예제 6: AES132 암호화 (Encrypt) - AES-CCM Mode
 *
 * AES-CCM 모드로 데이터를 암호화하고 MAC(인증 태그)을 생성합니다.
 * 이 예제는 I2C 주소(0xC2)와 Wakeup 시퀀스가 올바르게 적용되어야 성공합니다.
 *
 * Flow:
 * 1. I2C Init (Wakeup + Delay 필수)
 * 2. Generate Nonce (Random or Fixed)
 * 3. Encrypt Data -> Output MAC & Ciphertext
 * Both are required for decryption.
 *
 * Requires:
 * 1. Correct ChipConfig (0xC3) -> Enabled by Ex 99.
 * 2. Nonce Generation (Mode 1) -> Implemented here.
 * 3. Valid Key in Slot 0 -> Loaded by Ex 99.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <Wire.h>

// --- Configuration ---
#define KEY_SLOT_ID 0 // Uses Slot 0 (Configured by Tool 99 as 0x0D)
#define BLOCK_SIZE 16

// --- Helper: Encrypt Function ---
// Returns true on success
// OUT: out_mac (16 bytes), out_ciphertext (16 bytes)
bool encryptBlock(uint8_t key_id, const uint8_t *plaintext, uint8_t *out_mac,
                  uint8_t *out_ciphertext) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // Encrypt OpCode: 0x06, Mode: 0 (Encrypt)
  // Param1: KeyID, Param2: Data Length (16)
  // Response: [Count][Status][MAC(16)][Ciphertext(16)][CRC]
  // [AES132 Datasheet 8.6 Encrypt Command]
  // OpCode: 0x06 (AES132_ENCRYPT) -> 암호화 명령
  // Mode: 0 (Normal Encryption) -> 일반 암호화 모드 (Inbound)
  // Param1: KeyID -> 암호화에 사용할 키 슬롯 번호
  // Param2: Data Length -> 입력 데이터 길이 (16 bytes)
  // 결과: 칩은 입력된 평문을 암호화하고 MAC을 생성하여 반환합니다.
  // OutData: [Count][Status][MAC(16)][Ciphertext(16)][CRC]
  uint8_t ret = aes132m_execute(
      AES132_ENCRYPT,       // [OpCode] Encrypt (0x06): 암호화 명령
      0,                    // [Mode] 0: Normal Encryption (일반 암호화 모드)
      key_id,               // [Param1] KeyID: 암호화에 사용할 키 슬롯 번호
      BLOCK_SIZE,           // [Param2] Data Length: 입력 데이터 길이 (16 bytes)
      BLOCK_SIZE,           // [Data1 Length] 16: 입력 데이터(Plaintext) 길이
      (uint8_t *)plaintext, // [Data1 Pointer] Plaintext: 평문 데이터 포인터
      0,                    // [Data2 Length] 0: 입력 데이터 없음
      NULL,                 // [Data2 Pointer] NULL: 입력 데이터 없음
      0,                    // [Data3 Length] 0: 입력 데이터 없음
      NULL,                 // [Data3 Pointer] NULL: 입력 데이터 없음
      0,                    // [Data4 Length] 0: 입력 데이터 없음
      NULL,                 // [Data4 Pointer] NULL: 입력 데이터 없음
      tx_buf,               // [TX Buffer] 송신 버퍼 포인터
      rx_buf                // [RX Buffer] 수신 버퍼 포인터
  );

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    uint8_t count = rx_buf[AES132_RESPONSE_INDEX_COUNT];
    // Expected Length: 1 (Count) + 1 (Status) + 16 (MAC) + 16 (Ciphertext) + 2
    // (CRC) = 36 bytes
    if (count >= 36) {
      // Correct indexing:
      // Index 0: Count
      // Index 1: Status
      // Index 2~17: MAC (16 bytes)
      // Index 18~33: Ciphertext (16 bytes)
      memcpy(out_mac, &rx_buf[AES132_RESPONSE_INDEX_DATA], 16);
      memcpy(out_ciphertext, &rx_buf[AES132_RESPONSE_INDEX_DATA + 16], 16);
      return true;
    } else {
      Serial.print("[Encrypt] Response too short: ");
      Serial.println(count);
    }
  }

  Serial.print("[Encrypt] Failed. Code: 0x");
  Serial.println(ret, HEX);
  return false;
}

// --- Helper: Generate Nonce ---
// Required before Encryption in Unlocked/Testing mode
bool generateNonce() {
  Serial.println("Generating Nonce...");
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];
  uint8_t seed[12] = {0}; // Zero seed for Random Mode

  // OpCode: 0x01 (Nonce)
  // Mode: 1 (Random Nonce, required for KeyConfig 0x0D compatibility)
  // Param1: 0, Param2: 0
  // [AES132 Datasheet 8.11 Nonce Command]
  // OpCode: 0x01 (AES132_NONCE) -> Nonce 생성 명령
  // Mode: 1 (Random Nonce) -> 내부 RNG를 사용하여 Random Nonce 생성 및 TempKey
  // 갱신 Param1: 0 (Mode 1에서는 무시됨) Param2: 0 (Mode 1에서는 무시됨) Data1:
  // Seed (12 bytes) -> Random 모드에서도 호스트 측 Seed 입력이 필요함
  uint8_t ret =
      aes132m_execute(AES132_NONCE, // [OpCode] Nonce (0x01): Nonce 생성 명령
                      1,            // [Mode] 1: Random Mode (Random Nonce 생성)
                      0,            // [Param1] 0: Mode 1에서는 무시됨
                      0,            // [Param2] 0: Mode 1에서는 무시됨
                      12,           // [Data1 Length] 12: Host Seed 길이
                      seed,         // [Data1 Pointer] Seed: Host Seed 포인터
                      0,            // [Data2 Length] 0: 입력 데이터 없음
                      NULL,         // [Data2 Pointer] NULL: 입력 데이터 없음
                      0,            // [Data3 Length] 0: 입력 데이터 없음
                      NULL,         // [Data3 Pointer] NULL: 입력 데이터 없음
                      0,            // [Data4 Length] 0: 입력 데이터 없음
                      NULL,         // [Data4 Pointer] NULL: 입력 데이터 없음
                      tx_buf,       // [TX Buffer] 송신 버퍼 포인터
                      rx_buf        // [RX Buffer] 수신 버퍼 포인터
      );

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.println("-> Success.");
    return true;
  }

  Serial.print("-> Failed: 0x");
  Serial.println(ret, HEX);
  return false;
}

// --- Main Setup ---
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n=== Example 06: AES Encryption (AES-CCM) ===");

  // Force Wakeup before Init
  i2c_enable_phys();
  aes132c_wakeup();
  delay(200);

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("Warning: First Init attempt failed. Retrying...");
    delay(100);
    aes132c_wakeup();
    delay(100);
    // --- I2C Scanner & Debug ---
    Serial.println("Debugging: Performing I2C Scan...");
    int nDevices = 0;
    for (byte address = 1; address < 127; ++address) {
      Wire.beginTransmission(address);
      byte error = Wire.endTransmission();
      if (error == 0) {
        Serial.print(" -> I2C device found at address 0x");
        if (address < 16)
          Serial.print("0");
        Serial.print(address, HEX);
        Serial.println("  !");
        nDevices++;
      }
    }
    if (nDevices == 0)
      Serial.println(" -> No I2C devices found.");

    // Check Configured Address
    // Note: This requires extern access or we just assume it's correct from lib

    if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
      Serial.println("Error: I2C Init Failed.");
      return;
    }
  }

  // 1. Generate Nonce
  if (!generateNonce()) {
    Serial.println("Critical Error: Nonce generation failed.");
    return;
  }

  // 2. Encryption Test
  Serial.println("\n--- Test 1: Simple Data ---");
  uint8_t plaintext[16] = {0, 1, 2,  3,  4,  5,  6,  7,
                           8, 9, 10, 11, 12, 13, 14, 15};
  uint8_t mac[16];
  uint8_t ciphertext[16];

  Serial.print("Plaintext: ");
  print_hex("", plaintext, 16);

  if (encryptBlock(KEY_SLOT_ID, plaintext, mac, ciphertext)) {
    Serial.println("-> Encryption SUCCESS!");
    Serial.print("MAC:        ");
    print_hex("", mac, 16);
    Serial.print("Ciphertext: ");
    print_hex("", ciphertext, 16);

    Serial.println(
        "\n[IMPORTANT] Store BOTH MAC and Ciphertext for Decryption (Ex 07).");
  }

  Serial.println("\n--- Test 2: Text Data ---");
  char text_msg[] = "Hello AES132!   "; // 16 chars
  memcpy(plaintext, text_msg, 16);

  Serial.print("Text: \"");
  Serial.print(text_msg);
  Serial.println("\"");

  if (encryptBlock(KEY_SLOT_ID, plaintext, mac, ciphertext)) {
    Serial.println("-> Encryption SUCCESS!");
    Serial.print("MAC:        ");
    print_hex("", mac, 16);
    Serial.print("Ciphertext: ");
    print_hex("", ciphertext, 16);
  }
}

void loop() { delay(1000); }
