# 예제 9: 인증 (Authentication)

## 목적

AES132 디바이스와 호스트(ESP32) 간의 **인증(Authentication)** 절차를 학습합니다. `Nonce` 명령어를 통해 난수를 교환하고, `Auth` 명령어를 통해 상호 신뢰를 검증하는 과정을 이해합니다.

## 설명

인증은 보안 시스템에서 가장 중요한 요소 중 하나입니다. ATAES132A는 **Challenge-Response** 방식을 사용하여 인증을 수행합니다.

### 인증 절차 (Inbound Authentication)

1.  **Challenge 생성 (Nonce)**:
    - 호스트가 디바이스에게 난수(`Challenge`) 생성을 요청합니다 (`Nonce` 명령어).
    - 디바이스는 내부 RNG를 사용하여 난수를 생성하고, 이를 내부 `TempKey` 레지스터에 저장합니다.

2.  **MAC 계산**:
    - 호스트는 공유된 비밀 키(Key Slot 2)와 `Challenge`를 사용하여 **MAC (Message Authentication Code)**을 계산합니다.
    - 참고: ATAES132A는 AES-CCM 모드를 유사하게 사용합니다.

3.  **검증 (Auth)**:
    - 호스트가 계산한 MAC을 디바이스로 전송합니다 (`Auth` 명령어).
    - 디바이스는 내부적으로 동일한 MAC을 계산하여 비교합니다.
    - MAC이 일치하면 인증이 성공하고, 해당 키 슬롯에 대한 권한(Authenticated)이 부여됩니다.

## 동작 방식

이 예제는 인증의 **프로토콜 흐름**을 보여주는 데 중점을 둡니다.

1.  `AES132_NONCE` (0x01): 디바이스로부터 12바이트 Random Nonce를 받아옵니다.
2.  `AES132_AUTH` (0x03): 호스트가 계산한 MAC을 보냅니다. (본 예제에서는 데모용 Dummy MAC을 전송)

> **주의**: 실제 인증을 성공시키려면 호스트(ESP32) 측에서 AES-128 엔진을 사용하여 정확한 MAC을 계산해야 합니다. 이 예제에서는 단순화를 위해 Dummy MAC을 사용하므로 `0x40` (MacError)가 발생하는 것이 정상입니다.

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 9

# 빌드 및 업로드
pio run -e esp-wrover-kit -t upload

# 모니터링
pio device monitor
```

## 예상 출력

```
========================================
ESP32 AES132 CryptoAuth Example
Example 09: Authentication (Fixed)
========================================

AES132 initialized successfully

=== Authenticating Key Slot 2 ===
Performing Inbound Authentication (Placeholder)...
DEBUG RX: 4 50 99 E3 0 0 0 0 
Device Nonce: 00 00 00 CE ... (12 bytes)

FAILED: Authentication Failed. RetCode: 0x20
--------------------------------------------------
[OK] Expected Result: 0x20 (MacError/CheckMac Failure)
The Nonce command succeeded, but Auth failed
because we sent a dummy MAC. This confirms communication works.
--------------------------------------------------
```

## 결과 분석

- **Device Nonce 출력**: `Nonce` 명령어가 성공적으로 수행되어 디바이스로부터 12바이트 난수를 받았음을 의미합니다. (0xFF 오류 해결)
- **Error 0x20 (Auth Check Failed)**: 호스트가 보낸 Dummy MAC(0xAA...)이 디바이스가 계산한 값과 다르기 때문에 발생합니다. 
    - 이는 **정상적인 보안 동작**입니다. 디바이스가 비정상적인 인증 시도를 정확히 거부했음을 보여줍니다.
    - 만약 이 단계에서 `0x00` (Success)가 나왔다면 오히려 심각한 보안 결함이 있음을 의미합니다.

## 코드 설명

-   **`generateNonce()`**: `Nonce` 명령어를 실행하여 디바이스의 난수를 받아오고 `TempKey`를 업데이트합니다. **Mode 0x00**과 **20바이트 InSeed**를 사용하여 구성되었습니다.
-   **`performInboundAuth()`**: `Auth` 명령어를 실행합니다. 실제 AES 구현이 없으므로 Placeholder MAC을 전송합니다.

## 다음 단계

-   **예제 10: 키 생성 (KeyCreate)**: 새로운 키를 생성하는 방법을 학습합니다.
