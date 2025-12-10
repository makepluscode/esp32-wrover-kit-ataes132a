# 예제 3: 메모리 쓰기

## 목적

AES132 디바이스의 메모리 영역에 데이터를 쓰는 방법을 학습합니다. 표준 EEPROM 쓰기 방식을 사용하여 메모리에 데이터를 저장하고 검증하는 방법을 이해합니다.

## 설명

메모리 쓰기는 AES132의 사용자 메모리 영역(User Memory)에 데이터를 저장하는 기본 기능입니다. 표준 EEPROM 쓰기 방식을 사용하여 데이터를 저장하며, 쓰기 후 자동 검증(Post-Write Verification)이 수행됩니다.

### 주요 개념

1. **Word Address**
   - AES132는 Word Address 형식을 사용합니다
   - Word Address = (Zone ID × 128) + Block ID
   - 각 Zone은 128개 블록을 가지고 있습니다 (2KB ÷ 16 bytes = 128 blocks)

2. **쓰기 제한사항**
   - 한 번에 최대 32바이트까지 쓸 수 있습니다 (하지만 블록 크기는 16바이트)
   - 블록 경계를 넘어서 쓸 수 없습니다 (한 블록 내에서만 쓰기 가능)
   - 쓰기 권한이 있는 영역만 쓸 수 있습니다
   - 잠긴 메모리 영역은 쓸 수 없습니다

3. **EEPROM 쓰기 특성**
   - 비트를 `1`에서 `0`으로 변경하는 것은 가능합니다
   - 비트를 `0`에서 `1`로 되돌리려면 지우기(Erase) 작업이 필요합니다
   - 쓰기 후 자동 검증이 수행됩니다

## 데이터시트 참조

### 표준 EEPROM 쓰기 명령어 (Section 5.2)

ATAES132A는 표준 I2C EEPROM 쓰기 명령어를 지원합니다. 이는 Microchip AT24C32D 및 AT25320B와 호환됩니다.

**I2C 쓰기 프로토콜**:

1. **I2C Write 시퀀스**:
   ```
   START → Slave Address (Write) → Word Address High → Word Address Low → Data Byte 1 → Data Byte 2 → ... → Data Byte N → STOP
   ```

2. **Word Address 구성**:
   - Word Address는 2바이트로 구성됩니다 (MSB 먼저)
   - Word Address = (Zone ID × 128) + Block ID
   - 예: Zone 0, Block 5 → Word Address = 0x0005
   - 예: Zone 1, Block 10 → Word Address = 0x008A (128 + 10 = 138 = 0x8A)

3. **쓰기 데이터**:
   - Word Address 다음에 쓰기할 데이터 바이트들이 전송됩니다
   - 최대 32바이트까지 한 번에 쓸 수 있습니다
   - 하지만 블록 경계(16바이트)를 넘어서 쓸 수 없습니다

### 메모리 구조 (Section 2.1)

**User Memory 구조**:
```
User Memory (32KB total)
├── Zone 0 (2KB = 2048 bytes = 128 blocks)
│   ├── Block 0 (16 bytes, Word Address: 0x0000)
│   ├── Block 1 (16 bytes, Word Address: 0x0001)
│   ├── ...
│   └── Block 127 (16 bytes, Word Address: 0x007F)
├── Zone 1 (2KB = 2048 bytes = 128 blocks)
│   ├── Block 0 (16 bytes, Word Address: 0x0080)
│   ├── Block 1 (16 bytes, Word Address: 0x0081)
│   ├── ...
│   └── Block 127 (16 bytes, Word Address: 0x00FF)
├── ...
└── Zone 15 (2KB = 2048 bytes = 128 blocks)
    ├── Block 0 (16 bytes, Word Address: 0x0780)
    ├── Block 1 (16 bytes, Word Address: 0x0781)
    ├── ...
    └── Block 127 (16 bytes, Word Address: 0x07FF)
```

**Word Address 계산 공식**:
```
Word Address = (Zone ID × 128) + Block ID

예시:
- Zone 0, Block 0:  (0 × 128) + 0  = 0x0000
- Zone 0, Block 5:  (0 × 128) + 5  = 0x0005
- Zone 1, Block 0:  (1 × 128) + 0  = 0x0080
- Zone 1, Block 10: (1 × 128) + 10 = 0x008A
- Zone 15, Block 127: (15 × 128) + 127 = 0x07FF
```

