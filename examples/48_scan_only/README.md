# 예제 48: I2C 버스 스캐너 (Scan Only)

이 예제는 I2C 버스를 스캔하여 연결된 모든 슬레이브 디바이스의 주소를 출력합니다.

## 목적
- 하드웨어 연결 상태 확인
- I2C 버스에 연결된 모든 디바이스 주소 확인
- ATAES132A 칩의 실제 I2C 주소(7비트 및 8비트 형식) 확인

## 실행 방법
1. 예제 98 선택 및 실행:
   ```powershell
   .\build.ps1 48 all
   ```

## 예상 출력
I2C 버스에 ATAES132A가 정상적으로 연결되어 있다면 다음과 유사한 출력을 볼 수 있습니다:

```text
========================================
ESP32 I2C Bus Scanner
Example 48: Scan Only
========================================

Scanning I2C bus (SDA: 21, SCL: 22)...
I2C device found at address 0x61  (C2 in 8-bit format)
Scan complete. Found 1 device(s).

Scanning finished.
```

## 참고
- 7비트 주소 `0x61`은 8비트 형식으로 `0xC2` (쓰기) 또는 `0xC3` (읽기)에 해당합니다.
- 디지로그나 다른 센서가 연결되어 있다면 추가적인 주소가 출력될 수 있습니다.
