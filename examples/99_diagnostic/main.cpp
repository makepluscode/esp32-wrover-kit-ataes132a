/**
 * @file main.cpp
 * @brief Example 99: Chip Repair & Initialization Tool
 *
 * Diagnostic and repair tool for ATAES132A.
 * 1. Checks Lock State (Config Memory & Key Memory).
 * 2. Fixes ChipConfig to Default (0xC3) to enable Encryption.
 * 3. Configures Key Slots 0 and 1 with Permissive Auth (0x0D).
 * 4. Loads Test Keys into Slots 0 and 1 using Direct Write (for Unlocked
 * Chips).
 */

#include "aes132_comm_marshaling.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>
#include <Wire.h>

// --- Constants & Addresses ---
#define ADDR_LOCK_CONFIG 0xF020
#define ADDR_CHIP_CONFIG 0xF040
#define ADDR_KEY_CONFIG 0xF080
#define ADDR_KEY_0_DIRECT 0xF200
#define ADDR_KEY_1_DIRECT 0xF210

// ChipConfig: 0xC3 (Default) -> Enables Encrypt/Decrypt/Legacy
#define CHIP_CONFIG_DEFAULT 0xC3

// KeyConfig: 0x0D (External Crypto Allowed, Inbound Auth Disabled)
// Bit 3 (Legacy): 1, Bit 2 (Random): 0 (Disable to allow Fixed Nonce testing)
// KeyConfig: 0x09 00 00 00 (Random Nonce Required 비트 해제)
// 0x09 = 0000 1001 (Link=1, Random=0, Legacy=1)
// Fixed Nonce를 사용하기 위해 Random Bit를 0으로 설정함.
// 이는 Example 07 루프백 테스트에 필수적임.
const uint8_t keyConfig[4] = {0x09, 0x00, 0x00, 0x00};

// Test Key (0x00..0x0F)
const uint8_t TEST_KEY[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                              0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

// --- Helper Functions ---

void print_header(const char *title) {
  Serial.println("\n--------------------------------------------");
  Serial.println(title);
  Serial.println("--------------------------------------------");
}

uint8_t read_4bytes(uint16_t addr, uint8_t *data) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];
  // [AES132 Datasheet 8.4 BlockRead Command]
  // OpCode: 0x02 (AES132_BLOCK_READ) -> 블록 읽기
  // Mode: 0 -> 메모리 직접 읽기
  // Param1: Address -> 읽을 주소 (4바이트 단위)
  // Param2: Length (4) -> 읽을 길이
  uint8_t ret = aes132m_execute(
      AES132_BLOCK_READ, // [OpCode] BlockRead (0x02): 블록 읽기 명령
      0,                 // [Mode] 0: 메모리 직접 읽기
      addr,              // [Param1] Address: 읽을 주소 (4바이트)
      4,                 // [Param2] Length: 읽을 데이터 길이
      0,                 // [Data1 Length] 0: 입력 데이터 없음
      NULL,              // [Data1 Pointer] NULL: 입력 데이터 없음
      0,                 // [Data2 Length] 0: 입력 데이터 없음
      NULL,              // [Data2 Pointer] NULL: 입력 데이터 없음
      0,                 // [Data3 Length] 0: 입력 데이터 없음
      NULL,              // [Data3 Pointer] NULL: 입력 데이터 없음
      0,                 // [Data4 Length] 0: 입력 데이터 없음
      NULL,              // [Data4 Pointer] NULL: 입력 데이터 없음
      tx_buf,            // [TX Buffer] 송신 버퍼 포인터
      rx_buf             // [RX Buffer] 수신 버퍼 포인터
  );
  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    memcpy(data, &rx_buf[AES132_RESPONSE_INDEX_DATA], 4);
  }
  return ret;
}

uint8_t write_config_4bytes(uint16_t addr, const uint8_t *data) {
  return aes132m_write_memory(4, addr, (uint8_t *)data);
}

