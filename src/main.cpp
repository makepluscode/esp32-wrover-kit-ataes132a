/**
 * @file main.cpp
 * @brief ESP32 AES132 CryptoAuth Example - Main Entry Point
 * 
 * 이 파일은 기본 템플릿입니다.
 * 예제를 실행하려면 이 파일을 examples/XX_example_name/main.cpp로 교체하거나
 * 스크립트를 사용하여 예제를 선택하세요.
 * 
 * 예제 선택 방법:
 * - 스크립트: .\scripts\select_example.ps1 <예제번호>
 * - 수동: examples\XX_example_name\main.cpp를 이 파일로 복사
 */

#include <Arduino.h>

void setup(void) {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println("\n========================================");
    Serial.println("Hello ATAES132A");
    Serial.println("========================================");
    Serial.println("\n이 파일은 기본 템플릿입니다.");
    Serial.println("예제를 실행하려면 예제를 선택하세요:");
    Serial.println("  .\\scripts\\select_example.ps1 <번호>");
    Serial.println("\n사용 가능한 예제: 1-10");
}

void loop(void) {
    delay(1000);
}
