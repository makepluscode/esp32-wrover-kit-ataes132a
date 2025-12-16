# Example 99: ATAES132A 진단 및 설정 도구 (Diagnostic Tool)

이 예제는 ATAES132A 칩의 상태를 진단하고, 개발 및 테스트를 위한 필수 설정을 적용하는 도구입니다.

## 주요 기능 (Features)

1.  **I2C 버스 스캔 (I2C Scan)**
    *   연결된 모든 I2C 장치를 검색합니다.
    *   **결과**: ATAES132A의 기본 주소는 `0x50`이나, 본 하드웨어에서는 `0x61` (`0xC2` 8-bit)로 설정되어 있음을 확인하고 자동으로 감지합니다.

2.  **칩 상태 확인 (Lock Status)**
    *   `LockChipConfig`, `LockConfig`, `LockKey` 레지스터를 읽어 잠금(Lock) 상태를 확인합니다.
    *   현재 상태: **UNLOCKED** (모든 설정 변경 및 키 직접 쓰기 가능).

3.  **칩 구성 설정 (Chip Configuration)**
    *   필수 설정값(ChipConfig)을 확인하고 업데이트합니다.

4.  **키 슬롯 설정 (Key Slot Configuration)**
    *   **변경 사항**: `Example 07`의 루프백(Loopback) 테스트를 지원하기 위해 **Fixed Nonce** 사용이 가능하도록 설정합니다.
    *   **KeyConfig**: `0x09 00 00 00`
        *   `ExternalCrypto` (Bit 0): Enable (외부 암호화 허용)
        *   `InboundAuth` (Bit 1): Disable (인증 필수 아님)
        *   `RandomNonce` (Bit 2): **Disable** (Fixed Nonce 허용 -> 개발용)
        *   `Legacy` (Bit 3): Enable

5.  **키 주입 (Key Provisioning)**
    *   개발용 `Test Key` (00 01 02 ... 0F)를 **Slot 0**과 **Slot 1**에 직접 씁니다.

## 실행 방법

1.  이 프로젝트를 빌드하고 업로드합니다.
    ```powershell
    ./build.ps1 99 all
    ```
2.  시리얼 모니터(`115200 bps`)를 통해 로그를 확인합니다.

## 실행 로그 예시

```text
=== ATAES132A Repair & Init Tool ===
Performing I2C Scan (Pre-Wakeup)...
I2C device found at address 0x61 !

Step 1: Diagnostics
LockConfig (0xF020): 55 55 55 FF
Status: UNLOCKED

Step 3: Key Slot Configuration
Configuring Slot 0...
-> Slot 0 Config Updated (0x09...).

Step 4: Key Loading
Loading Key 0... Success.

Final Verification: KeyConfig Table
Slot | KeyConfig
 0   | 09 00 00 00 
...
Done. Chip is ready for Example 6/7.
```

## 주의 사항

*   이 도구는 **UNLOCKED** 상태의 칩에서만 정상 작동합니다.
*   칩이 이미 잠겨있다면(LOCKED), `KeyConfig`나 `Key`를 변경할 수 없습니다.
*   `Example 07` (AES Decrypt) 예제를 실행하기 전에 반드시 이 도구를 한 번 실행하여 KeyConfig를 `0x09`로 설정해야 합니다.