# 예제 10: 키 생성 (Key Create)

## 목적
AES132 디바이스의 내부 난수 생성기(True Random Number Generator)를 사용하여 새로운 암호화 키를 생성하고, 이를 지정된 Key 슬롯에 안전하게 저장하는 방법을 학습합니다.

이 기능은 외부(호스트)에서 키를 생성하여 주입하는 `KeyLoad`와 달리, 키가 외부 버스를 통해 노출되지 않고 디바이스 내부에서 생성 및 저장되므로 보안성이 매우 뛰어납니다.

## 사용 명령어
- **OpCode**: `0x08` (AES132_KEY_CREATE)
- **설명**: 내부 RNG를 이용해 키 생성 후 지정 슬롯 덮어쓰기

## 사전 준비 사항
- **KeyConfig 설정**: 대상 키 슬롯(예: Slot 4)의 `KeyConfig` 레지스터에서 `Create` 권한이 허용되어 있어야 합니다. (기본값이 잠겨있을 수 있습니다)

## 동작 과정
1. **초기화**: I2C 통신 및 AES132 디바이스 초기화
2. **KeyCreate 실행**: Slot 4를 대상으로 키 생성 명령 전송 (Mode 0: Random)
3. **결과 확인**: 
   - 성공 시: `0x00` 반환. 해당 슬롯에 새로운 128비트 랜덤 키가 저장됨.
   - 실패 시: `0x80` (KeyError) 등 반환. (주로 권한 문제)

## 예상 출력 (성공 시)

```
========================================
ESP32 AES132 CryptoAuth Example
Example 10: Key Create
========================================

AES132 initialized successfully

=== Step: Create New Key ===
Target Key Slot: 4
SUCCESS: Key Created Successfully!
```

## 예상 출력 (실패 시 - 권한 없음)

```
FAILED: Key Create Error. RetCode: 0x80
Hint: 0x80 (KeyError) - The slot might be locked or KeyConfig.Create is disabled.
```

## 코드 설명
- `createNewKey(key_id)`: `aes132m_execute`를 사용하여 `KEY_CREATE` 명령을 전송합니다.
- `Mode 0x00`: 호스트가 제공하는 데이터 없이, 칩 내부의 RNG만을 사용하여 키를 만듭니다.

## 다음 단계
이것으로 기본~고급 예제 시리즈를 모두 마쳤습니다!
이제 이 라이브러리와 예제들을 바탕으로 자신만의 보안 애플리케이션(예: 보안 부팅, 데이터 암호화 전송 등)을 구축할 수 있습니다.
