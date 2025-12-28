# 예제 32: 인증 기반 데이터 접근 제어 (Auth-Only)

이 예제는 장치를 잠그지 않은(Unlocked) 상태에서 ATAES132A의 인증 메커니즘을 테스트합니다.

## 주요 목적
- **인증 전용 영역 설정**: Zone 1을 인증 없이는 읽기/쓰기가 불가능하도록 설정합니다.
- **실시간 MAC 계산**: ESP32의 `mbedtls` 라이브러리를 사용하여 칩에서 생성된 Nonce에 대응하는 AES-CCM MAC을 호스트 측에서 계산합니다.
- **인증 통과 확인**: 계산된 MAC으로 `Auth` 명령을 성공시키고, 이후 보호된 영역에 접근 가능한지 검증합니다.

## 실행 순서
1.  **초기 확인**: 설정 전 Zone 1 접근 시도 (칩 상태에 따라 결과가 다를 수 있음).
2.  **보안 설정**:
    - `ZoneConfig[1]`: `AuthRead`, `AuthWrite` 활성화.
    - `KeyConfig[0]`: `InboundAuth`, `RandomNonce` 활성화.
    - `KeyMemory[0]`: 16바이트 테스트 키 주입.
3.  **Nonce 생성**: 칩으로부터 12바이트 랜덤 난수를 가져옵니다.
4.  **MAC 계산 및 인증**: 호스트 측에서 AES-CCM MAC을 계산하여 `Auth` 명령을 실행합니다.
5.  **검증**: 인증 후 Zone 1에 데이터를 쓰고 읽어 정상 동작을 확인합니다.

## 실행 방법
```powershell
.\build.ps1 32 all
```

## 기술 참고
- **KeyID**: 00 (KeyMemory $F200)
- **ZoneID**: 01 (Data $0100)
- **AES-CCM Nonce**: 12 bytes
- **AES-CCM AAD**: Opcode(1) + Mode(1) + Param1(2) + Param2(2) = 6 bytes
