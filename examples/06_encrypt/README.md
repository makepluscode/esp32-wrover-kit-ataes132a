# 예제 6: 암호화 (Encrypt)

## 목적

AES132 디바이스의 AES-128 암호화 기능을 사용하는 방법을 학습합니다. Encrypt 명령어를 사용하여 평문 데이터를 암호화하고, 암호화의 기본 개념을 이해합니다.
특히 **Unlocked(잠금 해제)** 상태의 칩에서 암호화를 수행하기 위해 필요한 사전 조건(Nonce 생성, 올바른 KeyConfig)을 중점적으로 다룹니다.

## 중요: 실행 전 필수 확인 사항

이 예제는 칩이 올바르게 설정되어 있어야 정상 동작합니다.

1.  **Repair Tool 실행 필수**: `Example 99`를 먼저 실행하여 칩을 설정해야 합니다.
    *   `.\build.ps1 99 all`
    *   이 도구는 **KeyConfig(0x0D)** 설정 및 **키 로드(Direct Write)**를 수행합니다.
2.  **Nonce 생성**: 소스 코드(`main.cpp`)에는 암호화 전 `Nonce`를 생성하는 코드가 포함되어 있습니다. 이는 Unlocked 상태에서 필수적인 절차입니다.

## 설명

Encrypt 명령어는 AES132의 하드웨어 기반 AES-128 암호화 엔진을 사용하여 데이터를 암호화합니다.

### 주요 개념

1.  **AES-128 암호화**
    *   128비트(16바이트) 키 사용
    *   16바이트 블록 단위 암호화

2.  **사전 조건 (Unlocked 상태)**
    *   **Nonce 생성 필수**: 칩이 잠기지 않은 상태(Unlocked)에서는 내부 난수생성기(RNG)가 고정된 값을 내보내므로, 암호화 명령 전 반드시 `Nonce` 명령(Mode 1, Random)을 실행하여 내부 레지스터를 초기화해야 합니다.
    *   **올바른 KeyConfig**: `0x0D` (External Crypto Allowed, Inbound Auth Disabled) 설정이 권장됩니다.

## 사용 방법

```bash
# 1. 칩 설정 (최초 1회 필수)
.\build.ps1 99 all

# 2. 암호화 예제 실행
.\build.ps1 6 all
```

## 실제 출력 결과

다음은 설정이 완료된 칩에서 실행한 성공 로그입니다.

```
========================================
ESP32 AES132 CryptoAuth Example
Example 06: Encryption (AES-128)
========================================

AES132 initialized successfully

DEBUG: ChipConfig [0xF040-0xF043]: C3 C3 FF FF
  -> EncDecrE: Enabled
DEBUG: Dumping Information...
  -> LegacyE:  Enabled
Note: This example uses Key Slot 0 (as configured by Repair Tool 99).
      Make sure you ran Example 99 first (or configured Slot 0 manually).

Generating Nonce (Required for Encrypt)...
-> Nonce Generated Successfully.

=== Example 1: Encrypt Simple Data ===
Plaintext (16 bytes):
  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

Encrypting with Key Slot 0...
[Encrypt] SUCCESS
Ciphertext (16 bytes):
  A9 7F 26 5F D7 30 90 37 FA EA 7E 5B 95 42 2B 51


=== Example 2: Encrypt Text Data ===
Plaintext: "Hello AES132!"
Plaintext (hex): 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 00 00

Encrypting with Key Slot 0...
[Encrypt] SUCCESS
Ciphertext (16 bytes):
  0C 07 A8 6B 8B 80 F3 22 75 B6 D9 43 4C 34 AC 58


=== Example 3: Encrypt Multiple Blocks ===
Note: AES-128 encrypts one 16-byte block at a time.
      For multiple blocks, call Encrypt multiple times.

Block 1:
  Plaintext:  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
  Ciphertext: 14 6A 2F 16 36 2F 78 81 01 24 C5 F9 2A C2 B7 E4

Block 2:
  Plaintext:  FF EE DD CC BB AA 99 88 77 66 55 44 33 22 11 00
  Ciphertext: 92 EF 3C 5D 67 FA 5B 31 78 1F E3 BB F2 8E 1E 0D

=== Example 4: Encryption Analysis ===
Characteristics of AES encryption:
1. Same plaintext always produces same ciphertext (with same key)
2. Small change in plaintext causes large change in ciphertext
3. Ciphertext appears random and unpredictable
4. Ciphertext length equals plaintext length (16 bytes)


=== Encryption Examples Complete ===

Note: To decrypt this data, use Decrypt command (Example 7)
      with the same key slot.
```

## 문제 해결 (Troubleshooting)

### 오류 코드별 원인 및 해결

1.  **`0x50` (Parse Error)**
    *   **원인 1**: `Encrypt` 명령어의 `Param2`가 0으로 설정됨 (데이터 길이인 16이어야 함).
    *   **원인 2**: `Nonce` 명령이 실행되지 않음 (내부 TempKey/Nonce 레지스터가 초기화되지 않음).
    *   **해결**: `aes132m_execute` 호출 시 `Param2`를 16으로 설정하고, 암호화 전 `Nonce` 생성 코드를 추가하십시오.

2.  **`0x80` (Key Error)**
    *   **원인**: `KeyConfig` 설정이 암호화를 허용하지 않음 (예: `0x08`, `0x1C` 등 잘못된 설정).
    *   **해결**: `KeyConfig`를 `0x0D`로 변경하십시오 (ExtCrypto=1, InboundAuth=0).

3.  **암호화 값 불일치**
    *   **원인**: `Mode 0` (ECB 유사) 특성상 키가 같으면 항상 같은 결과가 나옵니다.
    *   **해결**: 이는 정상적인 동작입니다.

## 관련 문서

*   **ATAES132A 데이터시트**: Section 7.7 - Encrypt Command
*   [예제 99: 진단 도구](../99_diagnostic/README.md)
*   [예제 7: 복호화](../07_decrypt/README.md)
