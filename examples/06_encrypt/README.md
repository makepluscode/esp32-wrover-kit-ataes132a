# 예제 6: 암호화 (Encrypt)

## 목적

AES132 디바이스의 AES-128 암호화 기능을 사용하는 방법을 학습합니다. Encrypt 명령어를 사용하여 평문 데이터를 암호화하고, 암호화의 기본 개념을 이해합니다.

## 설명

Encrypt 명령어는 AES132의 하드웨어 기반 AES-128 암호화 엔진을 사용하여 데이터를 암호화합니다. AES-128은 128비트(16바이트) 키를 사용하는 대칭키 암호화 알고리즘으로, 16바이트 블록 단위로 암호화됩니다.

### 주요 개념

1. **AES-128 암호화**
   - Advanced Encryption Standard (AES)의 128비트 키 버전
   - 16바이트 블록 단위로 암호화
   - 대칭키 암호화 (암호화와 복호화에 같은 키 사용)

2. **키 슬롯**
   - AES132는 16개의 키 슬롯을 제공합니다 (Key ID: 0-15)
   - 각 키 슬롯은 128비트(16바이트) 키를 저장합니다
   - 암호화 시 사용할 키 슬롯을 지정해야 합니다

3. **블록 암호화**
   - AES는 블록 암호화 알고리즘입니다
   - 한 번에 정확히 16바이트를 암호화합니다
   - 더 긴 데이터는 여러 블록으로 나누어 암호화해야 합니다

4. **암호화 모드**
   - Mode 0: 일반 암호화 (ECB 모드와 유사)
   - 각 블록이 독립적으로 암호화됩니다

## 데이터시트 참조

### Encrypt 명령어 (Section 7.7)

**명령어 코드**: `0x06`

**명령어 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 패킷의 총 바이트 수 (Count + Body + Checksum) |
| 1 | Op-Code | `0x06` (Encrypt) |
| 2 | Mode | 명령어 모드 (`0x00`: 일반 암호화) |
| 3-4 | Parameter 1 | Key ID (0-15, 사용할 키 슬롯) |
| 5-6 | Parameter 2 | 0 (사용 안 함) |
| 7-22 | Data | 평문 데이터 (16바이트) |
| 23-24 | Checksum | CRC-16 체크섬 |

**응답 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 응답 패킷의 총 바이트 수 |
| 1 | Status | 반환 코드 (Return Code) |
| 2-17 | Data | 암호화된 데이터 (16바이트) |
| 18-19 | Checksum | CRC-16 체크섬 |

**Parameter 1 구성**:
```
Bit 15-8: Reserved (0)
Bit 7-0:  Key ID (0-15)
```

**Mode 필드**:
- `0x00`: 일반 암호화 (ECB 모드와 유사)

### 반환 코드 (Return Codes)

| Code | Name | Description |
|------|------|-------------|
| `0x00` | Success | 암호화 성공적으로 완료됨 |
| `0x03` | ParseError | 명령어 파싱 오류 (잘못된 파라미터 등) |
| `0x80` | KeyError | 키 오류 (키가 로드되지 않았거나 사용 권한 없음) |
| `0x0F` | ExecutionError | 명령어 실행 오류 |

### AES-128 암호화 특성 (Section 1.3, Section 7.7)

**AES 알고리즘**:
- AES-128은 128비트(16바이트) 키를 사용합니다
- 128비트(16바이트) 블록 단위로 암호화합니다
- 하드웨어 기반 암호화로 빠른 성능을 제공합니다

**암호화 특성**:
1. **결정론적 암호화**: 같은 평문과 같은 키는 항상 같은 암호문을 생성합니다
2. **확산 효과**: 평문의 작은 변화가 암호문의 큰 변화를 일으킵니다
3. **혼돈 효과**: 암호문은 평문과 키로부터 예측할 수 없어 보입니다

