/**
 * @file main.cpp
 * @brief 예제 7: AES132 복호화 (Decrypt) - AES-CCM Mode Fixed
 *
 * AES-CCM 모드 복호화 예제입니다.
 * 복호화를 위해서는 암호화 시 생성된 MAC(Tag)과 Ciphertext가 모두 필요합니다.
 *
 * Flow:
 * 1. Setup Chip (Init)
 * 2. Generate Nonce (Random)
 * 3. Encrypt Data (Internal or Previous) -> Get MAC & Ciphertext
 * 4. Decrypt Data -> Verify Plaintext
 */

#include "aes132_comm_marshaling.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>

// --- Configuration ---
#define KEY_SLOT_ID 0 // Uses Slot 0
#define BLOCK_SIZE 16

// --- Global: Stored Nonce (Buffer) ---
// Step 1에서 사용한 Nonce를 저장해 두었다가 Step 2에서 재사용하기 위한
// 버퍼입니다. 암호화 시 Nonce가 무효화되므로, 복호화 전에 반드시 동일한 Nonce를
// 다시 로드해야 합니다.
uint8_t stored_nonce[12];

// --- Helper: Load Nonce (Fixed Mode 0) ---
// 저장된(stored_nonce) 값을 Chip의 TempKey로 로드합니다.
bool load_nonce_fixed() {
  Serial.println("Loading Stored Nonce (Fixed Mode)...");
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // OpCode: 0x01 (Nonce), Mode: 0 (Fixed), Param1: 0 (Update Seed), Param2: 0
  // [AES132 Datasheet 8.11 Nonce Command]
  // OpCode: 0x01 (AES132_NONCE) -> Nonce 생성 명령
  // Mode: 0 (Fixed Nonce) -> 입력된 Seed를 그대로 Nonce로 사용하여 TempKey 갱신
  // Param1: 0 (Update Seed to TempKey)
  // Data1: Stored Nonce (12 bytes) -> 이전 단계에서 저장해둔 Nonce 값
  uint8_t ret = aes132m_execute(
      AES132_NONCE, // [OpCode] Nonce (0x01): Nonce 생성 명령
      0,            // [Mode] 0: Fixed Mode (입력 Seed를 Nonce로 사용)
      0,            // [Param1] 0: TempKey 업데이트
      0,            // [Param2] 0: Mode 0에서는 무시됨
      12,           // [Data1 Length] 12: Fixed Seed 길이
      stored_nonce, // [Data1 Pointer] Stored Nonce: 이전 저장된 Nonce 사용
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
    Serial.println("-> Nonce Load Success.");
    return true;
  }
  Serial.print("-> Nonce Load Failed: 0x");
  Serial.println(ret, HEX);
  return false;
}

// --- Helper: Encrypt Block (Needed for Self-Test) ---
bool encrypt_block(uint8_t key_id, const uint8_t *plaintext, uint8_t *out_mac,
                   uint8_t *out_ciphertext) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // [AES132 Datasheet 8.6 Encrypt Command]
  // OpCode: 0x06 (AES132_ENCRYPT) -> 암호화 명령
  // Mode: 0 (Normal Encryption) -> 일반 암호화 모드
  // Param1: KeyID
  // Data1: Plaintext (16 bytes)
  uint8_t ret = aes132m_execute(
      AES132_ENCRYPT,       // [OpCode] Encrypt (0x06): 암호화 명령
      0,                    // [Mode] 0: Normal Encryption (일반 암호화 모드)
      key_id,               // [Param1] KeyID: 암호화 키 슬롯 번호
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
    // [Count][Status][MAC(16)][Cipher(16)][CRC]
    memcpy(out_mac, &rx_buf[AES132_RESPONSE_INDEX_DATA], 16);
    memcpy(out_ciphertext, &rx_buf[AES132_RESPONSE_INDEX_DATA + 16], 16);
    return true;
  }
  return false;
}