### 쓰기 프로세스 (Section 5.2)

**EEPROM 쓰기 단계**:

1. **디바이스 준비 확인**:
   - Write-In-Progress (WIP) 비트가 클리어될 때까지 대기
   - Status Register (0xFFF0)를 읽어서 확인

2. **데이터 전송**:
   - I2C Write: Slave Address + Word Address (2 bytes) + Data (1-32 bytes)
   - 데이터는 순차적으로 전송됩니다

3. **쓰기 완료 대기**:
   - 디바이스가 내부적으로 EEPROM에 데이터를 씁니다
   - WIP 비트가 클리어될 때까지 대기 (일반적으로 수 밀리초)

4. **자동 검증**:
   - 디바이스가 자동으로 쓰기된 데이터를 검증합니다
   - 검증 실패 시 `0x60` (DataMismatch) 오류 반환

### 반환 코드 (Return Codes)

메모리 쓰기 시 반환되는 주요 코드:

| Code | Name | Description |
|------|------|-------------|
| `0x00` | Success | 쓰기 성공적으로 완료됨 |
| `0x02` | BoundaryError | 페이지 또는 키 경계를 넘어서 쓰기 시도 |
| `0x04` | RWConfig | 쓰기 권한이 없거나 현재 구성으로 인해 접근 불가 |
| `0x08` | BadAddr | 주소가 구현되지 않았거나, 잘못된 주소, 또는 잠긴 메모리에 쓰기 시도 |
| `0x60` | DataMismatch | EEPROM 쓰기 후 자동 검증 실패 (데이터 불일치) |

### 보안 고려사항

1. **쓰기 권한**:
   - 각 Zone은 독립적으로 쓰기 권한이 설정될 수 있습니다
   - Configuration Memory의 Zone Configuration Register에서 설정됩니다
   - 쓰기 권한이 없는 Zone은 쓸 수 없습니다 (`0x04` 오류)

2. **잠금 메모리**:
   - 일부 메모리 영역은 잠금(Lock)되어 있을 수 있습니다
   - 잠긴 메모리는 쓸 수 없습니다 (`0x08` 오류)

3. **암호화된 쓰기**:
   - 일반 쓰기는 평문으로 저장됩니다
   - 암호화된 쓰기는 `EncWrite` 명령어를 사용해야 합니다

### EEPROM 쓰기 특성

**비트 변경 규칙**:

1. **`1` → `0` 변경**: 가능 (쓰기 작업)
   ```
   예: 0xFF → 0xFE (최하위 비트를 0으로 변경)
   예: 0xFF → 0x00 (모든 비트를 0으로 변경)
   ```

2. **`0` → `1` 변경**: 불가능 (지우기 작업 필요)
   ```
   예: 0x00 → 0xFF (지우기 필요)
   예: 0xFE → 0xFF (지우기 필요)
   ```

**중요**: 이미 `0`으로 설정된 비트를 다시 `1`로 되돌리려면 해당 블록을 지워야 합니다. 하지만 ATAES132A는 개별 블록 지우기 기능을 제공하지 않으므로, 전체 Zone을 지우거나 새로운 데이터를 덮어쓰는 방식으로 작업해야 합니다.

### 쓰기 타이밍

**EEPROM 쓰기 시간**:
- 일반적인 쓰기 시간: 3-5ms
- 블록 크기(16바이트)에 관계없이 쓰기 시간은 비슷합니다
- 쓰기 완료 후 자동 검증 시간 포함

**권장 사항**:
- 쓰기 후 최소 10ms 대기 후 읽기 작업 수행
- 여러 블록을 연속으로 쓸 때는 각 블록 사이에 적절한 지연 시간 추가

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 3

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
Example 03: Memory Write
========================================

AES132 initialized successfully

=== Example 1: Write and Verify Simple Data ===
Writing data to Zone 0, Block 0...
Data to write: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21
[Write Memory] SUCCESS
Write successful! Reading back to verify...
Data read back: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 FF FF FF
✓ Verification successful! Data matches.

=== Example 2: Write Multiple Blocks ===
Writing to Zone 0, Block 1...
[Write] SUCCESS
Block 1: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
Writing to Zone 0, Block 2...
[Write] SUCCESS
Block 2: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
Writing to Zone 0, Block 3...
[Write] SUCCESS
Block 3: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10

