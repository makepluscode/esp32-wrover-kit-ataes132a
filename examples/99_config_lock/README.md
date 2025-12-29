# 예제 99: 보안 설정 및 영구 잠금 (ZZ_config_lock)

이 예제는 UNLOCKED 상태인 ATAES132A 칩을 보안 모듈로 설정하고 최종 Lock을 수행합니다.

> [!CAUTION]
> **잠금(Lock)**은 영구적이며 되돌릴 수 없습니다. 한 번 잠긴 칩은 설정을 변경하거나 키를 다시 로드할 수 없습니다.

## 주요 단계
1. **[Phase 0] 기기 진단**: `aes132c_wakeup()` 호출, `BlockRead` 명령(32바이트 분할)으로 상태 확인 및 `ZoneConfig` 덤프.
2. **[Phase 1] 초기 보안 설정**: Zone 0을 쓰기 가능 상태(`0x00000000`)로 임시 설정.
3. **[Phase 2] 유저 데이터 기입**: User Zone 0에 Magic Number(`0xC0DE`) 128바이트 기록.
4. **[Phase 3] 최종 보안 정책**: Zone 0을 영구 읽기 전용(`0x10000000`)으로 최종 설정.
5. **[Phase 4] 키 로딩**: Key 0(마스터), Key 1(사용자) 등 비밀 키 주입.
6. **[Phase 5] 물리적 잠금**: `LOCK` 명령으로 Config(Mode 0x02) 및 Key(Mode 0x01) 메모리 잠금.

## 🔑 비밀 키 정보 (Secret Keys)
초기화 시 주입되는 키 리스트입니다.

| 슬롯 | 키 유형 | 값 (Hex) | 설명 |
|:---:|:---:|:---|:---|
| **Key 0** | **마스터 키** | `A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF` | Zone 1, 2 쓰기 인증 및 관리용 |
| **Key 1** | **사용자 키** | `F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF` | 일반 암복호화 테스트용 |

## 🛡️ 보안 정책 (Security Policy)
잠금 이후 적용되는 메모리 보호 설정입니다.

*   **Zone 0**: **영구 읽기 전용 (Permanently Read-only)**. 수정 불가.
*   **Zone 1, 2**: **인증 후 쓰기 (Auth-Write)**. Key 0으로 `Auth` 명령 성공 시에만 쓰기 가능.
*   **Zone 3-15**: **제한 없음 (Open)**. 자유로운 R/W 가능.

## 실행 방법
```powershell
.\build.ps1 99 all
```

## 검증
- Zone 0이 읽기 전용으로 동작하는지 확인합니다.
- Zone 1이 인증 없이 쓰기가 거부되는지 확인합니다.
- Zone 3이 자유롭게 읽기/쓰기가 되는지 확인합니다.
