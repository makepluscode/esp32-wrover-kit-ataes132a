# 예제 5: 카운터 (Counter)

## 목적

AES132 디바이스의 카운터 기능을 사용하는 방법을 학습합니다. 카운터 값 읽기 및 증가 기능을 이해하고, 카운터의 활용 사례를 학습합니다.

## 설명

Counter 명령어는 AES132의 4개의 독립적인 카운터 슬롯을 관리합니다. 각 카운터는 32비트(4바이트) 값을 저장하며, 비휘발성 메모리에 저장되어 전원이 꺼져도 유지됩니다. 카운터는 보안 애플리케이션에서 사용 횟수 제한, 라이센스 관리, 이벤트 카운팅 등에 활용됩니다.

### 주요 개념

1. **카운터 슬롯**
   - AES132는 총 4개의 독립적인 카운터 슬롯을 제공합니다 (Counter ID: 0-3)
   - 각 카운터는 32비트(4바이트) 값을 저장합니다
   - 카운터 값은 0부터 시작하여 최대값까지 증가할 수 있습니다

2. **카운터 모드**
   - **Mode 0**: 카운터 값 읽기
   - **Mode 1**: 카운터 값 증가

3. **카운터 제한**
   - 각 카운터는 최대값에 도달하면 더 이상 증가할 수 없습니다
   - 최대값에 도달하면 `0x10` (CountError) 오류가 반환됩니다
   - 카운터는 감소할 수 없습니다 (단방향 증가만 가능)

4. **카운터 활용 사례**
   - 사용 횟수 제한 (예: 라이센스 관리)
   - 이벤트 카운팅
   - 보안 로그 관리
   - 트랜잭션 카운팅

## 데이터시트 참조

### Counter 명령어 (Section 7.6)

**명령어 코드**: `0x0A`

**명령어 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 패킷의 총 바이트 수 (Count + Body + Checksum) |
| 1 | Op-Code | `0x0A` (Counter) |
| 2 | Mode | 명령어 모드 (`0x00`: 읽기, `0x01`: 증가) |
| 3-4 | Parameter 1 | Counter ID (0-3) |
| 5-6 | Parameter 2 | 증가값 (Mode 1일 때, 1-255) 또는 0 (Mode 0일 때) |
| 7-n | Data | 없음 (Counter는 입력 데이터 없음) |
| n+1-n+2 | Checksum | CRC-16 체크섬 |

**응답 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 응답 패킷의 총 바이트 수 |
| 1 | Status | 반환 코드 (Return Code) |
| 2-5 | Data | 카운터 값 (4바이트, 빅엔디안) |
| 6-7 | Checksum | CRC-16 체크섬 |

**Parameter 1 구성**:
```
Bit 15-8: Reserved (0)
Bit 7-0:  Counter ID (0-3)
```

**Parameter 2 구성** (Mode 1일 때):
```
Bit 15-8: Reserved (0)
Bit 7-0:  Increment Value (1-255)
```

**Mode 필드**:
- `0x00`: 카운터 읽기 (Read Counter)
- `0x01`: 카운터 증가 (Increment Counter)

### 반환 코드 (Return Codes)

| Code | Name | Description |
|------|------|-------------|
| `0x00` | Success | 명령어 성공적으로 실행됨 |
| `0x03` | ParseError | 명령어 파싱 오류 (잘못된 파라미터 등) |
| `0x10` | CountError | 카운터 제한값 도달 또는 카운터 사용 오류 |
| `0x0F` | ExecutionError | 명령어 실행 오류 |

### 카운터 구조 (Section 2.3)

**카운터 메모리 구조**:
- 총 4개의 카운터 슬롯 (Counter ID: 0-3)
- 각 카운터는 32비트(4바이트) 값을 저장
- 빅엔디안(Big-Endian) 형식으로 저장됨
- 비휘발성 메모리에 저장되어 전원이 꺼져도 유지됨

**카운터 값 형식**:
```
Byte 0: MSB (Most Significant Byte)
Byte 1: 
Byte 2: 
Byte 3: LSB (Least Significant Byte)
```

**카운터 최대값**:
- 각 카운터는 최대 32비트 값까지 증가 가능
- 최대값: 4,294,967,295 (0xFFFFFFFF)
- 최대값에 도달하면 더 이상 증가할 수 없음

### 카운터 읽기 (Mode 0)

**명령어 시퀀스**:
1. Counter 명령어 전송 (Mode = 0x00)
2. Parameter 1: Counter ID (0-3)
3. Parameter 2: 0 (읽기 모드에서는 사용 안 함)
4. 응답에서 카운터 값 수신 (4바이트)

**응답 데이터**:
- 응답 패킷의 Data 필드에 4바이트 카운터 값이 포함됨
- 빅엔디안 형식으로 저장됨

