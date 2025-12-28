/**
 * @file main.cpp
 * @brief 예제 31: ReadOnly 권한 제어 및 검증 테스트
 *
 * 이 예제는 User Zone 0의 쓰기 권한을 Read/Write에서 Read-Only로 변경하고,
 * 실제로 쓰기가 차단되는지 보안 동작을 검증합니다.
 */

#include "aes132_comm_marshaling.h"
#include "aes132_config.h"
#include "aes132_utils.h"
#include "i2c_phys.h"
#include <Arduino.h>

// Zone 0 설정 주소 (4바이트)
#define ZONE0_CONFIG_ADDR 0xF0C0
// User Zone 0 데이터 시작 주소
#define ZONE0_DATA_ADDR 0x0000

/**
 * @brief BlockRead 명령어를 사용하여 특정 주소의 메모리 읽기 (Mode 1: Direct
 * Address)
 */
uint8_t blockReadDirect(uint16_t address, uint8_t length, uint8_t *data) {
  uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX];
  uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];

  // BlockRead 명령어 실행 (Mode 1: Direct Addressing)
  uint8_t ret = aes132m_execute(
      0x10,    // [OpCode] BlockRead (AES132_BLOCK_READ)
      0x01,    // [Mode] 1: Direct Memory Access (Address in Param1)
      address, // [Param1] Address
      length,  // [Param2] Length
      0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

  if (ret == AES132_DEVICE_RETCODE_SUCCESS) {
    uint8_t data_length = rx_buffer[0] - 3;
    uint8_t copy_length = (data_length < length) ? data_length : length;
    memcpy(data, &rx_buffer[2], copy_length);
    return copy_length;
  }
  return 0;
}

/**
 * @brief 표준 I2C Write를 사용하여 메모리 쓰기
 */
uint8_t i2cWriteDirect(uint16_t address, uint8_t length, uint8_t *data) {
  return aes132p_write_memory_physical(length, address, data);
}

void setup(void) {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial) {
    delay(10);
  }

  Serial.println("\n========================================");
  Serial.println("ESP32 AES132 CryptoAuth Example");
  Serial.println("Example 31: ReadOnly Test (Zone 0)");
  Serial.println("========================================\n");

  // AES132 초기화
  uint8_t ret = aes132_init();
  if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.print("Failed to initialize AES132: 0x");
    Serial.println(ret, HEX);
    return;
  }

  Serial.println("Step 1: 초기 확인 - Zone 0 설정 읽기");
  uint8_t zone_config[4] = {0};
  uint8_t bytes_read = blockReadDirect(ZONE0_CONFIG_ADDR, 4, zone_config);
  if (bytes_read == 4) {
    print_hex("Current ZoneConfig[0]: ", zone_config, 4);
  } else {
    Serial.println("Failed to read ZoneConfig[0]");
    return;
  }
  Serial.println();

  Serial.println("Step 2: RW 활성화 - WriteMode=10b(0x20), ReadOnly=0x55 설정");
  // Byte 0: Bit 5:4 = 10b (WriteMode=2) -> 0x20
  // Byte 3: ReadOnly = 0x55 (Enabled R/W)
  zone_config[0] = 0x20;
  zone_config[3] = 0x55;
  print_hex("Writing to ZoneConfig[0]: ", zone_config, 4);
  ret = i2cWriteDirect(ZONE0_CONFIG_ADDR, 4, zone_config);
  print_result("Set RW Mode", ret);

  // 확인을 위해 다시 읽기
  uint8_t verify_config[4] = {0};
  blockReadDirect(ZONE0_CONFIG_ADDR, 4, verify_config);
  print_hex("Verified ZoneConfig[0]: ", verify_config, 4);
  delay(50); // EEPROM 쓰기 대기
  Serial.println();

  Serial.println("Step 3: 데이터 쓰기 테스트 - Zone 0 (0x0000)에 데이터 쓰기");
  uint8_t test_data[16] = {0xAA, 0xBB, 0xCC, 0xDD, 0x11, 0x22, 0x33, 0x44,
                           0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xFF};
  print_hex("Data to write: ", test_data, 16);
  ret = i2cWriteDirect(ZONE0_DATA_ADDR, 16, test_data);
  print_result("Write Data", ret);

  uint8_t verify_data[16] = {0};
  blockReadDirect(ZONE0_DATA_ADDR, 16, verify_data);
  print_hex("Data read back: ", verify_data, 16);
  if (memcmp(test_data, verify_data, 16) == 0) {
    Serial.println("Data Verification: SUCCESS (Match)");
  } else {
    Serial.println("Data Verification: FAILED (Mismatch)");
  }
  Serial.println();

  Serial.println("Step 4: ReadOnly 변경 - ReadOnly=0x00 설정");
  uint8_t ro_val = 0x00;
  Serial.println("Setting ReadOnly byte to 0x00...");
  ret = i2cWriteDirect(ZONE0_CONFIG_ADDR + 3, 1,
                       &ro_val); // ReadOnly 바이트만 수정
  print_result("Set ReadOnly Mode", ret);

  // 확인을 위해 다시 읽기
  blockReadDirect(ZONE0_CONFIG_ADDR, 4, verify_config);
  print_hex("Verified ZoneConfig[0] (ReadOnly): ", verify_config, 4);
  delay(50);
  Serial.println();

  Serial.println("Step 5: 차단 검증 - 쓰기 금지 동작 확인");
  uint8_t fail_data[16];
  memset(fail_data, 0xEE, 16);

  Serial.println("Attempting to write 0xEE to Zone 0 (Read-Only state)...");
  print_hex("Sent Data (Write Attempt): ", fail_data, 16);
  ret = i2cWriteDirect(ZONE0_DATA_ADDR, 16, fail_data);

  // Status register 읽어서 EERR 비트 확인
  uint8_t status = 0;
  aes132c_read_device_status_register(&status);

  Serial.print("Return Code (I2C): 0x");
  Serial.print(ret, HEX);
  Serial.println(ret == 0 ? " (SUCCESS - I2C ACK)" : " (FAILED)");

  Serial.print("Device Status Register: 0x");
  if (status < 0x10)
    Serial.print("0");
  Serial.print(status, HEX);
  Serial.println((status & 0x20) ? " (EERR Bit SET - Hardware Error Detected)"
                                 : " (EERR Bit NOT SET)");

  // 데이터가 실제로 바뀌지 않았는지 확인 (가장 확실한 증거)
  uint8_t final_data[16] = {0};
  blockReadDirect(ZONE0_DATA_ADDR, 16, final_data);
  print_hex("Memory content after write attempt: ", final_data, 16);

  bool is_blocked = (memcmp(test_data, final_data, 16) == 0);

  if (is_blocked) {
    Serial.println(
        "Verification Result: SUCCESS (Write was Blocked by Hardware)");
    Serial.println("-> Data remains unchanged as expected.");
  } else {
    Serial.println("Verification Result: FAILED (Write was NOT blocked!)");
    Serial.println("-> Data was modified despite Read-Only setting.");
  }

  Serial.println("\n=== ReadOnly Test Complete ===");
}

void loop(void) { delay(1000); }
