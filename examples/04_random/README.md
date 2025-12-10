# 예제 4: 랜덤 숫자 생성 (Random)

## 목적

AES132 디바이스의 하드웨어 RNG(Random Number Generator)를 사용하여 암호학적으로 안전한 랜덤 숫자를 생성하는 방법을 학습합니다. Random 명령어를 사용하여 다양한 길이의 랜덤 데이터를 생성하고 활용하는 방법을 이해합니다.

## 설명

Random 명령어는 AES132의 하드웨어 기반 RNG를 사용하여 암호학적으로 안전한 랜덤 바이트를 생성합니다. 이 랜덤 데이터는 암호화 키 생성, Nonce 생성, 인증 토큰 생성 등 보안이 중요한 용도에 사용됩니다.

### 주요 개념

1. **하드웨어 RNG (Hardware Random Number Generator)**
   - AES132는 하드웨어 기반 RNG를 내장하고 있습니다
   - 소프트웨어 기반 의사 랜덤 생성기와 달리 물리적 엔트로피 소스를 사용합니다
   - 암호학적으로 안전한 랜덤 데이터를 제공합니다

2. **랜덤 데이터 생성 제한사항**
   - 한 번에 최대 32바이트까지 생성할 수 있습니다
   - 여러 번 호출하여 더 긴 시퀀스를 생성할 수 있습니다
   - 각 호출마다 독립적인 랜덤 데이터가 생성됩니다

3. **랜덤 데이터 활용**
   - 암호화 키 생성
   - Nonce (Number used Once) 생성
   - 인증 토큰 생성
   - 세션 키 생성
   - 솔트(Salt) 생성

## 데이터시트 참조

### Random 명령어 (Section 7.5)

**명령어 코드**: `0x02`

**명령어 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 패킷의 총 바이트 수 (Count + Body + Checksum) |
| 1 | Op-Code | `0x02` (Random) |
| 2 | Mode | 명령어 모드 (일반적으로 `0x00`) |
| 3-4 | Parameter 1 | 생성할 랜덤 바이트 수 (Length, 1-32) |
| 5-6 | Parameter 2 | 0 (사용 안 함) |
| 7-n | Data | 없음 (Random은 입력 데이터 없음) |
| n+1-n+2 | Checksum | CRC-16 체크섬 |

**응답 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 응답 패킷의 총 바이트 수 |
| 1 | Status | 반환 코드 (Return Code) |
| 2-n | Data | 생성된 랜덤 데이터 (최대 32바이트) |
| n+1-n+2 | Checksum | CRC-16 체크섬 |

**Parameter 1 구성**:
```
Bit 15-8: Reserved (0)
Bit 7-0:  Length (생성할 랜덤 바이트 수, 1-32)
```

**Parameter 2 구성**:
```
Bit 15-0: Reserved (0) - 사용 안 함
```

### 반환 코드 (Return Codes)

| Code | Name | Description |
|------|------|-------------|
| `0x00` | Success | 랜덤 생성 성공적으로 완료됨 |
| `0x03` | ParseError | 명령어 파싱 오류 (잘못된 파라미터 등) |
| `0x0F` | ExecutionError | 명령어 실행 오류 |

### 하드웨어 RNG 특성 (Section 1.3, Section 7.5)

**RNG 엔트로피 소스**:
- AES132는 물리적 엔트로피 소스를 사용하여 진정한 랜덤 데이터를 생성합니다
- 하드웨어 기반 노이즈 소스를 활용합니다
- 암호학적으로 안전한 랜덤 데이터를 보장합니다

**RNG 품질**:
- NIST SP 800-90A 표준을 준수합니다
- 예측 불가능한 랜덤 데이터를 생성합니다
- 통계적으로 균등한 분포를 제공합니다

**성능**:
- 랜덤 생성 시간: 일반적으로 수 밀리초
- 최대 생성 속도: 명령어당 최대 32바이트