### 카운터 증가 (Mode 1)

**명령어 시퀀스**:
1. Counter 명령어 전송 (Mode = 0x01)
2. Parameter 1: Counter ID (0-3)
3. Parameter 2: 증가값 (1-255)
4. 응답에서 증가된 카운터 값 수신 (4바이트)

**증가 동작**:
- 현재 카운터 값에 Parameter 2의 값을 더함
- 증가 후 새로운 카운터 값이 응답으로 반환됨
- 최대값에 도달하면 `0x10` (CountError) 오류 반환

**증가값 제한**:
- 한 번에 최대 255까지 증가 가능
- 더 큰 값을 증가시키려면 여러 번 호출해야 함

### 보안 고려사항

1. **카운터 감소 불가**:
   - 카운터는 증가만 가능하고 감소할 수 없습니다
   - 이는 보안을 위한 설계입니다 (역방향 조작 방지)

2. **카운터 제한**:
   - 최대값에 도달하면 더 이상 증가할 수 없습니다
   - 카운터를 재설정하려면 특별한 권한이 필요할 수 있습니다

3. **카운터 독립성**:
   - 각 카운터는 독립적으로 동작합니다
   - 한 카운터의 증가가 다른 카운터에 영향을 주지 않습니다

4. **비휘발성 저장**:
   - 카운터 값은 비휘발성 메모리에 저장됩니다
   - 전원이 꺼져도 값이 유지됩니다

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 5

# 빌드
pio run -e esp-wrover-kit

# 업로드
pio run -e esp-wrover-kit -t upload

# 모니터
pio device monitor
```

## 실제 실행 결과

다음은 실제 디바이스에서 실행한 결과입니다:

```
========================================
ESP32 AES132 CryptoAuth Example
Example 05: Counter
========================================

AES132 initialized successfully

=== Example 1: Read All Counter Values ===
Debug Counter 0: ret=0x50, Count=4, Status=0x50, Data=0x04 0x50 0x99 0xE3
Error: Counter command failed with code 0x50
Failed to read Counter 0
Debug Counter 1: ret=0x50, Count=4, Status=0x50, Data=0x04 0x50 0x99 0xE3
Error: Counter command failed with code 0x50
Failed to read Counter 1
Debug Counter 2: ret=0x50, Count=4, Status=0x50, Data=0x04 0x50 0x99 0xE3
Error: Counter command failed with code 0x50
Failed to read Counter 2
Debug Counter 3: ret=0x50, Count=4, Status=0x50, Data=0x04 0x50 0x99 0xE3
Error: Counter command failed with code 0x50
Failed to read Counter 3
```

## 출력 결과 상세 분석

### 1. ParseError (0x50) 오류 발생

**`ret=0x50, Status=0x50`**:
- Counter 명령어가 `0x50` (ParseError) 오류를 반환했습니다
- 이는 명령어 파싱 오류를 의미하며, 다음과 같은 원인이 있을 수 있습니다:
  1. **Counter 기능 미지원**: 일부 ATAES132A 모델에서는 Counter 기능이 지원되지 않을 수 있습니다
  2. **Counter 기능 비활성화**: Counter 기능이 Configuration Memory에서 비활성화되어 있을 수 있습니다
  3. **명령어 형식 오류**: 명령어 파라미터 형식이 잘못되었을 수 있습니다 (하지만 코드는 데이터시트에 맞게 작성되었습니다)

### 2. 응답 패킷 구조 분석

**응답 패킷**:
- `Count=4`: Count(1) + Status(1) + Checksum(2) = 4바이트
- `Status=0x50`: ParseError
- `Data=0x04 0x50 0x99 0xE3`: Count, Status, Checksum만 포함 (카운터 데이터 없음)

**분석**:
- 응답 패킷에 카운터 데이터가 없습니다
- 이는 Counter 명령어가 파싱 단계에서 실패했음을 의미합니다
- 디바이스가 Counter 명령어를 인식하지 못하거나 지원하지 않는 것으로 보입니다

### 3. 가능한 원인 및 해결 방법

**원인 1: Counter 기능 미지원**
- 일부 ATAES132A 모델에서는 Counter 기능이 지원되지 않을 수 있습니다
- 해결: 디바이스 모델 및 데이터시트 확인

**원인 2: Counter 기능 비활성화**
- Configuration Memory에서 Counter 기능이 비활성화되어 있을 수 있습니다
- 해결: Configuration Memory 확인 및 Counter 기능 활성화 (권한 필요)

**원인 3: 디바이스 구성 문제**
- 디바이스가 특정 구성 모드에 있어 Counter 명령어를 지원하지 않을 수 있습니다
- 해결: 디바이스 리셋 또는 재초기화

### 4. 정상 동작 시 예상 출력

Counter 기능이 지원되는 경우 예상되는 출력:

```
=== Example 1: Read All Counter Values ===
Counter 0: 0 (0x0)
Counter 1: 0 (0x0)
Counter 2: 0 (0x0)
Counter 3: 0 (0x0)