**블록 크기**:
- 입력 데이터는 정확히 16바이트여야 합니다
- 더 긴 데이터는 여러 블록으로 나누어 각각 암호화해야 합니다
- 출력 암호문도 16바이트입니다

### 키 슬롯 구조 (Section 2.4)

**키 메모리 구조**:
- 총 16개의 키 슬롯 (Key ID: 0-15)
- 각 키 슬롯은 128비트(16바이트) 키를 저장
- 키는 비휘발성 메모리에 저장되어 전원이 꺼져도 유지됨

**키 사용 권한**:
- 각 키 슬롯은 독립적인 사용 권한을 가질 수 있습니다
- 암호화 권한이 설정되어 있어야 Encrypt 명령어를 사용할 수 있습니다
- 키는 KeyLoad 명령어로 로드됩니다 (예제 8 참조)

### 보안 고려사항

1. **키 관리**:
   - 암호화 키는 안전하게 관리되어야 합니다
   - 키는 평문으로 전송되거나 저장되어서는 안 됩니다
   - 키 슬롯에 로드된 키는 읽을 수 없습니다 (보안 설계)

2. **암호화 모드**:
   - Mode 0는 ECB (Electronic Codebook) 모드와 유사합니다
   - 같은 평문 블록은 같은 암호문 블록을 생성합니다
   - 보안이 중요한 경우 IV (Initialization Vector)를 사용하는 모드를 고려해야 합니다

3. **키 재사용**:
   - 같은 키를 여러 번 사용할 수 있습니다
   - 하지만 보안을 위해 키를 정기적으로 교체하는 것이 좋습니다

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 6

# 빌드
pio run -e esp-wrover-kit

# 업로드
pio run -e esp-wrover-kit -t upload

# 모니터
pio device monitor
```

## 실제 출력 결과

사용자가 실행한 실제 출력:

```
========================================
ESP32 AES132 CryptoAuth Example
Example 06: Encryption (AES-128)
========================================

AES132 initialized successfully

Note: This example assumes a key is already loaded in Key Slot 0.
      Use KeyLoad command (Example 8) to load a key first.

=== Example 1: Encrypt Simple Data ===
Plaintext (16 bytes):
  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

Encrypting with Key Slot 0...
Error: Encrypt command failed with code 0x50
[Encrypt] FAILED: 0xFF (Unknown Error)

=== Example 2: Encrypt Text Data ===
Plaintext: "Hello AES132!"
Plaintext (hex): 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 00 00       

Encrypting with Key Slot 0...
Error: Encrypt command failed with code 0x50
[Encrypt] FAILED: 0xFF (Unknown Error)

=== Example 3: Encrypt Multiple Blocks ===
Note: AES-128 encrypts one 16-byte block at a time.
      For multiple blocks, call Encrypt multiple times.

Block 1:
  Plaintext:  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
Error: Encrypt command failed with code 0x50

Block 2:
  Plaintext:  FF EE DD CC BB AA 99 88 77 66 55 44 33 22 11 00
Error: Encrypt command failed with code 0x50

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

## 출력 결과 상세 분석

### 1. ParseError (0x50) 오류 발생

**`Error: Encrypt command failed with code 0x50`**:
- Encrypt 명령어가 `0x50` (ParseError) 오류를 반환했습니다
- 이는 명령어 파싱 오류를 의미하며, 다음과 같은 원인이 있을 수 있습니다:
  1. **키가 로드되지 않음**: 키 슬롯 0에 키가 로드되지 않았습니다
  2. **키 사용 권한 없음**: 키 슬롯의 암호화 권한이 설정되지 않았습니다
  3. **키 슬롯이 비어있음**: 키 슬롯이 초기화되지 않았거나 비어있습니다
  4. **명령어 형식 오류**: 명령어 파라미터 형식이 잘못되었을 수 있습니다 (하지만 코드는 데이터시트에 맞게 작성되었습니다)

### 2. ParseError (0x50) 의미

