# 예제 7: 복호화 (Decrypt)

## 목적

AES132 디바이스의 AES-128 복호화 기능을 사용하는 방법을 학습합니다. Decrypt 명령어를 사용하여 암호문을 평문으로 복호화하고, 암호화-복호화 전체 사이클을 이해합니다.

## 설명

Decrypt 명령어는 AES132의 하드웨어 기반 AES-128 복호화 엔진을 사용하여 암호문을 평문으로 복호화합니다. 복호화는 암호화의 역연산으로, 암호화에 사용한 것과 동일한 키를 사용해야 합니다.

### 주요 개념

1. **AES-128 복호화**
   - AES-128 암호화의 역연산
   - 16바이트 블록 단위로 복호화
   - 암호화에 사용한 것과 동일한 키 필요

2. **키 일치 필요**
   - 복호화에는 암호화에 사용한 것과 동일한 키가 필요합니다
   - 다른 키를 사용하면 올바른 평문을 얻을 수 없습니다
   - 키 슬롯 ID도 동일해야 합니다

3. **블록 복호화**
   - AES는 블록 복호화 알고리즘입니다
   - 한 번에 정확히 16바이트를 복호화합니다
   - 더 긴 데이터는 여러 블록으로 나누어 복호화해야 합니다

4. **복호화 검증**
   - 복호화된 데이터가 원본 평문과 일치하는지 확인해야 합니다
   - 일치하지 않으면 키 오류 또는 데이터 손상 가능성

## 데이터시트 참조

### Decrypt 명령어 (Section 7.8)

**명령어 코드**: `0x07`

**명령어 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 패킷의 총 바이트 수 (Count + Body + Checksum) |
| 1 | Op-Code | `0x07` (Decrypt) |
| 2 | Mode | 명령어 모드 (`0x00`: 일반 복호화) |
| 3-4 | Parameter 1 | Key ID (0-15, 암호화에 사용한 것과 동일) |
| 5-6 | Parameter 2 | 0 (사용 안 함) |
| 7-22 | Data | 암호문 데이터 (16바이트) |
| 23-24 | Checksum | CRC-16 체크섬 |

**응답 패킷 구조**:

| Byte | Name | Description |
|------|------|-------------|
| 0 | Count | 응답 패킷의 총 바이트 수 |
| 1 | Status | 반환 코드 (Return Code) |
| 2-17 | Data | 복호화된 평문 데이터 (16바이트) |
| 18-19 | Checksum | CRC-16 체크섬 |

**Parameter 1 구성**:
```
Bit 15-8: Reserved (0)
Bit 7-0:  Key ID (0-15, 암호화에 사용한 것과 동일)
```

**Mode 필드**:
- `0x00`: 일반 복호화 (ECB 모드와 유사)

### 반환 코드 (Return Codes)

| Code | Name | Description |
|------|------|-------------|
| `0x00` | Success | 복호화 성공적으로 완료됨 |
| `0x03` | ParseError | 명령어 파싱 오류 (잘못된 파라미터 등) |
| `0x80` | KeyError | 키 오류 (키가 로드되지 않았거나 사용 권한 없음, 또는 잘못된 키) |
| `0x0F` | ExecutionError | 명령어 실행 오류 |

### AES-128 복호화 특성 (Section 1.3, Section 7.8)

**복호화 알고리즘**:
- AES-128 복호화는 암호화의 정확한 역연산입니다
- 암호화에 사용한 것과 동일한 키를 사용해야 합니다
- 하드웨어 기반 복호화로 빠른 성능을 제공합니다

**복호화 특성**:
1. **역연산**: 복호화는 암호화의 정확한 역연산입니다
2. **키 일치 필요**: 암호화에 사용한 것과 동일한 키가 필요합니다
3. **완전 복원**: 올바른 키를 사용하면 원본 평문이 정확히 복원됩니다

**블록 크기**:
- 입력 암호문은 정확히 16바이트여야 합니다
- 더 긴 데이터는 여러 블록으로 나누어 각각 복호화해야 합니다
- 출력 평문도 16바이트입니다

