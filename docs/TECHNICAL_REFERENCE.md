# 기술 참조

이 문서는 AES132 CryptoAuth 칩의 기술적 세부사항과 API 참조를 제공합니다.

## 목차

- [AES132 응답 패킷 구조](#aes132-응답-패킷-구조)
- [에러 코드](#에러-코드)
- [Info 명령어 응답 분석](#info-명령어-응답-분석)
- [API 참조](#api-참조)

---

## AES132 응답 패킷 구조

AES132의 모든 응답 패킷은 다음 구조를 따릅니다:

```
[0] Count      : 패킷의 총 바이트 수 (Count + Body + Checksum 포함)
[1] Status     : 명령어 실행 결과 코드 (Return Code)
[2~n] Data     : 명령어별 응답 데이터
[n+1~n+2] CRC  : 체크섬 (2바이트)
```

### 패킷 구조 상세

| 바이트 위치 | 이름 | 설명 |
|------------|------|------|
| 0 | Count | 패킷의 총 바이트 수 (자기 자신 포함) |
| 1 | Status | 명령어 실행 결과 코드 (0x00 = 성공) |
| 2 ~ n | Data | 명령어별 응답 데이터 (가변 길이) |
| n+1 ~ n+2 | CRC | 체크섬 (2바이트) |

---

## 에러 코드

### 성공 코드

- `0x00` - `AES132_DEVICE_RETCODE_SUCCESS`: 성공

### 에러 코드 목록

| 코드 | 이름 | 설명 |
|------|------|------|
| `0x02` | Boundary Error | 페이지/키 경계 오류 |
| `0x04` | Read/Write Config Error | 접근 권한 오류 |
| `0x08` | Bad Address | 잘못된 주소 |
| `0x10` | Count Error | 카운터 오류 |
| `0x20` | Nonce Error | Nonce 오류 |
| `0x40` | MAC Error | 인증 MAC 오류 |
| `0x50` | Parse Error | 명령어 파싱 오류 |
| `0x60` | Data Mismatch | 데이터 불일치 |
| `0x70` | Lock Error | 잠금 오류 |
| `0x80` | Key Error | 키 오류 |
| `0x90` | Temp Sense Error | 온도 센서 타임아웃 |

### 에러 코드 확인 방법

```cpp
#include "aes132_utils.h"

uint8_t result = aes132m_execute(...);
if (result != AES132_DEVICE_RETCODE_SUCCESS) {
    const char* error_msg = get_error_string(result);
    Serial.print("Error: ");
    Serial.println(error_msg);
}
```

---

## Info 명령어 응답 분석

### 예제 출력

```
Response count: 6
Response data:
  [0] = 0x06  ← Count 바이트
  [1] = 0x00  ← Status (성공)
  [2] = 0x00  ← Data Byte 1
  [3] = 0x00  ← Data Byte 2
  [4] = 0x78  ← Data Byte 3
  [5] = 0x00  ← Data Byte 4
```

### 각 바이트의 의미

#### [0] = 0x06 (Count)
- **의미**: 응답 패킷의 총 바이트 수
- **값**: 6바이트 (Count + Status + Data + CRC 포함)
- **설명**: 이 값은 패킷의 길이를 나타냅니다.

#### [1] = 0x00 (Status/Return Code)
- **의미**: 명령어 실행 결과 코드
- **값**: `0x00` = `AES132_DEVICE_RETCODE_SUCCESS` (성공)
- **설명**: 명령어가 성공적으로 실행되었음을 의미합니다.

#### [2] = 0x00 (Data Byte 1)
- **의미**: Info 명령어 응답 데이터의 첫 번째 바이트
- **설명**: 디바이스 정보의 일부 (일반적으로 상위 바이트 또는 플래그)

#### [3] = 0x00 (Data Byte 2)
- **의미**: Info 명령어 응답 데이터의 두 번째 바이트
- **설명**: 디바이스 정보의 일부

#### [4] = 0x78 (Data Byte 3)
- **의미**: Info 명령어 응답 데이터의 세 번째 바이트
- **값**: `0x78` (이진수: `0111 1000`)
- **설명**: 
  - 디바이스 ID, 버전 정보, 또는 구성 정보를 나타낼 수 있습니다.
  - 비트 패턴 분석:
    - Bit 7-4: `0111` (7)
    - Bit 3-0: `1000` (8)
  - 이는 디바이스의 하드웨어 버전이나 제조사 ID의 일부일 수 있습니다.

#### [5] = 0x00 (Data Byte 4 또는 CRC 시작)
- **의미**: 네 번째 데이터 바이트이거나 CRC 체크섬의 시작 바이트
- **설명**: Count가 6이므로, 실제 데이터는 [2]~[5]까지 4바이트일 수 있습니다.

### Info 명령어의 일반적인 응답

Info 명령어는 일반적으로 다음 정보를 반환합니다:
- 디바이스 버전 정보
- 제조사 ID
- 하드웨어 리비전
- 구성 플래그

정확한 데이터 형식은 ATAES132A 데이터시트의 "Info Command" 섹션을 참조하세요.

---

## API 참조

### 초기화 함수

#### `aes132_init()`

AES132 디바이스를 초기화합니다.

```cpp
uint8_t aes132_init(void);
```

**반환값**:
- `AES132_FUNCTION_RETCODE_SUCCESS`: 성공
- 그 외의 값: 오류 코드

**사용 예**:
```cpp
uint8_t ret = aes132_init();
if (ret != AES132_FUNCTION_RETCODE_SUCCESS) {
    Serial.println("Initialization failed!");
    return;
}
```

### 출력 유틸리티 함수

#### `print_hex()`

바이트 배열을 16진수 형식으로 출력합니다.

```cpp
void print_hex(const char* label, const uint8_t* data, uint8_t length);
```

**매개변수**:
- `label`: 출력할 레이블 (NULL 가능)
- `data`: 출력할 데이터 배열
- `length`: 데이터 길이 (바이트 수)

**사용 예**:
```cpp
uint8_t data[] = {0x01, 0x02, 0x03};
print_hex("Data: ", data, 3);
// 출력: Data: 01 02 03
```

#### `print_response()`

AES132 응답 패킷을 분석하여 출력합니다.

```cpp
void print_response(const uint8_t* rx_buffer, uint8_t count);
```

**매개변수**:
- `rx_buffer`: 응답 버퍼
- `count`: 응답 패킷의 Count 바이트 값

**사용 예**:
```cpp
uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX];
uint8_t count = rx_buffer[AES132_RESPONSE_INDEX_COUNT];
print_response(rx_buffer, count);
```

#### `get_error_string()`

에러 코드를 문자열로 변환합니다.

```cpp
const char* get_error_string(uint8_t error_code);
```

**매개변수**:
- `error_code`: 에러 코드

**반환값**: 에러 코드에 해당하는 문자열

**사용 예**:
```cpp
uint8_t result = aes132m_execute(...);
if (result != AES132_DEVICE_RETCODE_SUCCESS) {
    Serial.println(get_error_string(result));
}
```

#### `print_result()`

성공/실패 메시지를 출력합니다.

```cpp
void print_result(const char* operation, uint8_t result);
```

**매개변수**:
- `operation`: 작업 이름
- `result`: 결과 코드

**사용 예**:
```cpp
uint8_t ret = aes132m_execute(...);
print_result("Info Command", ret);
// 출력: [Info Command] SUCCESS
// 또는: [Info Command] FAILED: 0x40 (MAC Error)
```

### 데이터 비교 함수

#### `compare_data()`

두 바이트 배열을 비교합니다.

```cpp
int compare_data(const uint8_t* data1, const uint8_t* data2, uint8_t length);
```

**매개변수**:
- `data1`: 첫 번째 데이터 배열
- `data2`: 두 번째 데이터 배열
- `length`: 비교할 길이

**반환값**:
- `0`: 같음
- 그 외: 다름

**사용 예**:
```cpp
uint8_t data1[] = {0x01, 0x02, 0x03};
uint8_t data2[] = {0x01, 0x02, 0x03};
if (compare_data(data1, data2, 3) == 0) {
    Serial.println("Data matches!");
}
```

---

## 관련 문서

- [README.md](../README.md) - 프로젝트 메인 README
- [PROJECT_GUIDE.md](PROJECT_GUIDE.md) - 프로젝트 가이드
- [EXAMPLE_PLAN.md](EXAMPLE_PLAN.md) - 예제 계획서
- [ATAES132A 데이터시트](ATAES132A-Data-Sheet-40002023A.pdf)

