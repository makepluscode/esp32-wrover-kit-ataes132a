# 프로젝트 가이드

이 문서는 ESP32 AES132 CryptoAuth 예제 프로젝트의 전체 가이드를 제공합니다.

## 목차

- [프로젝트 구조](#프로젝트-구조)
- [예제 선택 방법](#예제-선택-방법)
- [빌드 및 설정](#빌드-및-설정)
- [예제 작성 가이드](#예제-작성-가이드)

---

## 프로젝트 구조

### 전체 폴더 구조

```
ESP32-CRYPTOAUTH-Example/
│
├── platformio.ini              # PlatformIO 프로젝트 설정
├── README.md                   # 프로젝트 메인 README
│
├── lib/                        # 라이브러리
│   ├── aes132/                 # AES132 라이브러리 (C)
│   │   ├── *.c, *.h
│   │   └── library.json
│   │
│   ├── i2c_phys/               # I2C 물리 계층 (C++)
│   │   ├── i2c_phys.cpp, .h
│   │   └── library.json
│   │
│   └── aes132_utils/           # 공통 유틸리티 (C++)
│       ├── aes132_utils.cpp
│       └── library.json
│
├── include/                    # 공통 헤더 파일
│   ├── aes132_config.h         # 공통 설정 (핀, 주소 등)
│   └── aes132_utils.h         # 공통 유틸리티 함수 선언
│
├── src/                        # 현재 활성 예제 (기본)
│   └── main.cpp                # 예제 코드
│
├── examples/                   # 모든 예제들
│   ├── README.md               # 예제 전체 가이드
│   ├── 01_info/                # 예제 1: Info 명령어
│   ├── 02_memory_read/         # 예제 2: 메모리 읽기
│   └── ...                     # 나머지 예제들
│
├── scripts/                    # 유틸리티 스크립트
│   ├── select_example.ps1      # Windows용 예제 선택
│   └── select_example.sh       # Linux/Mac용 예제 선택
│
└── docs/                       # 문서
    ├── PROJECT_GUIDE.md       # 이 파일
    ├── EXAMPLE_PLAN.md        # 예제 계획서
    └── TECHNICAL_REFERENCE.md  # 기술 참조
```

### 주요 파일 설명

#### 공통 설정 (`include/aes132_config.h`)
- I2C 핀 설정 (SDA: GPIO 21, SCL: GPIO 22)
- AES132 I2C 주소 (기본값: 0xA0)
- 시리얼 통신 보드레이트 (115200)

#### 공통 유틸리티 (`include/aes132_utils.h` + `lib/aes132_utils/`)
- `aes132_init()`: AES132 초기화 함수
- `print_hex()`: 16진수 출력
- `print_response()`: 응답 패킷 분석 및 출력
- `get_error_string()`: 에러 코드 → 문자열 변환
- `print_result()`: 작업 결과 출력
- `compare_data()`: 데이터 비교

#### 예제 구조 (`examples/XX_name/`)
각 예제는 독립적인 폴더로 구성:
- `main.cpp`: 예제 코드
- `README.md`: 예제 설명 및 사용법

### 의존성 구조

```
examples/XX_example/
  └── main.cpp
      ├── aes132_utils.h        (include/)
      │   └── aes132_utils.cpp  (lib/aes132_utils/)
      │
      ├── aes132_config.h        (include/)
      │
      ├── aes132_comm_marshaling.h  (lib/aes132/)
      │   └── aes132_comm_marshaling.c
      │
      └── i2c_phys.h             (lib/i2c_phys/)
          └── i2c_phys.cpp
```

---

## 예제 선택 방법

각 예제를 실행하려면 먼저 예제를 선택해야 합니다. 다음 3가지 방법 중 하나를 사용할 수 있습니다.

### 방법 1: 스크립트 사용 (가장 쉬움) ⭐⭐⭐

#### Windows (PowerShell)

```powershell
# 예제 1 선택
.\scripts\select_example.ps1 1

# 빌드 및 업로드
pio run -e esp-wrover-kit -t upload
pio device monitor
```

#### Linux/Mac (Bash)

```bash
# 실행 권한 부여 (최초 1회)
chmod +x scripts/select_example.sh

# 예제 1 선택
./scripts/select_example.sh 1

# 빌드 및 업로드
pio run -e esp-wrover-kit -t upload
pio device monitor
```

**장점**:
- ✅ 가장 간단함
- ✅ 자동으로 src/main.cpp 교체
- ✅ 모든 PlatformIO 버전에서 작동

### 방법 2: 수동으로 src/main.cpp 교체 ⭐⭐

#### Windows

```powershell
Copy-Item examples\01_info\main.cpp src\main.cpp -Force
pio run -e esp-wrover-kit -t upload monitor
```

#### Linux/Mac

```bash
cp examples/01_info/main.cpp src/main.cpp
pio run -e esp-wrover-kit -t upload monitor
```

**장점**:
- ✅ 스크립트 없이도 사용 가능
- ✅ 가장 빠름

### 방법 3: 프로젝트 디렉토리 지정 ⭐

```bash
# 예제 디렉토리로 이동
cd examples/01_info
pio run -e esp-wrover-kit -t upload monitor
```

또는 프로젝트 루트에서:

```bash
pio run -e esp-wrover-kit --project-dir examples/01_info -t upload monitor
```

**장점**:
- ✅ 각 예제가 완전히 독립적
- ✅ 여러 예제를 동시에 작업 가능

### 빠른 참조 테이블

| 예제 | 디렉토리 | 스크립트 사용 |
|------|----------|---------------|
| 예제 1: Info | `examples/01_info/` | `.\scripts\select_example.ps1 1` |
| 예제 2: Memory Read | `examples/02_memory_read/` | `.\scripts\select_example.ps1 2` |
| 예제 3: Memory Write | `examples/03_memory_write/` | `.\scripts\select_example.ps1 3` |
| ... | ... | ... |

---

## 빌드 및 설정

### PlatformIO 설정

`platformio.ini` 주요 설정:

```ini
[env:esp-wrover-kit]
platform = espressif32
board = esp-wrover-kit
framework = arduino
monitor_speed = 115200
build_flags = 
    -Wall
    -Wextra
    -Iinclude              # include 폴더를 헤더 검색 경로에 추가
lib_extra_dirs = lib       # lib 폴더의 라이브러리 자동 인식
```

### 자동 라이브러리 인식

PlatformIO는 다음을 자동으로 인식합니다:
- `lib/*/library.json`이 있는 폴더를 라이브러리로 인식
- `lib/*/*.cpp`, `lib/*/*.c` 파일을 자동 컴파일
- `lib/*/*.h` 파일을 헤더 검색 경로에 추가

### 빌드 명령어

```bash
# 빌드만 수행
pio run -e esp-wrover-kit

# 빌드 + 업로드
pio run -e esp-wrover-kit -t upload

# 빌드 + 업로드 + 모니터 (두 단계로 실행)
pio run -e esp-wrover-kit -t upload
pio device monitor

# 시리얼 모니터만 실행
pio device monitor
```

---

## 예제 작성 가이드

### 새 예제 추가 시

1. **폴더 생성**
   ```bash
   mkdir examples/02_memory_read
   ```

2. **main.cpp 작성**
   ```cpp
   #include <Arduino.h>
   #include "aes132_comm_marshaling.h"
   #include "i2c_phys.h"
   #include "aes132_utils.h"      // 공통 유틸리티
   #include "aes132_config.h"     // 공통 설정
   
   void setup(void) {
       Serial.begin(AES132_SERIAL_BAUD);
       // ... 초기화 및 예제 코드
   }
   
   void loop(void) {
       // 반복 실행 (필요시)
   }
   ```

3. **README.md 작성**
   - 예제 목적
   - 사용 방법
   - 예상 출력
   - 관련 문서 참조

### 공통 함수 사용

모든 예제는 `aes132_utils.h`의 공통 함수를 사용할 수 있습니다:

```cpp
// AES132 초기화
uint8_t ret = aes132_init();

// 16진수 출력
print_hex("Data: ", data, length);

// 응답 패킷 분석
print_response(rx_buffer, count);

// 결과 출력
print_result("Operation", ret);

// 에러 코드 확인
const char* error_msg = get_error_string(error_code);
```

---

## 문제 해결

### 빌드 오류

**문제**: `aes132_utils.h`를 찾을 수 없음
- **해결**: `platformio.ini`에 `-Iinclude` 플래그가 있는지 확인

**문제**: 라이브러리를 찾을 수 없음
- **해결**: `lib_extra_dirs = lib` 설정 확인

### I2C 통신 오류

**문제**: 디바이스 응답 없음
- **해결**: 
  - 하드웨어 연결 확인 (SDA: GPIO 21, SCL: GPIO 22)
  - I2C 주소 확인 (기본값: 0xA0)
  - 풀업 저항 확인
  - 전원 공급 확인

**문제**: 통신 실패
- **해결**:
  - 시리얼 모니터 보드레이트 확인 (115200)
  - Wake-up 명령어 실행 확인
  - I2C 버스 상태 확인

### 예제 선택 오류

**문제**: 스크립트 실행 오류
- **해결**: 
  - Windows: PowerShell 실행 정책 확인 (`Set-ExecutionPolicy RemoteSigned`)
  - Linux/Mac: 실행 권한 확인 (`chmod +x scripts/select_example.sh`)

---

## 팁

### 빠른 예제 전환
`src/main.cpp`를 원하는 예제의 `main.cpp`로 교체하여 빠르게 테스트할 수 있습니다.

### 디버깅
각 예제는 독립적으로 빌드되므로, 문제가 발생하면 해당 예제 폴더에서만 디버깅하면 됩니다.

### 코드 재사용
공통 함수는 `aes132_utils.h`에 추가하여 모든 예제에서 사용할 수 있습니다.

---

## 관련 문서

- [README.md](../README.md) - 프로젝트 메인 README
- [EXAMPLE_PLAN.md](EXAMPLE_PLAN.md) - 예제 계획서
- [TECHNICAL_REFERENCE.md](TECHNICAL_REFERENCE.md) - 기술 참조