### 키 일치 요구사항

**동일 키 필요**:
- 복호화에는 암호화에 사용한 것과 동일한 키가 필요합니다
- 키 슬롯 ID도 동일해야 합니다
- 다른 키를 사용하면 올바른 평문을 얻을 수 없습니다

**키 오류 감지**:
- 잘못된 키를 사용하면 복호화는 성공하지만 의미 없는 데이터가 생성됩니다
- 복호화된 데이터를 검증하여 키가 올바른지 확인해야 합니다

### 보안 고려사항

1. **키 보안**:
   - 복호화 키는 암호화 키와 동일하게 안전하게 관리되어야 합니다
   - 키가 노출되면 암호화된 데이터를 복호화할 수 있습니다

2. **데이터 무결성**:
   - 암호문이 손상되면 복호화 결과가 올바르지 않을 수 있습니다
   - 데이터 무결성을 보장하기 위해 MAC이나 체크섬을 사용하는 것이 좋습니다

3. **키 관리**:
   - 암호화와 복호화에 사용하는 키를 일관되게 관리해야 합니다
   - 키 교체 시 기존 암호문을 복호화할 수 없게 됩니다

## 사용 방법

```bash
# 예제 선택
.\scripts\select_example.ps1 7

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
Example 07: Decryption (AES-128)
========================================

AES132 initialized successfully

Note: This example assumes a key is already loaded in Key Slot 0.
      Use KeyLoad command (Example 8) to load a key first.

=== Example 1: Decrypt Ciphertext ===
Ciphertext (16 bytes):
  3A D7 7B B4 0D 7A 36 60 A8 9E CA F3 24 66 EF 5A
Decrypting with Key Slot 0...
[Decrypt] SUCCESS
Plaintext (16 bytes):
  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F

=== Example 2: Encrypt-Decrypt Cycle ===
Step 1: Encrypt plaintext
Original plaintext: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 00 00
Encrypted ciphertext: B2 3F 8A 1C 9D 4E 2B 7F 6A 5C 3D 8E 1F 2A 4B 6C

Step 2: Decrypt ciphertext
Ciphertext to decrypt: B2 3F 8A 1C 9D 4E 2B 7F 6A 5C 3D 8E 1F 2A 4B 6C
[Decrypt] SUCCESS
Decrypted plaintext: 48 65 6C 6C 6F 20 41 45 53 31 33 32 21 00 00 00

Step 3: Verify decryption
✓ Verification successful! Decrypted data matches original.

=== Example 3: Decrypt Multiple Blocks ===
Note: AES-128 decrypts one 16-byte block at a time.
      For multiple blocks, call Decrypt multiple times.

Block 1:
  Ciphertext: 2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C
  Plaintext:  00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
Block 2:
  Ciphertext: 4A 5B 2E 1F 3D 6A 8B 4C 7D 9E 2F 1A 3B 4C 5D 6E
  Plaintext:  FF EE DD CC BB AA 99 88 77 66 55 44 33 22 11 00

=== Example 4: Decryption Analysis ===
Characteristics of AES decryption:
1. Decryption is the inverse operation of encryption
2. Same ciphertext and key always produce same plaintext
3. Decryption restores the original data exactly
4. Wrong key produces garbage data (not original plaintext)

=== Decryption Examples Complete ===

Note: Decryption requires the same key used for encryption.
      Key must be loaded in the same key slot.
```

## 출력 결과 상세 분석

### 1. 복호화 성공 확인

**`[Decrypt] SUCCESS`**:
- 복호화가 성공적으로 완료되었습니다
- 암호문이 평문으로 변환되었습니다
- 키 슬롯 0에 올바른 키가 로드되어 있음을 의미합니다

### 2. 복호화 결과 분석

**예제 1의 복호화 결과**:
```
Ciphertext: 3A D7 7B B4 0D 7A 36 60 A8 9E CA F3 24 66 EF 5A
Plaintext:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
```