// --- Helper: Decrypt Block (Corrected for AES-CCM) ---
// Requires: KeyID, MAC(16), Ciphertext(16)
// Returns: Plaintext(16)
bool decrypt_block(uint8_t key_id, const uint8_t *mac,
                   const uint8_t *ciphertext, uint8_t *out_plaintext) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];

  // Decrypt Command
  // Data 1: Ciphertext (16 bytes)
  // Data 2: MAC (16 bytes)
  // Param 2: 16 (Result Length)
  // [AES132 Datasheet 8.5 Decrypt Command]
  // OpCode: 0x07 (AES132_DECRYPT) -> 복호화 명령
  // Mode: 0 (Normal Decryption) -> 일반 복호화 모드
  // Param1: KeyID (복호화 키 슬롯)
  // Param2: Result Length (16 bytes)
  // Data1: Ciphertext (16 bytes)
  // Data2: MAC (16 bytes)
  // 참고: Loopback 테스트를 위해 암호화 결과(Cipher+MAC)를 입력으로 사용함
  uint8_t ret = aes132m_execute(
      AES132_DECRYPT, // [OpCode] Decrypt (0x07): 복호화 명령
      0,              // [Mode] 0: Normal Decryption (일반 복호화 모드)
      key_id,         // [Param1] KeyID: 복호화 키 슬롯 번호
      BLOCK_SIZE,     // [Param2] Result Length: 결과 데이터 길이 (16 bytes)
      BLOCK_SIZE,     // [Data1 Length] 16: 입력 데이터 1 (Ciphertext) 길이
      (uint8_t *)ciphertext, // [Data1 Pointer] Ciphertext: 암호문 포인터
      BLOCK_SIZE,            // [Data2 Length] 16: 입력 데이터 2 (MAC) 길이
      (uint8_t *)
          mac, // [Data2 Pointer] MAC: 인증 태그 포인터 (Data order confirmed)
      0,       // [Data3 Length] 0: 입력 데이터 없음
      NULL,    // [Data3 Pointer] NULL: 입력 데이터 없음
      0,       // [Data4 Length] 0: 입력 데이터 없음
      NULL,    // [Data4 Pointer] NULL: 입력 데이터 없음
      tx_buf,  // [TX Buffer] 송신 버퍼 포인터
      rx_buf   // [RX Buffer] 수신 버퍼 포인터
  );

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    uint8_t count = rx_buf[AES132_RESPONSE_INDEX_COUNT];
    if (count >= 20) { // Count(1)+Status(1)+Data(16)+CRC(2)
      memcpy(out_plaintext, &rx_buf[AES132_RESPONSE_INDEX_DATA], 16);
      return true;
    }
  }

  Serial.print("[Decrypt] Failed. Code: 0x");
  Serial.println(ret, HEX);
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n=== Example 07: AES Decryption (AES-CCM) ===");

  // Force Wakeup
  i2c_enable_phys();
  aes132c_wakeup();
  delay(200);

  if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("Warning: Init failed, retrying...");
    aes132c_wakeup();
    if (aes132_init() != AES132_FUNCTION_RETCODE_SUCCESS) {
      Serial.println("Error: I2C Init Failed.");
      return;
    }
  }

  // 0. Prepare Stored Nonce (Fixed Pattern)
  for (int i = 0; i < 12; i++)
    stored_nonce[i] = i;

  // 1. Load Nonce for Encryption
  if (!load_nonce_fixed())
    return;

  // 2. Encrypt
  Serial.println("\n--- Step 1: Encrypting Data ---");
  uint8_t original[16] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x41, 0x45,
                          0x53, 0x31, 0x33, 0x32, 0x21, 0x00, 0x00, 0x00};
  uint8_t mac[16];
  uint8_t ciphertext[16];
  uint8_t decrypted[16];

  Serial.print("Original: ");
  print_hex("", original, 16);

  if (!encrypt_block(KEY_SLOT_ID, original, mac, ciphertext)) {
    Serial.println("Encrypt failed.");
    return;
  }
  Serial.print("MAC:    ");
  print_hex("", mac, 16);
  Serial.print("Cipher: ");
  print_hex("", ciphertext, 16);

  // 3. Load Nonce for Decryption (MUST be same as above)
  Serial.println("\n--- Step 2: Decrypting Data ---");
  // [CRITICAL] Encrypt invalidated the nonce. Reload it.
  if (!load_nonce_fixed())
    return;

  Serial.println("Sending Decrypt Command (Cipher + MAC)...");

  // Data Order: Cipher then MAC (per User Instruction)
  if (decrypt_block(KEY_SLOT_ID, mac, ciphertext, decrypted)) {
    Serial.println("-> Decrypt SUCCESS!");
    Serial.print("Decrypted: ");
    print_hex("", decrypted, 16);
    if (memcmp(original, decrypted, 16) == 0)
      Serial.println("MATCH!");
    else
      Serial.println("MISMATCH!");
  } else {
    Serial.println("-> Decrypt FAILED.");
  }
}

void loop() { delay(1000); }