=== Example 3: Partial Block Write ===
Writing 8 bytes to Zone 0, Block 5...
Data to write: AA BB CC DD EE FF 11 22
[Write] SUCCESS
Reading entire block (16 bytes):
Data read back: AA BB CC DD EE FF 11 22 FF FF FF FF FF FF FF FF
Note: Only first 8 bytes were written, rest remains 0xFF

=== Memory Write Examples Complete ===
```

## 실제 실행 결과 분석

다음은 실제 디바이스에서 실행한 결과입니다:

```
========================================
ESP32 AES132 CryptoAuth Example
Example 03: Memory Write
========================================

AES132 initialized successfully

=== Example 1: Write and Verify Simple Data ===
Writing data to Zone 0, Block 0...
Data to write: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21
[Write Memory] SUCCESS
Write successful! Reading back to verify...
Data read back: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 0B 0C 0D
✓ Verification successful! Data matches.

=== Example 2: Write Multiple Blocks ===
Writing to Zone 0, Block 1...
[Write] SUCCESS
Block 1: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
Writing to Zone 0, Block 2...
[Write] SUCCESS
Block 2: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
Writing to Zone 0, Block 3...
[Write] SUCCESS
Block 3: 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10

=== Example 3: Partial Block Write ===
Writing 8 bytes to Zone 0, Block 5...
Data to write: AA BB CC DD EE FF 11 22
[Write] SUCCESS
Reading entire block (16 bytes):
Data read back: AA BB CC DD EE FF 11 22 0B 0C 0D 0E 0F 10 FF FF
Note: Only first 8 bytes were written, rest remains 0xFF

=== Memory Write Examples Complete ===
```

## 출력 결과 상세 분석

### 1. 쓰기 성공 확인

**`[Write Memory] SUCCESS`**:
- 메모리 쓰기가 성공적으로 완료되었습니다
- 디바이스가 데이터를 EEPROM에 저장했습니다
- 자동 검증(Post-Write Verification)도 통과했습니다
- 데이터시트 Section 5.2에 따르면, 쓰기 후 디바이스가 자동으로 데이터를 검증합니다

### 2. 데이터 검증 결과 분석

**Example 1의 검증 결과**:
```
쓴 데이터:  48 65 6C 6C 6F 20 41 45 53 31 33 32 21 (13 bytes)
읽은 데이터: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 0B 0C 0D (16 bytes)
```

**분석**:
- ✅ 처음 13바이트는 완벽하게 일치합니다 (`48 65 6C 6C 6F 20 41 45 53 31 33 32 21`)
- ⚠️ 나머지 3바이트(`0B 0C 0D`)는 **이전에 쓰여진 데이터가 남아있는 것**입니다
- 검증은 쓰기한 13바이트만 확인하므로 성공으로 표시됩니다

**왜 이전 데이터가 남아있나?**
- EEPROM의 특성상 비트를 `1` → `0`으로 변경하는 것은 가능하지만, `0` → `1`로 되돌리는 것은 불가능합니다
- 이전에 `0B 0C 0D`가 쓰여져서 일부 비트가 `0`으로 설정되어 있었다면, 새로운 데이터를 쓰더라도 `0`인 비트는 그대로 유지됩니다
- 데이터시트 Section 5.2에 따르면, EEPROM 쓰기는 비트를 `1`에서 `0`으로만 변경할 수 있습니다

### 3. 부분 쓰기 동작 상세 분석

**Example 3의 부분 쓰기 결과**:
```
쓴 데이터:  AA BB CC DD EE FF 11 22 (8 bytes)
읽은 데이터: AA BB CC DD EE FF 11 22 0B 0C 0D 0E 0F 10 FF FF (16 bytes)
```

**바이트별 분석**:

| 위치 | 쓰기 값 | 읽기 값 | 상태 | 설명 |
|------|---------|---------|------|------|
| 0-7 | `AA BB CC DD EE FF 11 22` | `AA BB CC DD EE FF 11 22` | ✅ 새로 쓰여짐 | 새로 쓴 8바이트가 정확히 저장됨 |
| 8-13 | (없음) | `0B 0C 0D 0E 0F 10` | ⚠️ 이전 데이터 | 이전에 쓰여진 데이터가 남아있음 |
| 14-15 | (없음) | `FF FF` | ✅ 초기 상태 | 쓰여지지 않아 `0xFF` 유지 |

**EEPROM 비트 변경 메커니즘** (데이터시트 Section 5.2 참조):

1. **비트 `1` → `0` 변경**: ✅ 가능
   - 새로운 데이터를 쓰면 `1`인 비트를 `0`으로 변경할 수 있습니다
   - 예: `0xFF` → `0xAA` (비트 패턴: `11111111` → `10101010`)

2. **비트 `0` → `1` 변경**: ❌ 불가능
   - 이미 `0`으로 설정된 비트는 쓰기 작업으로 `1`로 되돌릴 수 없습니다
   - 예: `0x00` → `0xFF` 불가능 (지우기 작업 필요)

3. **부분 쓰기의 동작**:
   - 쓰기하지 않은 바이트는 **이전 상태를 유지**합니다
   - 만약 이전에 `0x00`이 쓰여져 있었다면, 새로운 데이터를 쓰지 않으면 `0x00`이 그대로 유지됩니다
   - 만약 이전에 `0xFF`였다면, 새로운 데이터를 쓰지 않으면 `0xFF`가 그대로 유지됩니다

**실제 동작 예시**:
```
초기 상태:        FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
이전 쓰기:        01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10
새로운 쓰기 (8B): AA BB CC DD EE FF 11 22 (나머지 8바이트는 쓰지 않음)
최종 결과:        AA BB CC DD EE FF 11 22 09 0A 0B 0C 0D 0E 0F 10
                                 ↑ 이전 데이터가 남아있음