**분석**:
- 암호문이 원본 평문으로 정확히 복원되었습니다
- 복호화가 암호화의 정확한 역연산임을 확인할 수 있습니다
- 올바른 키를 사용했을 때 완벽한 복원이 가능합니다

### 3. 암호화-복호화 전체 사이클

**예제 2의 전체 사이클**:
```
Original → Encrypt → Ciphertext → Decrypt → Decrypted
```

**검증 결과**:
- `✓ Verification successful!`: 복호화된 데이터가 원본과 정확히 일치합니다
- 암호화와 복호화가 올바르게 동작하고 있음을 확인할 수 있습니다
- 키가 올바르게 설정되어 있음을 의미합니다

### 4. 여러 블록 복호화

**블록 독립성**:
- 각 블록은 독립적으로 복호화됩니다
- 블록 순서는 복호화 결과에 영향을 주지 않습니다
- 각 블록을 개별적으로 복호화할 수 있습니다

## 코드 설명

### 주요 함수

1. **`decrypt_data()`**
   - Decrypt 명령어를 실행하여 데이터를 복호화합니다
   - Parameter 1: Key ID (0-15, 암호화에 사용한 것과 동일)
   - Parameter 2: 0 (사용 안 함)
   - Data 1: 암호문 데이터 (16바이트)
   - 성공 시 복호화된 평문을 반환합니다

### 예제 시나리오

1. **예제 1**: 암호문 복호화
   - 암호문을 평문으로 복호화
   - 복호화 결과 확인

2. **예제 2**: 암호화-복호화 전체 사이클
   - 평문을 암호화한 후 다시 복호화
   - 원본과 복호화된 데이터 비교 및 검증

3. **예제 3**: 여러 블록 복호화
   - 여러 블록을 연속으로 복호화
   - 블록 독립성 확인

4. **예제 4**: 복호화 특성 분석
   - AES 복호화의 특성 설명

## 학습 포인트

1. **AES-128 복호화 이해**
   - 복호화가 암호화의 역연산임을 이해
   - 블록 복호화 알고리즘의 동작

2. **키 일치 필요성**
   - 복호화에 동일한 키가 필요함을 이해
   - 키 불일치 시 결과 분석

3. **복호화 검증**
   - 복호화된 데이터가 원본과 일치하는지 확인하는 방법
   - 데이터 무결성 검증

4. **암호화-복호화 사이클**
   - 전체 암호화-복호화 프로세스 이해
   - 데이터 보호 및 복원 과정

## 관련 문서

- **ATAES132A 데이터시트**: Section 7.8 - Decrypt Command
- **ATAES132A 데이터시트**: Section 7.7 - Encrypt Command
- **ATAES132A 데이터시트**: Section 2.4 - Key Memory
- [예제 6: 암호화](../06_encrypt/README.md) - 평문을 암호문으로 암호화
- [예제 8: 키 로드](../08_key_load/README.md) - 키 슬롯에 키 로드

## 다음 단계

- [예제 8: 키 로드](../08_key_load/README.md) - 키 슬롯에 키 로드
- [예제 9: 인증](../09_authentication/README.md) - MAC 기반 인증
- [예제 10: 키 생성](../10_key_create/README.md) - 새로운 키 생성

## 참고사항

1. **키 사전 로드 필요**: 복호화를 수행하기 전에 키 슬롯에 키가 로드되어 있어야 합니다. 키 로드는 예제 8에서 다룹니다.

2. **동일 키 필요**: 복호화에는 암호화에 사용한 것과 동일한 키가 필요합니다. 다른 키를 사용하면 올바른 평문을 얻을 수 없습니다.

3. **블록 크기 제한**: AES-128은 정확히 16바이트 블록 단위로만 복호화할 수 있습니다. 더 긴 데이터는 여러 블록으로 나누어야 합니다.

4. **복호화 검증**: 복호화된 데이터가 원본과 일치하는지 항상 확인하세요. 일치하지 않으면 키 오류 또는 데이터 손상 가능성이 있습니다.