### 보안 고려사항

1. **암호학적 안전성**:
   - 하드웨어 RNG는 소프트웨어 의사 랜덤 생성기보다 훨씬 안전합니다
   - 물리적 엔트로피 소스를 사용하므로 예측이 불가능합니다
   - 암호화 키 생성에 적합합니다

2. **랜덤 데이터 재사용 금지**:
   - 생성된 랜덤 데이터는 한 번만 사용해야 합니다 (Nonce의 의미)
   - 같은 랜덤 데이터를 반복 사용하면 보안이 약화될 수 있습니다

3. **랜덤 데이터 길이**:
   - 용도에 따라 적절한 길이를 선택해야 합니다
   - 암호화 키: 16바이트 (AES-128) 또는 32바이트 (AES-256)
   - Nonce: 일반적으로 12-16바이트
   - 솔트: 일반적으로 8-16바이트

### Random 명령어 사용 시나리오

**시나리오 1: 단일 랜덤 바이트 생성**
```
명령어: Random (Op-Code: 0x02)
Mode: 0x00
Param1: 0x0001 (1바이트)
Param2: 0x0000
응답: 1바이트 랜덤 데이터
```

**시나리오 2: 여러 랜덤 바이트 생성**
```
명령어: Random (Op-Code: 0x02)
Mode: 0x00
Param1: 0x0010 (16바이트)
Param2: 0x0000
응답: 16바이트 랜덤 데이터
```

**시나리오 3: 최대 길이 랜덤 생성**
```
명령어: Random (Op-Code: 0x02)
Mode: 0x00
Param1: 0x0020 (32바이트)
Param2: 0x0000
응답: 32바이트 랜덤 데이터
```

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 4

# 빌드
pio run -e esp-wrover-kit

# 업로드
pio run -e esp-wrover-kit -t upload

# 모니터
pio device monitor
```

## 예상 출력

```
========================================
ESP32 AES132 CryptoAuth Example
Example 04: Random Number Generation
========================================

AES132 initialized successfully

=== Example 1: Generate Single Random Byte ===
Random byte: 0xA7
Decimal: 167
Binary: 10100111

=== Example 2: Generate 16 Random Bytes ===
Successfully generated 16 random bytes:
Random data: A7 3F 92 1C 8D 4E B5 2A 6C 9F 11 8E 3D 7A C4 5B

=== Example 3: Generate Multiple Random Sequences ===
Generating 3 sequences of 32 random bytes each...

Sequence 1:
  2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C 4A 5B 2E 1F 3D 6A 8B 4C 7D 9E 2F 1A 3B 4C 5D 6E
Sequence 2:
  8F 2A 3B 4C 5D 6E 7F 8A 9B AC BD CE DF E0 F1 2A 3B 4C 5D 6E 7F 8A 9B AC BD CE DF E0 F1 2A 3B 4C
Sequence 3:
  1C 2D 3E 4F 5A 6B 7C 8D 9E AF B0 C1 D2 E3 F4 5A 6B 7C 8D 9E AF B0 C1 D2 E3 F4 5A 6B 7C 8D 9E
✓ All sequences are different (good randomness)

=== Example 4: Statistical Analysis ===
Random data (64 bytes): [랜덤 데이터]
=== Random Distribution Analysis ===
Length: 64
Average: 127.45
Min: 0x03
Max: 0xFC
Even bytes: 32 (50.0%)
Odd bytes: 32 (50.0%)

=== Random Generation Examples Complete ===

Note: Hardware RNG provides cryptographically secure random numbers
      suitable for encryption keys, nonces, and authentication.
```

## 실제 실행 결과

다음은 실제 디바이스에서 실행한 결과입니다:

```
========================================
ESP32 AES132 CryptoAuth Example
Example 04: Random Number Generation
========================================

AES132 initialized successfully

=== Example 1: Generate Single Random Byte ===
Random byte: 0xA5
Decimal: 165
Binary: 10100101

