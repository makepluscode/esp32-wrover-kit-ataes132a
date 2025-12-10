# 예제 2: 메모리 읽기 (BlockRead)

## 목적

AES132 디바이스의 메모리 영역에서 데이터를 읽는 방법을 학습합니다. BlockRead 명령어를 사용하여 다양한 메모리 영역을 읽어오는 방법을 이해합니다.

## 설명

BlockRead 명령어는 AES132의 메모리 영역에서 데이터를 읽어오는 기본 명령어입니다. 이 명령어는 사용자 메모리 영역(User Memory)에서 데이터를 읽을 때 사용됩니다.

### 주요 개념

1. **메모리 구조**
   - AES132는 32KB의 EEPROM 메모리를 가지고 있습니다
   - 메모리는 16개의 사용자 영역(User Zone)으로 나뉩니다
   - 각 영역은 2KB (2048 바이트) 크기입니다
   - 각 영역은 8개의 블록(Block)으로 나뉩니다
   - 각 블록은 16바이트(128비트) 크기입니다

2. **주소 지정 방법**
   - Zone ID: 0-15 (16개 영역)
   - Block ID: 0-7 (각 영역당 8개 블록)
   - Byte Offset: 블록 내에서의 바이트 오프셋 (0-15)

3. **읽기 제한사항**
   - 한 번에 최대 16바이트까지 읽을 수 있습니다
   - 블록 경계를 넘어서 읽을 수 없습니다 (한 블록 내에서만 읽기 가능)
   - 읽기 권한이 있는 영역만 읽을 수 있습니다

## 데이터시트 참조

### BlockRead 명령어 (Section 7.4)

**명령어 코드**: `0x10`

**명령어 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 패킷의 총 바이트 수 (Count + Body + Checksum) |
| 1 | Op-Code | `0x10` (BlockRead) |
| 2 | Mode | 명령어 모드 (일반적으로 `0x00`) |
| 3-4 | Parameter 1 | Zone ID (상위 4비트) + Block ID (하위 4비트) |
| 5-6 | Parameter 2 | 읽을 바이트 수 (Length, 1-16) |
| 7-n | Data | 없음 (BlockRead는 입력 데이터 없음) |
| n+1-n+2 | Checksum | CRC-16 체크섬 |

**응답 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 응답 패킷의 총 바이트 수 |
| 1 | Status | 반환 코드 (Return Code) |
| 2-n | Data | 읽은 데이터 (최대 16바이트) |
| n+1-n+2 | Checksum | CRC-16 체크섬 |

**Parameter 1 구성**:
```
Bit 15-12: Zone ID (0-15)
Bit 11-8:  Reserved (0)
Bit 7-4:   Block ID (0-7)
Bit 3-0:   Reserved (0)
```

**Parameter 2 구성**:
```
Bit 15-8: Reserved (0)
Bit 7-0:  Length (읽을 바이트 수, 1-16)
```

### 반환 코드 (Return Codes)

| Code | Name | Description |
|------|------|-------------|
| `0x00` | Success | 명령어 성공적으로 실행됨 |
| `0x01` | CheckmacFail | MAC 검증 실패 |
| `0x03` | ParseError | 명령어 파싱 오류 |
| `0x0F` | ExecutionError | 명령어 실행 오류 |
| `0x11` | AfterWake | 디바이스가 Sleep 모드에서 깨어남 |
| `0xEE` | WatchdogTimeout | 워치독 타임아웃 |

### 메모리 맵 (Section 2.1)

ATAES132A의 메모리는 다음과 같이 구성됩니다:

```
Memory Map:
├── User Memory (32KB)
│   ├── Zone 0 (2KB, Blocks 0-127)
│   ├── Zone 1 (2KB, Blocks 0-127)
│   ├── ...
│   └── Zone 15 (2KB, Blocks 0-127)
├── Key Memory (16 keys, 128-bit each)
├── Configuration Memory
└── SRAM Memory
```

**각 Zone의 구조**:
- 각 Zone은 2KB (2048 바이트) = 128 블록
- 각 블록은 16바이트 (128비트)
- Block ID 범위: 0-127 (각 Zone당)

**참고**: 예제 코드에서는 간단하게 Block ID를 0-7로 사용하지만, 실제로는 각 Zone당 128개의 블록이 있습니다.

### I2C 통신 프로토콜

BlockRead 명령어는 표준 I2C EEPROM 읽기 명령어와 호환됩니다:

1. **명령어 전송**:
   - I2C Write: Slave Address + Word Address (2바이트) + Command Packet
   - Word Address는 I2C 버퍼 주소를 지정합니다

2. **응답 수신**:
   - I2C Read: Slave Address + Response Packet
   - Repeated Start 조건을 사용합니다

### 보안 고려사항

1. **읽기 권한**:
   - 각 Zone은 독립적으로 읽기 권한이 설정될 수 있습니다
   - Configuration Memory의 Zone Configuration Register에서 설정됩니다
   - 읽기 권한이 없는 Zone은 읽을 수 없습니다

