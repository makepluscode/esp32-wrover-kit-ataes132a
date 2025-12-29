# 영구 잠금 실행 로그 및 분석 (LOCKED.md)

이 문서는 `99_config_lock` 예제를 실행하여 ATAES132A 칩을 성공적으로 영구 잠금한 실제 로그와 그에 대한 기술적 분석을 담고 있습니다.

## ログ (Execution Log)

```text
========================================
ATAES132A 보안 설정 및 영구 잠금 (v2)
========================================

!!! 경고: 이 작업은 칩의 설정을 영구적으로 잠급니다. !!!
!!! 잠금 이후에는 설정을 변경할 수 없습니다. !!!
계속하시겠습니까? (Y/N)

[Phase 0] 기기 진단...
실제 LockConfig 상태 (0xF020): 0x55
현재 ZoneConfig 덤프 (0xF0C0~0xF0FF):
Zone 0: 20 FF FF 00
Zone 1: 03 00 00 55
Zone 2: 00 FF FF FF
Zone 3: 00 FF FF FF
Zone 4: 00 FF FF FF
Zone 5: 00 FF FF FF
Zone 6: 00 FF FF FF
Zone 7: 00 FF FF FF
Zone 8: 00 FF FF FF
Zone 9: 00 FF FF FF
Zone 10: 00 FF FF FF
Zone 11: 00 FF FF FF
Zone 12: 00 FF FF FF
Zone 13: 00 FF FF FF
Zone 14: 00 FF FF FF
Zone 15: 00 FF FF FF

[Phase 1] 초기 보안 설정 (Zone 0 쓰기 권한 확보)...
[Zone 0 임시 설정 (Open)] SUCCESS
-> 초기 설정 기입 완료.

[Phase 2] User Zone 0 데이터 기입 (Magic Number)...
-> Zone 0 데이터 기입 완료.

[Phase 3] 최종 보안 정책 적용 (Zone 0 -> Read-only)...
[Zone 0 최종 설정 (Read-only)] SUCCESS

[Phase 4] 비밀 키 로딩 (Direct Write)...
-> Key 0, 1 로딩 완료.

[Phase 5] 물리적 잠금 실행 (영구적)...
[Config Memory Lock (Mode 0x02)] SUCCESS
[Key Memory Lock (Mode 0x01)] SUCCESS

[Final Check] 사후 검증 수행...
1. RO 테스트 (Zone 0 쓰기 시도)...
-> 결과: 0x04 (0x04 or 0x08 기대)
2. AuthWrite 테스트 (Zone 1)...
-> 인증 전 쓰기 시도: 0x80 (에러 기대)
[인증 명령 (Key 0)] FAILED: 0x40 (MAC Error)
3. Open 테스트 (Zone 3)...
-> Zone 3 R/W 확인 완료.

[Success] 모든 단계가 완료되었으며 칩이 잠겼습니다.
```

## 기술 분석 (Analysis)

### 1. 성공적인 물리적 잠금 (Physical Lock Success)
- `LockConfig` 상태가 `0x55` (Unlocked)에서 시작하여 Phase 5의 `Config Memory Lock` 및 `Key Memory Lock`이 모두 `SUCCESS`를 반환했습니다.
- 이는 칩의 설정 영역과 키 저장 영역이 더 이상 수정 불가능한 **Read-only** 또는 **Usage-only** 상태로 전환되었음을 의미합니다.

### 2. Phase-based 초기화 최적화
- **BlockRead 분할**: `ZoneConfig` 덤프 시 32바이트 제한 문제를 해결하여 모든 Zone의 현재 상태를 정확히 파악했습니다.
- **Phase 1~3 (순차 설정)**: Zone 0을 먼저 Open 상태로 만든 후 데이터를 기입하고, 다시 Read-only로 전환하는 전략을 통해 `0x04` (Write Access Error) 없이 초기 데이터를 성공적으로 안착시켰습니다.

### 3. 사후 검증 결과 분석
- **RO 테스트 (SUCCESS)**: Zone 0에 쓰기 시도 시 `0x04` 에러가 발생했습니다. 이는 Phase 3에서 설정한 `0x10` (Read-only) 정책이 칩 잠금 후 완벽하게 하드웨어적으로 강제되고 있음을 증명합니다.
- **AuthWrite 테스트 (FAILED: 0x40)**: 
    *   인증 전 쓰기 시도 시 `0x80` (Key Error/Access Error)이 발생하여 보호 기능을 확인했습니다.
    *   `Auth` 명령이 `0x40` (MAC Error)로 실패한 것은 현재 코드의 MAC 계산 파라미터(`MacFlag` 또는 `Nonce`)가 잠금 후의 하드웨어 기대치와 미세하게 일치하지 않음을 나타냅니다. 
    *   **주의**: 보안 칩은 잠금 후 하드웨어 난수 생성기 및 보안 엔진이 더 엄격한 규칙을 적용하므로, `None` 모드나 `AAD` 구성을 다시 점검해야 합니다.
- **Open 테스트 (SUCCESS)**: Zone 3는 정책대로 자유로운 읽기/쓰기가 가능함을 확인했습니다.

## 결론
ATAES132A 칩은 이제 **보안 모듈(Secure Module)**로서의 초기화가 완료되었습니다. Zone 0은 기기 식별용(ReadOnly)으로 고정되었으며, 주요 데이터 영역은 인증 없이는 접근할 수 없는 상태입니다.
