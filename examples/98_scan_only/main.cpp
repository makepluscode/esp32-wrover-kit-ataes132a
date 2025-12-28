/**
 * @file main.cpp
 * @brief 예제 98: I2C 버스 스캐너 (Scan Only)
 *
 * 이 예제는 I2C 버스를 스캔하여 연결된 모든 디바이스의 주소를 출력합니다.
 * 하드웨어 연결 확인 및 ATAES132A의 주소를 확인하는 데 유용합니다.
 */

#include "aes132_config.h"
#include <Arduino.h>
#include <Wire.h>


void setup() {
  Serial.begin(AES132_SERIAL_BAUD);
  while (!Serial) {
    delay(10);
  }

  Serial.println("\n========================================");
  Serial.println("ESP32 I2C Bus Scanner");
  Serial.println("Example 98: Scan Only");
  Serial.println("========================================\n");

  // I2C 초기화 (기본 핀 사용)
  Wire.begin(AES132_SDA_PIN, AES132_SCL_PIN);
  Serial.print("Scanning I2C bus (SDA: ");
  Serial.print(AES132_SDA_PIN);
  Serial.print(", SCL: ");
  Serial.print(AES132_SCL_PIN);
  Serial.println(")...");

  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.print("  (");
      Serial.print(address << 1, HEX);
      Serial.println(" in 8-bit format)");

      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.print("Scan complete. Found ");
    Serial.print(nDevices);
    Serial.println(" device(s).\n");
  }

  Serial.println("Scanning finished.");
}

void loop() {
  // 5초마다 재스캔
  delay(5000);
  Serial.println("Rescanning...");

  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println();
      nDevices++;
    }
  }
}