2. **암호화된 읽기**:
   - 일반 BlockRead는 평문으로 읽습니다
   - 암호화된 읽기는 `EncRead` 명령어를 사용해야 합니다

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 2

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
Example 02: Memory Read (BlockRead)
========================================

AES132 initialized successfully

=== Example 1: Read Zone 0, Block 0 (16 bytes) ===
Successfully read 16 bytes:
Data: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

=== Example 2: Read Zone 0, Block 1 (8 bytes) ===
Successfully read 8 bytes:
Data: FF FF FF FF FF FF FF FF

=== Example 3: Read Multiple Blocks (Zone 0, Blocks 0-2) ===
Block 0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
Block 1: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
Block 2: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

=== Memory Read Examples Complete ===
```

## 출력 결과 상세 분석

### 1. 출력 값 `0xFF`의 의미

**`0xFF`는 EEPROM의 초기 상태(지워진 상태)입니다.**

- **`0xFF` (16진수) = `11111111` (2진수)**: 모든 비트가 1로 설정된 상태
- EEPROM 메모리에서 `0xFF`는 **"지워진(Erased)" 상태**를 의미합니다
- 데이터가 한 번도 쓰여지지 않았거나, 이전에 지워진 메모리 블록은 모두 `0xFF` 값을 가집니다

### 2. EEPROM 메모리의 특성

**EEPROM (Electrically Erasable Programmable Read-Only Memory)의 동작 원리:**

1. **초기 상태**: 제조 시 또는 지워진 후 모든 비트는 `0xFF` (모든 비트 = 1) 상태입니다
2. **쓰기 동작**: 특정 비트를 `0`으로 변경하여 데이터를 저장합니다
   - 예: `0xFF` → `0xFE` (최하위 비트를 0으로 변경)
   - 예: `0xFF` → `0x00` (모든 비트를 0으로 변경)
3. **지우기 동작**: 모든 비트를 다시 `1`로 되돌립니다 (`0xFF` 상태로 복귀)
4. **읽기 동작**: 현재 저장된 값을 읽어옵니다

**중요**: EEPROM은 비트를 `1`에서 `0`으로 변경하는 것은 쉽지만, `0`에서 `1`로 되돌리는 것은 **지우기(Erase) 작업**이 필요합니다.

### 3. 현재 출력 결과 해석

**모든 블록에서 `0xFF`를 읽는 것은 정상입니다:**

- ✅ **디바이스가 정상적으로 초기화되었습니다**
- ✅ **BlockRead 명령어가 정상적으로 실행되었습니다**
- ✅ **I2C 통신이 정상적으로 작동하고 있습니다**
- ✅ **메모리 영역에 접근할 수 있는 권한이 있습니다**

**이것은 다음을 의미합니다:**
1. AES132 디바이스가 제대로 연결되어 있고 통신이 정상입니다
2. 메모리 영역이 아직 사용되지 않았거나, 이전에 지워진 상태입니다
3. 예제 3 (메모리 쓰기)를 실행하면 실제 데이터를 쓸 수 있습니다

### 4. 메모리 상태 확인 방법

**다양한 메모리 상태를 확인하려면:**

1. **초기 상태 확인** (현재 상태):
   ```
   Block 0: FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
   ```
   → 모든 비트가 1 (지워진 상태)

2. **데이터 쓰기 후** (예제 3 실행 후):
   ```
   Block 0: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 FF FF
   ```
   → 일부 비트가 0으로 변경됨 (데이터가 쓰여짐)

3. **모든 비트를 0으로 설정한 후**:
   ```
   Block 0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
   ```
   → 모든 비트가 0 (데이터가 쓰여진 상태)

### 5. 왜 `0xFF`인가?

**ATAES132A의 메모리 초기화 과정:**

1. **제조 시**: 모든 EEPROM 셀은 `0xFF` 상태로 제조됩니다
2. **전원 인가 시**: 메모리 내용은 유지됩니다 (비휘발성)
3. **초기 사용 시**: 사용자가 데이터를 쓰기 전까지는 `0xFF` 상태를 유지합니다

**데이터시트 참조:**
- ATAES132A 데이터시트 Section 2.1 (Memory)에 따르면, User Memory는 초기 상태에서 모든 비트가 1 (`0xFF`)입니다
- 이는 표준 EEPROM 동작 방식과 일치합니다

### 6. 다음 단계

**이 출력 결과를 바탕으로 다음을 학습할 수 있습니다:**

1. ✅ **메모리 읽기 성공**: BlockRead 명령어가 정상 작동함을 확인
2. 🔜 **메모리 쓰기 학습**: 예제 3에서 실제 데이터를 써서 `0xFF`가 아닌 값을 확인
3. 🔜 **데이터 검증**: 쓰기 후 다시 읽어서 데이터가 올바르게 저장되었는지 확인

**실제 사용 예시:**
```cpp
// 현재 상태: 모든 값이 0xFF
read_memory_block(0, 0, 16, data);  // 결과: FF FF FF FF ...

// 데이터 쓰기 (예제 3에서 학습)
write_memory_block(0, 0, "Hello AES132!", 13);