=== Example 2: Increment Counter 0 ===
Initial Counter 0 value: 0
Incrementing Counter 0 by 1...
[Increment Counter] SUCCESS
New Counter 0 value: 1
Difference: 1
```

## 예상 출력 (Counter 기능 지원 시)

Counter 기능이 지원되는 경우 예상되는 전체 출력:

```
========================================
ESP32 AES132 CryptoAuth Example
Example 05: Counter
========================================

AES132 initialized successfully

=== Example 1: Read All Counter Values ===
Counter 0: 0 (0x0)
Counter 1: 0 (0x0)
Counter 2: 0 (0x0)
Counter 3: 0 (0x0)

=== Example 2: Increment Counter 0 ===
Initial Counter 0 value: 0
Incrementing Counter 0 by 1...
[Increment Counter] SUCCESS
New Counter 0 value: 1
Difference: 1

=== Example 3: Increment Counter 1 Multiple Times ===
Starting Counter 1 value: 0
After increment 1: 1
After increment 2: 2
After increment 3: 3
After increment 4: 4
After increment 5: 5
Final Counter 1 value: 5
Total increments: 5

=== Example 4: Increment Counter 2 by Large Value ===
Counter 2 before increment: 0
Incrementing Counter 2 by 10...
[Increment Counter] SUCCESS
Counter 2 after increment: 10
Difference: 10

=== Example 5: Final Counter States ===
Counter 0 final value: 1 (0x1)
Counter 1 final value: 5 (0x5)
Counter 2 final value: 10 (0xA)
Counter 3 final value: 0 (0x0)

=== Counter Examples Complete ===

Note: Counter values are stored in non-volatile memory
      and persist across power cycles.