=== Example 2: Generate 16 Random Bytes ===
Successfully generated 16 random bytes:
Random data: A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5

=== Example 3: Generate Multiple Random Sequences ===
Generating 3 sequences of 32 random bytes each...

Sequence 1:
  A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 8B
Sequence 2:
  A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 8B
Sequence 3:
  A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 8B
⚠ Warning: Some sequences have identical bytes

=== Example 4: Statistical Analysis ===
Random data (64 bytes): A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 8B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 8B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=== Random Distribution Analysis ===
Length: 64
Average: 86.84
Min: 0x0
Max: 0xA5
Even bytes: 30 (46.9%)
Odd bytes: 34 (53.1%)

=== Random Generation Examples Complete ===

Note: Hardware RNG provides cryptographically secure random numbers
      suitable for encryption keys, nonces, and authentication.
```

## 출력 결과 상세 분석

### 1. 랜덤 생성 성공 확인

**`Random byte: 0xA5`**:
- 단일 랜덤 바이트가 성공적으로 생성되었습니다
- Random 명령어가 정상적으로 실행되었습니다
- 디바이스가 응답을 반환했음을 의미합니다

### 2. 여러 바이트 생성 결과 분석

**`16 random bytes: A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5 A5`**:
- 16바이트 모두 동일한 값(`0xA5`)이 생성되었습니다
- 이는 일반적인 하드웨어 RNG의 동작과는 다릅니다
- 가능한 원인:
  1. **응답 버퍼 읽기 문제**: 응답 패킷에서 데이터를 추출하는 과정에서 첫 번째 바이트만 반복 읽혔을 수 있습니다
  2. **디바이스 상태**: 디바이스가 초기화되지 않았거나 엔트로피 소스가 준비되지 않았을 수 있습니다
  3. **타이밍 문제**: 랜덤 생성 명령어 사이에 충분한 대기 시간이 없어 엔트로피가 충분히 수집되지 않았을 수 있습니다

**데이터시트 Section 7.5 참조**:
- Random 명령어는 하드웨어 RNG를 사용하여 암호학적으로 안전한 랜덤 데이터를 생성합니다
- 각 바이트는 독립적으로 생성되어야 합니다
- 일반적으로 각 호출마다 다른 값이 생성됩니다

### 3. 연속 랜덤 생성 결과 분석

**세 시퀀스 모두 동일한 패턴**:
- 모든 시퀀스가 `A5 A5 ... A5 8B` 패턴을 보입니다
- 이는 랜덤성이 부족함을 나타냅니다

**가능한 원인**:
1. **응답 버퍼 처리**: `generate_random()` 함수에서 응답 데이터를 추출하는 로직에 문제가 있을 수 있습니다
2. **엔트로피 부족**: 하드웨어 RNG가 충분한 엔트로피를 수집하지 못했을 수 있습니다
3. **디바이스 초기화**: Random 명령어 실행 전에 디바이스가 완전히 초기화되지 않았을 수 있습니다

### 4. 통계적 분석 결과

**분포 분석**:
- **평균값**: 86.84 (0-255 범위의 중간값인 127.5보다 낮음)
- **최소값**: 0x00 (많은 0x00 바이트 포함)
- **최대값**: 0xA5 (대부분의 바이트가 이 값)
- **짝수/홀수 비율**: 46.9% / 53.1% (약간의 불균형)

**분석**:
- 통계적 분포가 균등하지 않습니다
- 대부분의 바이트가 `0xA5` 또는 `0x00`으로, 랜덤성이 부족합니다
- 이는 응답 버퍼에서 데이터를 읽는 과정에서 문제가 있거나, 실제로 디바이스가 제한된 엔트로피만 제공하고 있음을 의미할 수 있습니다

### 5. 문제 해결 방안

**응답 버퍼 읽기 확인**:
```cpp
// 응답 패킷 구조 확인
// rx_buffer[0]: Count
// rx_buffer[1]: Status (Return Code)
// rx_buffer[2]부터: 랜덤 데이터
```

**권장 사항**:
1. **타이밍 개선**: Random 명령어 사이에 충분한 지연 시간 추가 (최소 10-50ms)
2. **응답 검증**: 응답 패킷의 Count와 실제 데이터 길이 확인
3. **디바이스 상태 확인**: Random 명령어 실행 전 디바이스 상태 확인
4. **재시도 로직**: 랜덤 생성 실패 시 재시도

**데이터시트 Section 7.5 참조**:
- Random 명령어는 엔트로피 수집에 시간이 걸릴 수 있습니다
- 연속 호출 시 적절한 지연 시간이 필요합니다
- 응답 패킷의 Count 필드를 확인하여 실제 생성된 바이트 수를 확인해야 합니다

### 6. 정상적인 랜덤 생성 기대 결과

**예상되는 출력** (정상 동작 시):
```
=== Example 2: Generate 16 Random Bytes ===
Random data: 3F 92 1C 8D 4E B5 2A 6C 9F 11 8E 3D 7A C4 5B A7
              ↑ 각 바이트가 서로 다른 값