// Check Lock State and return safe-to-write status
bool check_and_print_lock_state() {
  uint8_t buf[4];
  if (read_4bytes(ADDR_LOCK_CONFIG, buf) != 0) {
    Serial.println("Error: Failed to read LockConfig.");
    return false;
  }

  Serial.print("LockConfig (0xF020): ");
  print_hex("", buf, 4);

  uint8_t config_lock = buf[2];
  bool unlocked = (config_lock == 0x55);

  Serial.print("Status: ");
  if (unlocked) {
    Serial.println("UNLOCKED (Config Writable, Key Direct Write Required)");
  } else {
    Serial.println("LOCKED (Config Immutable, KeyLoad Command Required)");
  }
  return unlocked;
}

void configure_chip() {
  uint8_t buf[4];
  if (read_4bytes(ADDR_CHIP_CONFIG, buf) == 0) {
    Serial.print("Current ChipConfig: ");
    print_hex("", buf, 4);
    if (buf[0] != CHIP_CONFIG_DEFAULT) {
      Serial.println("-> Updating ChipConfig to 0xC3 (Enable Encryption)...");
      buf[0] = CHIP_CONFIG_DEFAULT;
      if (write_config_4bytes(ADDR_CHIP_CONFIG, buf) == 0) {
        Serial.println("-> Success: ChipConfig Updated.");
      } else {
        Serial.println("-> Error: Failed to update ChipConfig.");
      }
    } else {
      Serial.println("-> ChipConfig is correct.");
    }
  }
}

void configure_key_slots() {
  // Slot 0
  Serial.println("Configuring Slot 0 (0xF080)...");
  if (write_config_4bytes(ADDR_KEY_CONFIG, KEY_CONFIG_OPEN) == 0) {
    Serial.println("-> Slot 0 Config Updated (0x0D...).");
  } else {
    Serial.println("-> Error: Failed to update Slot 0.");
  }

  // Slot 1
  Serial.println("Configuring Slot 1 (0xF084)...");
  if (write_config_4bytes(ADDR_KEY_CONFIG + 4, KEY_CONFIG_OPEN) == 0) {
    Serial.println("-> Slot 1 Config Updated (0x0D...).");
  } else {
    Serial.println("-> Error: Failed to update Slot 1.");
  }
}

void load_keys_direct() {
  // Slot 0
  Serial.println("Loading Key 0 (Direct Write to 0xF200)...");
  if (aes132m_write_memory(16, ADDR_KEY_0_DIRECT, (uint8_t *)TEST_KEY) == 0) {
    Serial.println("-> Success: Key 0 Loaded.");
  } else {
    Serial.println("-> Error: Failed to load Key 0.");
  }

  // Slot 1
  Serial.println("Loading Key 1 (Direct Write to 0xF210)...");
  if (aes132m_write_memory(16, ADDR_KEY_1_DIRECT, (uint8_t *)TEST_KEY) == 0) {
    Serial.println("-> Success: Key 1 Loaded.");
  } else {
    Serial.println("-> Error: Failed to load Key 1.");
  }
}