5. **키 슬롯 일치**: 암호화에 사용한 키 슬롯 ID와 동일한 키 슬롯을 사용해야 합니다.

## 문제 해결

### 복호화 실패 시 확인사항

1. **I2C 연결 확인**: SDA, SCL 핀이 올바르게 연결되었는지 확인
2. **디바이스 주소 확인**: I2C 주소가 올바른지 확인 (기본값: 0xC0)
3. **키 로드 확인**: 키 슬롯에 키가 로드되어 있는지 확인
4. **키 일치 확인**: 암호화에 사용한 것과 동일한 키인지 확인
5. **키 권한 확인**: 해당 키 슬롯의 복호화 권한이 설정되어 있는지 확인
6. **데이터 크기 확인**: 암호문 데이터가 정확히 16바이트인지 확인

### 일반적인 오류 코드

- `0x03` (ParseError): 잘못된 파라미터 (Key ID가 0-15 범위를 벗어남, 데이터 크기가 16바이트가 아님)
- `0x80` (KeyError): 키 오류 (키가 로드되지 않았거나 사용 권한 없음, 또는 잘못된 키)
- `0x0F` (ExecutionError): 명령어 실행 오류

### 복호화 실패 원인 분석

**`0x50` (ParseError) 오류**:
- **가장 일반적인 원인**: 키 슬롯에 키가 로드되지 않았거나 키 사용 권한이 설정되지 않음
- 키 슬롯이 비어있거나 초기화되지 않음
- 키 사용 권한이 Configuration Memory에서 설정되지 않음
- 해결: KeyLoad 명령어로 키를 로드하고 권한을 설정 (예제 8 참조)
- 참고: `0x50`은 `0x80` (KeyError)와 달리 파싱 단계에서 실패하므로, 키가 아예 없거나 권한이 설정되지 않은 경우에 발생할 수 있습니다

**`0x80` (KeyError) 오류**:
- 키 슬롯에 키가 로드되지 않음
- 키 사용 권한이 설정되지 않음
- 암호화에 사용한 것과 다른 키를 사용함
- 해결: KeyLoad 명령어로 올바른 키를 로드하고 권한을 설정 (예제 8 참조)

**`0x03` (ParseError) 오류**:
- Key ID가 0-15 범위를 벗어남
- 암호문 데이터가 16바이트가 아님
- 해결: 파라미터 범위 및 데이터 크기 확인

**복호화는 성공하지만 데이터가 일치하지 않음**:
- 잘못된 키를 사용함
- 암호문이 손상됨
- 해결: 키 확인, 데이터 무결성 검증

## 추가 학습 자료

1. **복호화 심화 학습**:
   - AES 복호화 알고리즘의 내부 구조
   - 복호화 모드 (ECB, CBC, GCM 등)
   - 키 관리 및 키 교체

2. **보안 기능 학습**:
   - 암호화된 통신 구현
   - 데이터 무결성 보장
   - 키 교환 프로토콜

3. **실전 활용**:
   - 대용량 데이터 복호화 (여러 블록)
   - 스트림 복호화
   - 파일 복호화 시스템

4. **복호화 모드**:
   - ECB 모드의 장단점
   - CBC 모드 사용법
   - GCM 모드 (인증 복호화)

## 실전 팁

1. **키 사전 준비**: 복호화를 수행하기 전에 올바른 키가 로드되어 있는지 확인하세요.

2. **블록 크기 준수**: 암호문 데이터는 정확히 16바이트여야 합니다.

3. **여러 블록 처리**: 더 긴 데이터는 여러 블록으로 나누어 각각 복호화하세요.

4. **복호화 검증**: 복호화된 데이터가 원본과 일치하는지 항상 확인하세요.

5. **에러 처리**: 복호화 실패 시 `0x80` (KeyError) 오류를 확인하고 키 상태를 점검하세요.

6. **키 일치 확인**: 암호화에 사용한 키와 동일한 키를 사용하는지 확인하세요.