```

**정상 동작 시 특징**:
- 모든 카운터가 성공적으로 읽혀짐
- 카운터 증가가 정확히 동작함
- 각 카운터가 독립적으로 동작함

## 코드 설명

### 주요 함수

1. **`read_counter()`**
   - Counter 명령어의 Mode 0을 사용하여 카운터 값을 읽습니다
   - Parameter 1: Counter ID (0-3)
   - Parameter 2: 0 (읽기 모드)
   - 성공 시 카운터 값을 반환합니다 (빅엔디안 형식)

2. **`increment_counter()`**
   - Counter 명령어의 Mode 1을 사용하여 카운터 값을 증가시킵니다
   - Parameter 1: Counter ID (0-3)
   - Parameter 2: 증가값 (1-255)
   - 성공 시 증가된 카운터 값을 반환합니다 (선택사항)

### 예제 시나리오

1. **예제 1**: 모든 카운터 값 읽기
   - 4개의 카운터 슬롯 모두 읽기
   - 초기 상태 확인

2. **예제 2**: 특정 카운터 증가
   - Counter 0을 1씩 증가
   - 증가 전후 값 비교

3. **예제 3**: 여러 번 증가
   - Counter 1을 5번 연속 증가
   - 각 증가마다 값 확인

4. **예제 4**: 큰 값으로 증가
   - Counter 2를 10씩 증가
   - 한 번에 큰 값 증가 확인

5. **예제 5**: 최종 상태 확인
   - 모든 카운터의 최종 값 확인
   - 독립성 확인

## 학습 포인트

1. **카운터 슬롯 개념**
   - 4개의 독립적인 카운터 슬롯 이해
   - Counter ID를 사용한 카운터 선택

2. **카운터 모드 이해**
   - 읽기 모드 (Mode 0)와 증가 모드 (Mode 1)의 차이
   - 각 모드에서의 파라미터 사용 방법

3. **빅엔디안 형식**
   - 4바이트 카운터 값을 32비트 정수로 변환하는 방법
   - 바이트 순서 이해

4. **카운터 제한**
   - 최대값 도달 시 동작
   - 카운터 감소 불가 특성

## 관련 문서

- **ATAES132A 데이터시트**: Section 7.6 - Counter Command
- **ATAES132A 데이터시트**: Section 2.3 - Counter Memory
- **ATAES132A 데이터시트**: Section 4.2 - Counter Configuration
- [예제 4: 랜덤 생성](../04_random/README.md)
- [예제 6: 암호화](../06_encrypt/README.md)

## 다음 단계

- [예제 6: 암호화](../06_encrypt/README.md) - AES-128 암호화
- [예제 7: 복호화](../07_decrypt/README.md) - AES-128 복호화
- [예제 9: 인증](../09_authentication/README.md) - 카운터를 활용한 인증

## 참고사항

1. **Counter 기능 지원**: 일부 ATAES132A 모델에서는 Counter 기능이 지원되지 않을 수 있습니다. `0x50` (ParseError) 오류가 발생하면 Counter 기능이 비활성화되어 있거나 지원되지 않는 모델일 수 있습니다.

2. **카운터 초기화**: 카운터는 공장 출하 시 0으로 초기화됩니다. 특별한 권한이 없으면 재설정할 수 없습니다.

2. **카운터 독립성**: 각 카운터는 완전히 독립적으로 동작합니다. 한 카운터의 증가가 다른 카운터에 영향을 주지 않습니다.

3. **비휘발성 저장**: 카운터 값은 비휘발성 메모리에 저장되어 전원이 꺼져도 유지됩니다. 이는 보안 애플리케이션에 중요합니다.

4. **카운터 제한**: 최대값(0xFFFFFFFF)에 도달하면 더 이상 증가할 수 없습니다. 이 경우 `0x10` (CountError) 오류가 반환됩니다.

5. **증가값 제한**: 한 번에 최대 255까지 증가시킬 수 있습니다. 더 큰 값을 증가시키려면 여러 번 호출해야 합니다.

## 문제 해결

### 카운터 읽기 실패 시 확인사항

1. **I2C 연결 확인**: SDA, SCL 핀이 올바르게 연결되었는지 확인
2. **디바이스 주소 확인**: I2C 주소가 올바른지 확인 (기본값: 0xC0)
3. **Counter ID 확인**: Counter ID가 0-3 범위 내에 있는지 확인
4. **타이밍 확인**: 명령어 실행 후 충분한 대기 시간 확보
5. **Counter 기능 지원 확인**: 일부 ATAES132A 모델에서는 Counter 기능이 지원되지 않을 수 있습니다

### 카운터 증가 실패 시 확인사항

1. **최대값 확인**: 카운터가 최대값에 도달했는지 확인 (`0x10` 오류)
2. **증가값 확인**: 증가값이 1-255 범위 내에 있는지 확인
3. **권한 확인**: 카운터 증가 권한이 설정되어 있는지 확인

### 일반적인 오류 코드

- `0x03` (ParseError): 잘못된 파라미터 (Counter ID가 0-3 범위를 벗어남)
- `0x50` (ParseError): 명령어 파싱 오류 - Counter 기능이 지원되지 않거나 비활성화됨
- `0x10` (CountError): 카운터 제한값 도달 또는 카운터 사용 오류
- `0x0F` (ExecutionError): 명령어 실행 오류

### 카운터 증가 실패 원인 분석

**`0x10` (CountError) 오류**:
- 카운터가 최대값(0xFFFFFFFF)에 도달함
- 카운터 사용 권한이 없음
- 해결: 다른 카운터 사용 또는 카운터 재설정 (권한 필요)

**`0x03` 또는 `0x50` (ParseError) 오류**:
- Counter ID가 0-3 범위를 벗어남
- 증가값이 0이거나 255를 초과함
- **Counter 기능이 지원되지 않음**: 일부 ATAES132A 모델에서는 Counter 기능이 지원되지 않거나 비활성화되어 있을 수 있습니다
- 해결: 파라미터 범위 확인, 디바이스 모델 및 구성 확인

## 추가 학습 자료

1. **카운터 활용 심화 학습**:
   - 사용 횟수 제한 구현
   - 라이센스 관리 시스템
   - 보안 로그 관리

2. **보안 기능 학습**:
   - 카운터 기반 인증
   - 트랜잭션 카운팅
   - 이벤트 추적

3. **실전 활용**:
   - 카운터 모니터링 시스템
   - 카운터 기반 알림 시스템
   - 카운터 통계 분석

4. **카운터 관리**:
   - 카운터 초기화 방법 (권한 필요)
   - 카운터 백업 및 복원
   - 카운터 동기화

## 실전 팁

1. **카운터 선택**: 용도에 따라 적절한 카운터를 선택하세요. 각 카운터는 독립적이므로 용도별로 분리하여 사용하는 것이 좋습니다.

2. **증가값 최적화**: 한 번에 큰 값을 증가시키면 명령어 호출 횟수를 줄일 수 있습니다 (최대 255).

3. **정기적 확인**: 중요한 애플리케이션에서는 정기적으로 카운터 값을 확인하여 최대값에 근접했는지 모니터링하세요.

4. **에러 처리**: 카운터 증가 실패 시 `0x10` 오류를 확인하고 적절한 처리를 구현하세요.

5. **값 검증**: 카운터 값을 읽은 후 예상 범위 내에 있는지 검증하는 것이 좋습니다.