uint8_t read_16bytes(uint16_t addr, uint8_t *data) {
  uint8_t tx_buf[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buf[AES132_RESPONSE_SIZE_MAX];
  // [AES132 Datasheet 8.4 BlockRead Command]
  // OpCode: 0x02 (AES132_BLOCK_READ) -> 블록 읽기
  // Mode: 0 -> 메모리 직접 읽기
  // Param1: Address -> 읽을 주소 (16바이트 단위)
  // Param2: Length (16) -> 읽을 길이
  uint8_t ret = aes132m_execute(
      AES132_BLOCK_READ, // [OpCode] BlockRead (0x02): 블록 읽기 명령
      0,                 // [Mode] 0: 메모리 직접 읽기
      addr,              // [Param1] Address: 읽을 주소 (16바이트)
      16,                // [Param2] Length: 읽을 데이터 길이
      0,                 // [Data1 Length] 0: 입력 데이터 없음
      NULL,              // [Data1 Pointer] NULL: 입력 데이터 없음
      0,                 // [Data2 Length] 0: 입력 데이터 없음
      NULL,              // [Data2 Pointer] NULL: 입력 데이터 없음
      0,                 // [Data3 Length] 0: 입력 데이터 없음
      NULL,              // [Data3 Pointer] NULL: 입력 데이터 없음
      0,                 // [Data4 Length] 0: 입력 데이터 없음
      NULL,              // [Data4 Pointer] NULL: 입력 데이터 없음
      tx_buf,            // [TX Buffer] 송신 버퍼 포인터
      rx_buf             // [RX Buffer] 수신 버퍼 포인터
  );
  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    memcpy(data, &rx_buf[AES132_RESPONSE_INDEX_DATA], 16);
  }
  return ret;
}

void verify_keys() {
  uint8_t buf[16];

  // Verify Key 0
  Serial.println("Verifying Key 0 (0xF200)...");
  uint8_t ret = read_16bytes(ADDR_KEY_0_DIRECT, buf);
  if (ret == 0) {
    if (memcmp(buf, TEST_KEY, 16) == 0) {
      Serial.println("-> Match! Key 0 verified.");
    } else {
      Serial.println("-> Mismatch! Read data:");
      print_hex("", buf, 16);
    }
  } else {
    Serial.print("-> Error: Failed to read Key 0. RetCode: 0x");
    Serial.println(ret, HEX);
  }

  // Verify Key 1
  Serial.println("Verifying Key 1 (0xF210)...");
  ret = read_16bytes(ADDR_KEY_1_DIRECT, buf);
  if (ret == 0) {
    if (memcmp(buf, TEST_KEY, 16) == 0) {
      Serial.println("-> Match! Key 1 verified.");
    } else {
      Serial.println("-> Mismatch! Read data:");
      print_hex("", buf, 16);
    }
  } else {
    Serial.print("-> Error: Failed to read Key 1. RetCode: 0x");
    Serial.println(ret, HEX);
  }
}

void dump_key_config_table() {
  print_header("Final Verification: KeyConfig Table");
  Serial.println("Slot | KeyConfig");
  Serial.println("-----|-----------");
  uint8_t buf[4];
  for (int i = 0; i < 16; i++) {
    if (read_4bytes(ADDR_KEY_CONFIG + (i * 4), buf) == 0) {
      if (i < 10)
        Serial.print(" ");
      Serial.print(i);
      Serial.print("  | ");
      print_hex("", buf, 4);
    }
    delay(10);
  }
}

// --- Main Setup ---

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println("\n\n=== ATAES132A Repair & Init Tool ===");

  // Initialize I2C
  i2c_enable_phys();

  // --- I2C Scanner Block ---
  Serial.println("Performing I2C Scan (Pre-Wakeup)...");
  int nDevices = 0;
  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found.");

  // Try to wake up
  aes132c_wakeup();
  delay(200);

  Serial.println("Performing I2C Scan (Post-Wakeup)...");
  nDevices = 0;
  for (byte address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found.");
  // -------------------------

  print_header("Step 1: Diagnostics");
  bool unlocked = check_and_print_lock_state();

  if (unlocked) {
    print_header("Step 2: Chip Configuration");
    configure_chip();

    print_header("Step 3: Key Slot Configuration");
    configure_key_slots();

    print_header("Step 4: Key Loading (Direct Write)");
    load_keys_direct();

    print_header("Step 5: Load Verification");
    verify_keys();
  } else {
    Serial.println("\n[WARNING] Chip is LOCKED. Skipping Write operations.");
    Serial.println("If KeyConfig allows, run Example 6 directly.");
  }

  dump_key_config_table();

  Serial.println("\nDone. Chip is ready for Example 6.");
}

void loop() { delay(1000); }
