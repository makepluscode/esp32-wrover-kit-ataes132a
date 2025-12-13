# 예제 8: 키 로드 (Key Load)

## 목적

AES132 디바이스의 키 메모리(Key Memory)에 암호화 키를 안전하게 저장하는 방법을 학습합니다. 특히 칩의 잠금 상태(Locked/Unlocked)에 따라 달라지는 키 저장 방식을 이해하고, `KeyConfig` 설정을 통해 키의 사용 권한을 제어하는 방법을 다룹니다.

## 중요: 실행 전 확인 사항

이 예제는 칩의 상태(Locked vs Unlocked)에 따라 동작이 다릅니다. 현재 코드는 **Unlocked(잠금 해제)** 상태의 칩에 최적화되어 있습니다.

*   **Unlocked 상태**: `KeyLoad` 명령어 대신 `BlockWrite`를 사용하여 키 메모리 주소(0xF200~)에 직접 키를 씁니다.
*   **Locked 상태**: `KeyLoad` 명령어를 사용하여 키를 저장해야 하며, `KeyConfig`에 따라 저장 가능 여부가 결정됩니다.

## 설명

### 1. 키 저장 방식 (Direct Write vs KeyLoad)

*   **Direct Write (Unlocked Only)**: 칩이 잠기지 않은 개발 단계에서는 보안 제약 없이 키 메모리 주소에 직접 접근하여 키를 쓸 수 있습니다. 이는 초기 설정 및 디버깅에 매우 유용합니다.
*   **KeyLoad (Locked Only)**: 칩이 잠긴 후에는 보안을 위해 `KeyLoad` 명령어만을 통해 키를 갱신할 수 있으며, 이는 `KeyConfig`의 `ChangeKeys` 설정에 따라 제한됩니다.

### 2. KeyConfig 분석 및 권장 설정

키를 저장할 때, 해당 키를 어떻게 사용할지 정의하는 `KeyConfig` 설정이 필수적입니다. 잘못된 설정은 키 사용을 불가능하게 만들 수 있습니다.

#### 분석 예시: `0x1C 00 00 00` (주의 필요)
사용자가 겪을 수 있는 설정 오류 예시입니다.

*   **0x1C (Byte 0) 분석**:
    *   `Bit 4 (AuthKey) = 1`: **인증 필수**. 이 키를 사용하려면 먼저 다른 키(LinkPointer가 가리키는 키)로 인증해야 합니다.
    *   `Bit 0 (ExternalCrypto) = 0`: **암호화 금지**. `Encrypt`/`Decrypt` 명령어를 사용할 수 **없습니다**.
*   **결과**: 이 설정으로는 암호화 예제(Ex 06)를 실행할 수 없으며, 복잡한 인증 절차를 거쳐야 합니다.

#### 권장 설정: `0x0D` (테스트용)
암호화 테스트를 위해 인증 없이 바로 사용할 수 있는 설정입니다.

*   **설정 값**: `0x0D 00 00 00`
*   **0x0D (Byte 0) 분석**:
    *   `Bit 4 (AuthKey) = 0`: **인증 불필요**. 누구나 바로 키 사용 가능.
    *   `Bit 0 (ExternalCrypto) = 1`: **암호화 허용**. `Encrypt`/`Decrypt` 명령어 사용 가능.

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 8

# 빌드 및 업로드
.\build.ps1 8 upload

# 실행 결과 모니터링
pio device monitor
```

## 실행 결과 로그 (성공 예시)

```
========================================
ESP32 AES132 CryptoAuth Example
Example 08: Key Load
========================================

AES132 initialized successfully

=== DEBUG: Dumping KeyConfig Table (0-15) ===
Slot | KeyConfig Bytes (4)
-----|---------------------
  2  | 0D 00 00 00  <-- 권장 설정 (0x0D: Encrypt 허용, Auth 불필요)
---------------------------

=== Step 1: Configure Key Slot 2 ===
Setting KeyConfig to allow Encryption/Decryption...
[Configure Slot] SUCCESS

=== Step 2: Load Key into Slot 2 ===
Key to Load: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 
[Key Direct Write] SUCCESS

SUCCESS: Key written successfully (Direct Write for Unlocked Chip)!
IMPORTANT: You must update Example 6 to use Slot 2.
```

## 문제 해결 (Troubleshooting)

### 1. KeyConfig 설정 오류 (`0x1C` vs `0x0D`)
*   **증상**: 키 로드는 성공했으나 `Example 06 (Encrypt)` 실행 시 `0x80 (KeyError)` 발생.
*   **원인**: `KeyConfig`가 `0x1C`로 설정되어 `ExternalCrypto` 권한이 없거나 `AuthKey` 인증을 요구함.
*   **해결**: `main.cpp`의 `configure_key_slot` 함수에서 설정값을 `0x0D`로 변경하여 다시 실행하십시오.

### 2. Write 실패
*   **증상**: `[Key Direct Write] FAILED` 메시지 출력.
*   **원인**: I2C 통신 오류 또는 주소 계산 오류.
*   **해결**: 배선 확인 및 `Key Memory` 주소(`0xF200` + ID*16)가 코드상 정확한지 확인하십시오.

## 관련 문서

*   **ATAES132A 데이터시트**: Section 4.3 - Key Configuration
*   [예제 6: 암호화](../06_encrypt/README.md)
*   [예제 99: 진단 도구](../99_diagnostic/README.md)