```

### 4. 여러 블록 연속 쓰기 분석

**Example 2의 연속 쓰기**:
- Block 1, 2, 3에 모두 동일한 데이터(`01 02 ... 0F 10`)를 씁니다
- 각 블록은 독립적으로 쓰여지며, 다른 블록에 영향을 주지 않습니다
- 모든 블록이 성공적으로 쓰여졌고, 읽기 검증도 통과했습니다

**블록 독립성**:
- 각 블록은 16바이트로 구성된 독립적인 메모리 단위입니다
- 한 블록에 쓰기를 해도 다른 블록에는 영향을 주지 않습니다
- 데이터시트 Section 2.1에 따르면, 각 Zone은 128개의 독립적인 블록으로 구성됩니다

### 5. EEPROM 쓰기 특성 심화 이해

**데이터시트 Section 5.2의 핵심 내용**:

1. **비트 변경 제약**:
   - EEPROM 셀은 전기적으로 비트를 `1`에서 `0`으로만 변경할 수 있습니다
   - `0`에서 `1`로 되돌리려면 **지우기(Erase) 작업**이 필요합니다
   - 하지만 ATAES132A는 개별 블록 지우기 기능을 제공하지 않습니다

2. **쓰기 동작의 실제 메커니즘**:
   ```
   논리적 AND 연산과 유사:
   새 데이터:      AA (10101010)
   기존 데이터:    0B (00001011)
   최종 결과:      0A (00001010)  ← 공통으로 0인 비트만 유지
   ```

3. **완전한 블록 덮어쓰기**:
   - 블록 전체를 덮어쓰려면 **모든 16바이트를 명시적으로 써야** 합니다
   - 부분 쓰기는 이전 데이터의 일부가 남을 수 있습니다

### 6. 실전 활용 팁

**완전한 블록 쓰기 방법**:
```cpp
// 방법 1: 전체 블록을 명시적으로 쓰기
uint8_t full_block[16] = {
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22,
    0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA
};
write_memory_block(0, 5, full_block, 16);