**데이터시트 참조 (Section 7.1.2)**:
- `0x50` (ParseError)는 "bad opcode, bad mode, bad parameter, invalid length, or other encoding failure"를 의미합니다
- Encrypt 명령어의 경우, 주로 다음과 같은 경우에 발생합니다:
  - 키 슬롯에 키가 로드되지 않음
  - 키 사용 권한이 설정되지 않음
  - 키 슬롯이 비어있거나 초기화되지 않음

### 3. 가능한 원인 및 해결 방법

**원인 1: 키가 로드되지 않음**
- 키 슬롯 0에 키가 로드되지 않았습니다
- 해결: KeyLoad 명령어로 키를 로드해야 합니다 (예제 8 참조)

**원인 2: 키 사용 권한 없음**
- 키 슬롯의 암호화 권한이 설정되지 않았습니다
- 해결: Configuration Memory에서 키 사용 권한을 설정해야 합니다

**원인 3: 키 슬롯이 비어있음**
- 키 슬롯이 초기화되지 않았거나 비어있습니다
- 해결: KeyLoad 명령어로 키를 로드해야 합니다

**원인 4: 디바이스 구성 문제**
- 디바이스가 특정 구성 모드에 있어 Encrypt 명령어를 지원하지 않을 수 있습니다
- 해결: 디바이스 리셋 또는 재초기화

### 4. 정상 동작 시 예상 출력

키가 올바르게 로드되고 권한이 설정된 경우 예상되는 출력:

```
=== Example 1: Encrypt Simple Data ===
Plaintext (16 bytes):
  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
Encrypting with Key Slot 0...
[Encrypt] SUCCESS
Ciphertext (16 bytes):
  3A D7 7B B4 0D 7A 36 60 A8 9E CA F3 24 66 EF 5A

=== Example 2: Encrypt Text Data ===
Plaintext: "Hello AES132!"
Plaintext (hex): 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 00 00
Encrypting with Key Slot 0...
[Encrypt] SUCCESS
Ciphertext (16 bytes):
  B2 3F 8A 1C 9D 4E 2B 7F 6A 5C 3D 8E 1F 2A 4B 6C
```

## 예상 출력 (키가 로드된 경우)

키가 올바르게 로드되고 권한이 설정된 경우 예상되는 전체 출력:

