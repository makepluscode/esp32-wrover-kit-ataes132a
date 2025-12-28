# 예제 31: ReadOnly 권한 제어 및 검증 테스트

이 예제는 ATAES132A의 **Configuration Memory**를 수정하여 특정 User Zone의 쓰기 권한을 제어하고, 하드웨어 수준에서 쓰기 차단이 정상적으로 동작하는지 확인합니다.

## 주요 기능
- **ZoneConfig 설정**: `0xF0C0` 주소의 구성을 수정하여 `WriteMode` 및 `ReadOnly` 기능을 활성화합니다.
- **권한 전환**: `ReadOnly` 바이트를 `0x55`(R/W)에서 `0x00`(Read-Only)으로 변경합니다.
- **보안 검증**: 읽기 전용 모드에서 쓰기 시도를 할 때 하드웨어가 이를 차단하고 STATUS 레지스터의 `EERR` 비트를 세팅하는지 확인합니다.

## 실행 순서
1. **Step 1**: 현재 Zone 0의 설정 상태를 읽어서 출력합니다.
2. **Step 2**: 쓰기 가능 모드(`WriteMode=10b`, `ReadOnly=0x55`)로 설정합니다.
3. **Step 3**: Zone 0에 테스트 데이터를 쓰고 다시 읽어 정상적으로 기록되었는지 확인합니다.
4. **Step 4**: 읽기 전용 모드(`ReadOnly=0x00`)로 전환합니다.
5. **Step 5**: 다시 쓰기를 시도하여 하드웨어가 이를 거부하는지(리턴 코드 및 EERR 비트) 확인합니다.

## 실행 방법
```powershell
.\build.ps1 31 all
```

## 주의사항
> [!IMPORTANT]
> 이 예제는 칩의 **Configuration Memory**(EEPROM)를 직접 수정합니다. 테스트가 끝난 후 다른 예제에서 쓰기가 안 된다면, 다시 이 예제를 통해 `ReadOnly`를 `0x55`로 복구해야 합니다.

## 기술 참조
- **ZoneConfig[0] 주소**: `0xF0C0`
- **WriteMode Bits**: Byte 0의 Bit 5:4 (`10b` = `0x20`)
- **ReadOnly Byte**: Byte 3 (`0x55` = RW, 그 외 = RO)
- **Status EERR**: STATUS 레지스터의 Bit 5 (0x20)가 에러 발생 시 세팅됨 (일부 쓰기 금지 상황에서는 비트 대신 데이터 변경 여부로 확인 가능)

## 실행 결과 로그
```text
========================================
ESP32 AES132 CryptoAuth Example
Example 31: ReadOnly Test (Zone 0)
========================================

Step 1: 초기 확인 - Zone 0 설정 읽기
Current ZoneConfig[0]: 20 FF FF 00

Step 2: RW 활성화 - WriteMode=10b(0x20), ReadOnly=0x55 설정
Writing to ZoneConfig[0]: 20 FF FF 55
[Set RW Mode] SUCCESS
Verified ZoneConfig[0]: 20 FF FF 55

Step 3: 데이터 쓰기 테스트 - Zone 0 (0x0000)에 데이터 쓰기
Data to write: AA BB CC DD 11 22 33 44 55 66 77 88 99 00 AA FF
[Write Data] SUCCESS
Data read back: AA BB CC DD 11 22 33 44 55 66 77 88 99 00 AA FF
Data Verification: SUCCESS (Match)

Step 4: ReadOnly 변경 - ReadOnly=0x00 설정
Setting ReadOnly byte to 0x00...
[Set ReadOnly Mode] SUCCESS
Verified ZoneConfig[0] (ReadOnly): 20 FF FF 00

Step 5: 차단 검증 - 쓰기 금지 동작 확인
Attempting to write 0xEE to Zone 0 (Read-Only state)...
Sent Data (Write Attempt): EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE EE
Return Code (I2C): 0x0 (SUCCESS - I2C ACK)
Device Status Register: 0x00 (EERR Bit NOT SET)
Memory content after write attempt: AA BB CC DD 11 22 33 44 55 66 77 88 99 00 AA FF
Verification Result: SUCCESS (Write was Blocked by Hardware)
-> Data remains unchanged as expected.

=== ReadOnly Test Complete ===
```

## 결과 분석
1. **하드웨어 보호 확인**: Step 5에서 I2C 통신 자체는 `0x0` (ACK)을 반환했지만, 실제 메모리 내용은 Step 3에서 쓴 데이터(`AA BB...`)가 그대로 유지되었습니다. 이는 ATAES132A 내부 로직이 쓰기 요청을 수신했음에도 불구하고 `ZoneConfig` 설정에 따라 실제 EEPROM 기록을 차단했음을 의미합니다.
2. **ReadOnly 바이트 작동 조건**: `ReadOnly` 바이트(Byte 3)가 작동하려면 반드시 `WriteMode`(Byte 0의 Bit 5:4)가 `10b` 또는 `11b`로 설정되어 있어야 합니다. 테스트 결과 `0x20` (10b) 설정 환경에서 정상 작동함이 확인되었습니다.
3. **보안성**: 일단 `ReadOnly`가 활성화되면 해당 영역은 물리적인 쓰기 시도로부터 보호됩니다. 다시 쓰기 권한을 얻으려면 `Configuration Memory`의 해당 바이트를 다시 `0x55`로 수정해야 합니다.