// 다시 읽기
read_memory_block(0, 0, 16, data);  // 결과: 48 65 6C 6C 6F ...
```

### 7. 주의사항

**`0xFF`가 아닌 다른 값이 나오는 경우:**

- **이전에 데이터를 쓴 경우**: 메모리에 이전 데이터가 남아있을 수 있습니다
- **다른 예제를 실행한 경우**: 예제 3 (메모리 쓰기)를 실행했다면 실제 데이터가 보일 수 있습니다
- **디바이스가 재설정된 경우**: 전원이 꺼졌다 켜져도 EEPROM 데이터는 유지됩니다

**정리:**
- `0xFF` = 정상적인 초기 상태 ✅
- 다른 값 = 이전에 데이터가 쓰여진 상태 ✅
- 둘 다 정상적인 동작입니다!

## 코드 설명

### 주요 함수

1. **`read_memory_block()`**
   - Zone ID와 Block ID를 사용하여 메모리 블록을 읽습니다
   - Parameter 1: Zone ID (상위 4비트) + Block ID (하위 4비트)
   - Parameter 2: 읽을 바이트 수 (1-16)
   - 성공 시 읽은 바이트 수를 반환합니다

2. **`aes132m_execute()`**
   - AES132 명령어를 실행하는 마샬링 함수입니다
   - 명령어 패킷을 구성하고 전송한 후 응답을 받습니다

### 예제 시나리오

1. **예제 1**: Zone 0, Block 0에서 전체 16바이트 읽기
2. **예제 2**: Zone 0, Block 1에서 8바이트 읽기
3. **예제 3**: 여러 블록을 연속으로 읽기

## 학습 포인트

1. **메모리 주소 지정**
   - Zone ID와 Block ID를 조합하여 메모리 위치를 지정하는 방법
   - Parameter 1의 비트 구성 이해

2. **명령어 패킷 구조**
   - BlockRead 명령어의 패킷 구조 이해
   - Op-Code, Mode, Parameter의 역할

3. **응답 처리**
   - 응답 패킷에서 데이터 추출 방법
   - 반환 코드 확인 및 오류 처리

4. **I2C 통신**
   - AES132와의 I2C 통신 프로토콜 이해
   - Repeated Start 조건의 사용

## 관련 문서

- **ATAES132A 데이터시트**: Section 7.4 - BlockRead Command
- **ATAES132A 데이터시트**: Section 2.1 - Memory
- **ATAES132A 데이터시트**: Section 5.1 - Standard Serial EEPROM Read Commands
- [예제 1: Info 명령어](../01_info/README.md)
- [예제 3: 메모리 쓰기](../03_memory_write/README.md)

## 다음 단계

- [예제 3: 메모리 쓰기](../03_memory_write/README.md) - 메모리에 데이터를 쓰는 방법 학습
- [예제 4: 랜덤 생성](../04_random/README.md) - 하드웨어 RNG 사용
- [예제 5: 카운터](../05_counter/README.md) - 카운터 기능 사용

## 참고사항

1. **초기화된 메모리**: 새로 초기화되거나 지워진 AES132 디바이스의 메모리는 모두 `0xFF` (모든 비트가 1)로 채워져 있습니다. 이것은 EEPROM의 표준 초기 상태입니다. `0xFF`는 "지워진" 상태를 의미하며, 데이터가 쓰여지지 않은 상태를 나타냅니다.

2. **읽기 권한**: 일부 Zone은 읽기 권한이 제한될 수 있습니다. Configuration Memory를 확인하여 읽기 권한을 확인하세요.

3. **블록 경계**: BlockRead는 블록 경계를 넘어서 읽을 수 없습니다. 여러 블록을 읽으려면 각 블록마다 별도의 명령어를 실행해야 합니다.

4. **성능**: BlockRead는 비교적 빠른 명령어입니다. 하지만 여러 블록을 읽을 때는 각 블록마다 명령어를 실행해야 하므로 시간이 걸릴 수 있습니다.

## 문제 해결

### 읽기 실패 시 확인사항

1. **I2C 연결 확인**: SDA, SCL 핀이 올바르게 연결되었는지 확인
2. **디바이스 주소 확인**: I2C 주소가 올바른지 확인 (기본값: 0xC0)
3. **읽기 권한 확인**: 해당 Zone의 읽기 권한이 설정되어 있는지 확인
4. **블록 범위 확인**: Block ID가 유효한 범위(0-127) 내에 있는지 확인

### 일반적인 오류 코드

- `0x03` (ParseError): 명령어 패킷 형식 오류
- `0x0F` (ExecutionError): 명령어 실행 오류 (읽기 권한 없음 등)
- `0xEE` (WatchdogTimeout): 통신 타임아웃

## 추가 학습 자료

1. **메모리 구조 심화 학습**:
   - User Memory의 세부 구조
   - Key Memory의 구조와 사용법
   - Configuration Memory의 역할

2. **보안 기능 학습**:
   - Zone별 읽기/쓰기 권한 설정
   - 암호화된 읽기 (EncRead)
   - 인증 후 읽기

3. **실전 활용**:
   - 대용량 데이터 읽기 (여러 블록 연속 읽기)
   - 메모리 덤프 유틸리티 작성
   - 메모리 검증 및 무결성 확인