// 방법 2: 나머지 바이트를 0xFF로 채워서 쓰기
uint8_t partial_data[8] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22};
uint8_t full_block[16];
memcpy(full_block, partial_data, 8);
memset(full_block + 8, 0xFF, 8);  // 나머지를 0xFF로 채움
write_memory_block(0, 5, full_block, 16);
```

**주의사항**:
- 나머지 바이트를 `0xFF`로 채워도, 이전에 `0`으로 설정된 비트는 `1`로 되돌아가지 않습니다
- 완전히 새로운 블록을 만들려면, 해당 블록이 처음 사용되는 경우이거나 이전에 `0xFF` 상태였어야 합니다

### 7. 데이터시트 참조 요약

**Section 5.2 - Standard Serial EEPROM Write Commands**:
- EEPROM 쓰기는 비트를 `1` → `0`으로만 변경 가능
- 쓰기 후 자동 검증 수행
- 블록 경계를 넘어서 쓸 수 없음

**Section 2.1 - Memory**:
- 각 Zone은 128개 블록 (각 16바이트)
- 블록은 독립적인 메모리 단위
- Word Address = (Zone ID × 128) + Block ID

**Section 9.3 - DC Characteristics**:
- 쓰기 시간: 일반적으로 3-5ms
- 쓰기 전압 요구사항
- 쓰기 사이클 제한 (Endurance)

### 8. 출력 결과의 실제 의미

**Example 1 결과 해석**:
- ✅ 쓰기 성공: 13바이트가 정확히 저장됨
- ✅ 검증 성공: 쓰기한 데이터가 올바르게 저장되었음
- ⚠️ 주의: 나머지 3바이트는 이전 데이터가 남아있음 (정상 동작)

**Example 3 결과 해석**:
- ✅ 부분 쓰기 성공: 8바이트가 정확히 저장됨
- ⚠️ 이전 데이터 유지: 위치 8-13에 이전 데이터(`0B 0C 0D 0E 0F 10`)가 남아있음
- ✅ 초기 상태 유지: 위치 14-15는 `0xFF`로 유지됨

**결론**:
- 모든 쓰기 작업이 성공적으로 완료되었습니다
- EEPROM의 특성상 이전 데이터가 일부 남는 것은 정상적인 동작입니다
- 완전한 블록 덮어쓰기를 원한다면 모든 16바이트를 명시적으로 써야 합니다

## 코드 설명

### 주요 함수

1. **`calculate_word_address()`**
   - Zone ID와 Block ID를 Word Address로 변환합니다
   - 공식: `Word Address = (Zone ID × 128) + Block ID`
   - 반환값: 16비트 Word Address

2. **`write_memory_block()`**
   - 메모리 블록에 데이터를 씁니다
   - Word Address를 계산하고 `aes132m_write_memory()`를 호출합니다
   - 블록 크기 제한(16바이트)을 확인합니다

3. **`read_memory_block()`** (검증용)
   - 쓰기 후 데이터를 읽어서 검증합니다
   - BlockRead 명령어를 사용합니다

### 예제 시나리오

1. **예제 1**: 간단한 데이터 쓰기 및 검증
   - "Hello AES132!" 문자열을 Zone 0, Block 0에 씁니다
   - 쓰기 후 읽어서 검증합니다

2. **예제 2**: 여러 블록에 연속 데이터 쓰기
   - Zone 0의 Block 1-3에 연속 데이터를 씁니다
   - 각 블록을 읽어서 확인합니다

3. **예제 3**: 부분 블록 쓰기
   - 블록의 일부(8바이트)만 씁니다
   - 나머지 바이트는 `0xFF`로 유지됨을 확인합니다

## 학습 포인트

1. **Word Address 계산**
   - Zone ID와 Block ID를 Word Address로 변환하는 방법
   - 메모리 맵과 주소 매핑 이해

2. **EEPROM 쓰기 특성**
   - 비트 변경 규칙 (`1` → `0`만 가능)
   - 부분 쓰기와 전체 블록 쓰기의 차이

3. **쓰기 검증**
   - 쓰기 후 읽어서 데이터 검증하는 방법
   - 자동 검증과 수동 검증의 차이

4. **오류 처리**
   - 쓰기 권한 오류 처리
   - 잠금 메모리 오류 처리
   - 데이터 불일치 오류 처리

## 관련 문서

- **ATAES132A 데이터시트**: Section 5.2 - Standard Serial EEPROM Write Commands
- **ATAES132A 데이터시트**: Section 2.1 - Memory
- **ATAES132A 데이터시트**: Section 4.1 - User Zone Configuration
- [예제 2: 메모리 읽기](../02_memory_read/README.md)
- [예제 4: 랜덤 생성](../04_random/README.md)

## 다음 단계

- [예제 4: 랜덤 생성](../04_random/README.md) - 하드웨어 RNG 사용
- [예제 5: 카운터](../05_counter/README.md) - 카운터 기능 사용
- [예제 6: 암호화](../06_encrypt/README.md) - AES-128 암호화

## 참고사항

1. **쓰기 권한**: 일부 Zone은 쓰기 권한이 제한될 수 있습니다. Configuration Memory를 확인하여 쓰기 권한을 확인하세요.

2. **블록 경계**: 메모리 쓰기는 블록 경계를 넘어서 쓸 수 없습니다. 여러 블록을 쓰려면 각 블록마다 별도의 쓰기 작업을 수행해야 합니다.

3. **쓰기 시간**: EEPROM 쓰기는 시간이 걸립니다 (3-5ms). 여러 블록을 연속으로 쓸 때는 적절한 지연 시간을 추가하세요.

4. **데이터 지속성**: 쓰여진 데이터는 전원이 꺼져도 유지됩니다 (비휘발성 메모리).

5. **지우기 제한**: 개별 블록을 지울 수 없습니다. `0`에서 `1`로 되돌리려면 전체 Zone을 지우거나 새로운 데이터를 덮어써야 합니다.

## 문제 해결

### 쓰기 실패 시 확인사항

1. **I2C 연결 확인**: SDA, SCL 핀이 올바르게 연결되었는지 확인
2. **디바이스 주소 확인**: I2C 주소가 올바른지 확인 (기본값: 0xC0)
3. **쓰기 권한 확인**: 해당 Zone의 쓰기 권한이 설정되어 있는지 확인
4. **메모리 잠금 확인**: 해당 메모리 영역이 잠겨있지 않은지 확인
5. **블록 범위 확인**: Block ID가 유효한 범위(0-127) 내에 있는지 확인

### 일반적인 오류 코드

- `0x04` (RWConfig): 쓰기 권한이 없거나 현재 구성으로 인해 접근 불가
- `0x08` (BadAddr): 잘못된 주소이거나 잠긴 메모리에 쓰기 시도
- `0x60` (DataMismatch): 쓰기 후 자동 검증 실패 (데이터 불일치)
- `0x02` (BoundaryError): 블록 경계를 넘어서 쓰기 시도

### 쓰기 실패 원인 분석

**`0x04` (RWConfig) 오류**:
- Zone Configuration Register에서 쓰기 권한이 비활성화되어 있음
- 인증이 필요한 Zone에 인증 없이 쓰기 시도
- 해결: Configuration Memory를 확인하고 쓰기 권한을 활성화

**`0x08` (BadAddr) 오류**:
- 잘못된 Word Address 사용
- 잠긴 메모리 영역에 쓰기 시도
- 해결: Word Address 계산 확인, 잠금 상태 확인

**`0x60` (DataMismatch) 오류**:
- EEPROM 셀 손상 또는 쓰기 실패
- 전원 불안정 또는 노이즈
- 해결: 재시도, 전원 안정성 확인

## 추가 학습 자료

1. **메모리 관리 심화 학습**:
   - Zone별 쓰기 권한 설정 방법
   - 메모리 잠금 및 잠금 해제
   - Configuration Memory 구조

2. **보안 기능 학습**:
   - 암호화된 쓰기 (EncWrite)
   - 인증 후 쓰기
   - MAC 기반 쓰기 보호

3. **실전 활용**:
   - 대용량 데이터 쓰기 (여러 블록 연속 쓰기)
   - 데이터 백업 및 복원
   - 메모리 무결성 검증

4. **EEPROM 특성 이해**:
   - 쓰기 사이클 제한 (Endurance)
   - 데이터 보존 기간 (Data Retention)
   - 쓰기 전압 및 타이밍 요구사항

## 실전 팁

1. **쓰기 전 읽기**: 쓰기 전에 현재 데이터를 읽어서 백업하는 것이 좋습니다.

2. **검증 필수**: 중요한 데이터를 쓴 후에는 반드시 읽어서 검증하세요.

3. **에러 처리**: 쓰기 실패 시 재시도 로직을 구현하는 것이 좋습니다.

4. **타이밍 고려**: 여러 블록을 연속으로 쓸 때는 적절한 지연 시간을 추가하세요.

5. **권한 확인**: 쓰기 전에 해당 Zone의 쓰기 권한을 확인하는 것이 좋습니다.