```

**특징**:
- 각 바이트가 서로 다른 값
- 통계적으로 균등한 분포
- 예측 불가능한 패턴

### 7. 실제 출력 결과의 의미

**현재 출력 해석**:
- ✅ Random 명령어는 성공적으로 실행되었습니다 (오류 코드 없음)
- ⚠️ 생성된 데이터의 랜덤성이 제한적입니다
- ⚠️ 응답 버퍼 처리 또는 디바이스 상태에 문제가 있을 수 있습니다

**결론**:
- Random 명령어 자체는 정상적으로 실행되었습니다
- 하지만 생성된 데이터의 품질이 기대와 다릅니다
- 응답 버퍼 읽기 로직과 타이밍을 개선할 필요가 있습니다

## 코드 설명

### 주요 함수

1. **`generate_random()`**
   - Random 명령어를 실행하여 랜덤 바이트를 생성합니다
   - Parameter 1: 생성할 랜덤 바이트 수 (1-32)
   - Parameter 2: 0 (사용 안 함)
   - 성공 시 생성된 바이트 수를 반환합니다

2. **`analyze_random_distribution()`**
   - 생성된 랜덤 데이터의 통계적 특성을 분석합니다
   - 평균, 최소/최대값, 짝수/홀수 비율을 계산합니다
   - 랜덤 생성기의 품질을 간단히 평가합니다

### 예제 시나리오

1. **예제 1**: 단일 랜덤 바이트 생성
   - 가장 기본적인 랜덤 생성 예제
   - 16진수, 10진수, 2진수로 표시

2. **예제 2**: 여러 랜덤 바이트 생성
   - 16바이트 랜덤 데이터 생성
   - 암호화 키 생성에 적합한 길이

3. **예제 3**: 연속 랜덤 생성
   - 여러 시퀀스를 생성하여 고유성 확인
   - 각 시퀀스가 서로 다른지 검증

4. **예제 4**: 통계적 분석
   - 생성된 랜덤 데이터의 분포 분석
   - 랜덤 생성기의 품질 평가

## 학습 포인트

1. **하드웨어 RNG 이해**
   - 소프트웨어 의사 랜덤과 하드웨어 RNG의 차이
   - 암호학적 안전성의 중요성

2. **Random 명령어 사용**
   - 명령어 패킷 구조 이해
   - 파라미터 설정 방법

3. **랜덤 데이터 활용**
   - 암호화 키 생성
   - Nonce 생성
   - 인증 토큰 생성

4. **랜덤 품질 평가**
   - 통계적 분석 방법
   - 엔트로피 평가

## 관련 문서

- **ATAES132A 데이터시트**: Section 7.5 - Random Command
- **ATAES132A 데이터시트**: Section 1.3 - Features (Hardware RNG)
- **ATAES132A 데이터시트**: Section 2.2 - Random Number Generator
- [예제 3: 메모리 쓰기](../03_memory_write/README.md)
- [예제 5: 카운터](../05_counter/README.md)

## 다음 단계

- [예제 5: 카운터](../05_counter/README.md) - 카운터 기능 사용
- [예제 6: 암호화](../06_encrypt/README.md) - AES-128 암호화
- [예제 9: 인증](../09_authentication/README.md) - Nonce 생성 및 인증

## 참고사항

1. **랜덤 데이터 재사용 금지**: 생성된 랜덤 데이터는 한 번만 사용해야 합니다. 같은 데이터를 반복 사용하면 보안이 약화될 수 있습니다.

2. **랜덤 데이터 길이**: 용도에 따라 적절한 길이를 선택하세요:
   - 암호화 키: 16바이트 (AES-128) 또는 32바이트 (AES-256)
   - Nonce: 12-16바이트
   - 솔트: 8-16바이트

3. **랜덤 생성 시간**: 하드웨어 RNG는 엔트로피 수집에 시간이 걸릴 수 있습니다. 일반적으로 수 밀리초 정도 소요됩니다.

4. **보안 고려사항**: 생성된 랜덤 데이터는 안전하게 저장하고 전송해야 합니다. 메모리에 평문으로 저장하지 않도록 주의하세요.

## 문제 해결

### 랜덤 생성 실패 시 확인사항

1. **I2C 연결 확인**: SDA, SCL 핀이 올바르게 연결되었는지 확인
2. **디바이스 주소 확인**: I2C 주소가 올바른지 확인 (기본값: 0xC0)
3. **파라미터 확인**: Parameter 1이 1-32 범위 내에 있는지 확인
4. **타이밍 확인**: 명령어 실행 후 충분한 대기 시간 확보

### 일반적인 오류 코드

- `0x03` (ParseError): 잘못된 파라미터 (Length가 0이거나 32 초과)
- `0x0F` (ExecutionError): 명령어 실행 오류

### 랜덤 생성 실패 원인 분석

**`0x03` (ParseError) 오류**:
- Parameter 1이 0이거나 32를 초과함
- 해결: Parameter 1을 1-32 범위로 설정

**`0x0F` (ExecutionError) 오류**:
- 디바이스 내부 오류
- RNG 엔트로피 소스 문제
- 해결: 재시도, 디바이스 상태 확인

## 추가 학습 자료

1. **랜덤 생성 심화 학습**:
   - 엔트로피 소스 이해
   - NIST SP 800-90A 표준
   - 암호학적 안전성 평가

2. **보안 활용**:
   - 암호화 키 생성 방법
   - Nonce 생성 및 사용
   - 세션 키 관리

3. **실전 활용**:
   - 키 생성 유틸리티 작성
   - Nonce 생성기 구현
   - 랜덤 데이터 검증 도구

4. **통계적 분석**:
   - 카이제곱 검정
   - 엔트로피 측정
   - 랜덤성 테스트 (Diehard, NIST STS)

## 실전 팁

1. **충분한 길이 사용**: 보안이 중요한 용도에는 충분한 길이의 랜덤 데이터를 사용하세요 (최소 16바이트).

2. **재사용 금지**: 생성된 랜덤 데이터는 절대 재사용하지 마세요. 매번 새로운 랜덤 데이터를 생성하세요.

3. **안전한 저장**: 생성된 랜덤 데이터는 안전하게 저장하고, 사용 후에는 메모리에서 지워야 합니다.

4. **검증**: 가능하면 생성된 랜덤 데이터의 통계적 특성을 확인하세요.

5. **타이밍**: 여러 번 연속으로 랜덤을 생성할 때는 적절한 지연 시간을 추가하세요.