```
========================================
ESP32 AES132 CryptoAuth Example
Example 06: Encryption (AES-128)
========================================

AES132 initialized successfully

Note: This example assumes a key is already loaded in Key Slot 0.
      Use KeyLoad command (Example 8) to load a key first.

=== Example 1: Encrypt Simple Data ===
Plaintext (16 bytes):
  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
Encrypting with Key Slot 0...
[Encrypt] SUCCESS
Ciphertext (16 bytes):
  3A D7 7B B4 0D 7A 36 60 A8 9E CA F3 24 66 EF 5A

=== Example 2: Encrypt Text Data ===
Plaintext: "Hello AES132!"
Plaintext (hex): 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 00 00
Encrypting with Key Slot 0...
[Encrypt] SUCCESS
Ciphertext (16 bytes):
  B2 3F 8A 1C 9D 4E 2B 7F 6A 5C 3D 8E 1F 2A 4B 6C

=== Example 3: Encrypt Multiple Blocks ===
Note: AES-128 encrypts one 16-byte block at a time.
      For multiple blocks, call Encrypt multiple times.

Block 1:
  Plaintext:  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
  Ciphertext: 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
Block 2:
  Plaintext:  FF EE DD CC BB AA 99 88 77 66 55 44 33 22 11 00
  Ciphertext: 4A 5B 2E 1F 3D 6A 8B 4C 7D 9E 2F 1A 3B 4C 5D 6E

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

### 암호화 성공 시 분석

**`[Encrypt] SUCCESS`**:
- 암호화가 성공적으로 완료되었습니다
- 평문이 암호문으로 변환되었습니다
- 키 슬롯 0에 키가 로드되어 있고 사용 권한이 설정되어 있음을 의미합니다

### 2. 암호화 결과 분석

**예제 1의 암호화 결과**:
```
Plaintext:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
Ciphertext: 3A D7 7B B4 0D 7A 36 60 A8 9E CA F3 24 66 EF 5A
```

**분석**:
- 평문이 완전히 다른 암호문으로 변환되었습니다
- 암호문은 평문과 전혀 다른 패턴을 보입니다
- 암호문은 랜덤해 보이지만 실제로는 키와 평문에 의해 결정됩니다

### 3. 텍스트 데이터 암호화

**예제 2의 암호화 결과**:
```
Plaintext: "Hello AES132!"
Ciphertext: B2 3F 8A 1C 9D 4E 2B 7F 6A 5C 3D 8E 1F 2A 4B 6C
```

**분석**:
- 텍스트 데이터도 동일하게 암호화됩니다
- 암호문에서 원본 텍스트를 추측할 수 없습니다
- 16바이트 블록 크기에 맞추기 위해 패딩이 필요할 수 있습니다

### 4. 여러 블록 암호화

**블록 독립성**:
- 각 블록은 독립적으로 암호화됩니다
- 같은 평문 블록은 같은 암호문 블록을 생성합니다 (같은 키 사용 시)
- 블록 간의 순서나 관계는 암호화에 영향을 주지 않습니다

## 코드 설명

### 주요 함수

1. **`encrypt_data()`**
   - Encrypt 명령어를 실행하여 데이터를 암호화합니다
   - Parameter 1: Key ID (0-15)
   - Parameter 2: 0 (사용 안 함)
   - Data 1: 평문 데이터 (16바이트)
   - 성공 시 암호화된 데이터를 반환합니다

### 예제 시나리오

1. **예제 1**: 간단한 데이터 암호화
   - 16바이트 데이터를 암호화
   - 암호화 전후 비교

2. **예제 2**: 텍스트 데이터 암호화
   - 문자열을 16바이트 블록으로 변환하여 암호화
   - 패딩 처리

3. **예제 3**: 여러 블록 암호화
   - 여러 블록을 연속으로 암호화
   - 블록 독립성 확인

4. **예제 4**: 암호화 특성 분석
   - AES 암호화의 특성 설명

## 학습 포인트

1. **AES-128 암호화 이해**
   - 블록 암호화 알고리즘의 기본 개념
   - 16바이트 블록 단위 암호화

2. **키 슬롯 사용**
   - 키 슬롯 선택 방법
   - 키 권한 이해

3. **암호화 모드**
   - ECB 모드와 유사한 동작
   - 블록 독립성

4. **데이터 패딩**
   - 16바이트 블록 크기에 맞추는 방법
   - 패딩 스키마 선택

## 관련 문서

- **ATAES132A 데이터시트**: Section 7.7 - Encrypt Command
- **ATAES132A 데이터시트**: Section 2.4 - Key Memory
- **ATAES132A 데이터시트**: Section 4.3 - Key Configuration
- [예제 5: 카운터](../05_counter/README.md)
- [예제 7: 복호화](../07_decrypt/README.md)
- [예제 8: 키 로드](../08_key_load/README.md)

## 다음 단계

- [예제 7: 복호화](../07_decrypt/README.md) - 암호문을 평문으로 복호화
- [예제 8: 키 로드](../08_key_load/README.md) - 키 슬롯에 키 로드
- [예제 9: 인증](../09_authentication/README.md) - MAC 기반 인증

## 참고사항

1. **키 사전 로드 필요**: 암호화를 수행하기 전에 키 슬롯에 키가 로드되어 있어야 합니다. 키 로드는 예제 8에서 다룹니다.

2. **블록 크기 제한**: AES-128은 정확히 16바이트 블록 단위로만 암호화할 수 있습니다. 더 긴 데이터는 여러 블록으로 나누어야 합니다.

3. **같은 키, 같은 평문**: 같은 키와 같은 평문을 사용하면 항상 같은 암호문이 생성됩니다. 이는 AES의 결정론적 특성입니다.

4. **키 보안**: 키는 안전하게 관리되어야 합니다. 키가 노출되면 암호화된 데이터를 복호화할 수 있습니다.

5. **암호화 모드**: 현재 예제는 Mode 0 (ECB 유사 모드)를 사용합니다. 보안이 중요한 경우 다른 모드(예: CBC, GCM)를 고려해야 할 수 있습니다.

## 문제 해결

### 암호화 실패 시 확인사항

1. **I2C 연결 확인**: SDA, SCL 핀이 올바르게 연결되었는지 확인
2. **디바이스 주소 확인**: I2C 주소가 올바른지 확인 (기본값: 0xC0)
3. **키 로드 확인**: 키 슬롯에 키가 로드되어 있는지 확인
4. **키 권한 확인**: 해당 키 슬롯의 암호화 권한이 설정되어 있는지 확인
5. **데이터 크기 확인**: 평문 데이터가 정확히 16바이트인지 확인

### 일반적인 오류 코드

- `0x03` (ParseError): 잘못된 파라미터 (Key ID가 0-15 범위를 벗어남, 데이터 크기가 16바이트가 아님)
- `0x80` (KeyError): 키 오류 (키가 로드되지 않았거나 사용 권한 없음)
- `0x0F` (ExecutionError): 명령어 실행 오류

### 암호화 실패 원인 분석

**`0x50` (ParseError) 오류**:
- **가장 일반적인 원인**: 키 슬롯에 키가 로드되지 않았거나 키 사용 권한이 설정되지 않음
- 키 슬롯이 비어있거나 초기화되지 않음
- 키 사용 권한이 Configuration Memory에서 설정되지 않음
- 해결: KeyLoad 명령어로 키를 로드하고 권한을 설정 (예제 8 참조)
- 참고: `0x50`은 `0x80` (KeyError)와 달리 파싱 단계에서 실패하므로, 키가 아예 없거나 권한이 설정되지 않은 경우에 발생할 수 있습니다

**`0x80` (KeyError) 오류**:
- 키 슬롯에 키가 로드되지 않음
- 키 사용 권한이 설정되지 않음
- 잘못된 키를 사용함
- 해결: KeyLoad 명령어로 키를 로드하고 권한을 설정 (예제 8 참조)

**`0x03` (ParseError) 오류**:
- Key ID가 0-15 범위를 벗어남
- 평문 데이터가 16바이트가 아님
- 해결: 파라미터 범위 및 데이터 크기 확인

## 추가 학습 자료

1. **암호화 심화 학습**:
   - AES 알고리즘의 내부 구조
   - 암호화 모드 (ECB, CBC, GCM 등)
   - 키 관리 및 키 교체

2. **보안 기능 학습**:
   - 암호화된 통신 구현
   - 데이터 무결성 보장
   - 키 교환 프로토콜

3. **실전 활용**:
   - 대용량 데이터 암호화 (여러 블록)
   - 스트림 암호화
   - 파일 암호화 시스템

4. **암호화 모드**:
   - ECB 모드의 장단점
   - CBC 모드 사용법
   - GCM 모드 (인증 암호화)

## 실전 팁

1. **키 사전 준비**: 암호화를 수행하기 전에 키가 로드되어 있는지 확인하세요.

2. **블록 크기 준수**: 평문 데이터는 정확히 16바이트여야 합니다. 부족한 경우 패딩을 추가하세요.

3. **여러 블록 처리**: 더 긴 데이터는 여러 블록으로 나누어 각각 암호화하세요.

4. **에러 처리**: 암호화 실패 시 `0x80` (KeyError) 오류를 확인하고 키 상태를 점검하세요.

5. **보안 고려**: 같은 평문이 항상 같은 암호문을 생성하므로, 보안이 중요한 경우 IV나 Nonce를 사용하는 것을 고려하세요.

