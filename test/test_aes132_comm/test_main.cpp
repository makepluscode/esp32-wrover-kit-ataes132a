#include "aes132_comm.h"
#include "aes132_comm_marshaling.h"
#include <Arduino.h>
#include <unity.h>


// Set up for tests
void setUp(void) {
  // Optional: Wake up device if needed for integration tests
}

void tearDown(void) {
  // Optional
}

/**
 * @brief Test CRC Calculation using known Sleep Command vector
 * Reference: aes132_comm.c -> aes132c_send_sleep_command
 * Data: 09 11 00 00 00 00 00
 * Expected CRC: 0x71 0x81
 */
void test_crc_calculation_sleep_command(void) {
  uint8_t data[] = {0x09, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t crc[2];

  // Calculate CRC for the first 7 bytes (Count through Data)
  aes132c_calculate_crc(7, data, crc);

  TEST_ASSERT_EQUAL_HEX8(0x71, crc[0]); // CRC High
  TEST_ASSERT_EQUAL_HEX8(0x81, crc[1]); // CRC Low
}

/**
 * @brief Test CRC Calculation for Standby Command
 * Reference: aes132_comm.c -> aes132c_send_sleep_command
 * Data: 09 11 40 00 00 00 00
 * Expected CRC: 0xEF 0x82
 */
void test_crc_calculation_standby_command(void) {
  uint8_t data[] = {0x09, 0x11, 0x40, 0x00, 0x00, 0x00, 0x00};
  uint8_t crc[2];

  aes132c_calculate_crc(7, data, crc);

  TEST_ASSERT_EQUAL_HEX8(0xEF, crc[0]);
  TEST_ASSERT_EQUAL_HEX8(0x82, crc[1]);
}

void setup() {
  delay(2000); // Wait for board to boot

  UNITY_BEGIN();

  RUN_TEST(test_crc_calculation_sleep_command);
  RUN_TEST(test_crc_calculation_standby_command);

  UNITY_END();
}

void loop() {}
