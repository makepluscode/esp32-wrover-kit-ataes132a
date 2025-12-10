# ESP32 AES132 CryptoAuth Example

ESP32에서 Atmel ATAES132A CryptoAuth 칩을 사용하는 예제 프로젝트입니다.

## 📋 목차

- [개요](#개요)
- [하드웨어 요구사항](#하드웨어-요구사항)
- [환경 설정](#환경-설정)
- [빌드 및 업로드](#빌드-및-업로드)
- [예제 목록](#예제-목록)
- [예제 선택 방법](#예제-선택-방법)
- [프로젝트 구조](#프로젝트-구조)
- [문서](#문서)

## 개요

이 프로젝트는 ESP32 마이크로컨트롤러와 Atmel ATAES132A CryptoAuth 칩 간의 I2C 통신을 구현한 예제 모음입니다. AES132의 다양한 기능을 단계별로 학습할 수 있도록 10개의 예제를 제공합니다.

### 주요 기능

- ✅ I2C 통신 (SDA: GPIO 21, SCL: GPIO 22)
- ✅ AES132 라이브러리 포팅 완료
- ✅ 공통 유틸리티 함수 제공
- ✅ 10개의 단계별 예제 (기초 → 고급)

## 하드웨어 요구사항

### 필수 하드웨어

- **ESP32 개발 보드** (ESP-WROVER-KIT 또는 호환 보드)
- **Atmel ATAES132A CryptoAuth 칩**
- **연결선** (I2C 통신용)

### 하드웨어 연결

```
ESP32          AES132
------         ------
GPIO 21  ←→   SDA
GPIO 22  ←→   SCL
3.3V     ←→   VCC
GND      ←→   GND
```

**참고**: I2C 통신을 위해 풀업 저항(4.7kΩ)이 필요할 수 있습니다.

## 환경 설정

### 1. 필수 소프트웨어 설치

#### PlatformIO 설치

**VS Code 확장 (권장)**:
1. VS Code 설치
2. 확장 마켓플레이스에서 "PlatformIO IDE" 검색 및 설치

**또는 CLI 설치**:
```bash
# Python pip를 통한 설치
pip install platformio
```

### 2. 프로젝트 클론/다운로드

```powershell
git clone <repository-url>
cd ESP32-CRYPTOAUTH-Example
```

### 3. 라이브러리 확인

프로젝트의 `lib/` 폴더에 다음 라이브러리가 포함되어 있습니다:
- `aes132/` - AES132 라이브러리 (C)
- `i2c_phys/` - I2C 물리 계층 (C++)
- `aes132_utils/` - 공통 유틸리티 (C++)

모든 라이브러리는 자동으로 빌드에 포함됩니다.

## 빌드 및 업로드

### 기본 빌드

```powershell
# 빌드만 수행
pio run -e esp-wrover-kit

# 빌드 + 업로드
pio run -e esp-wrover-kit -t upload

# 빌드 + 업로드 + 모니터 (두 단계로 실행)
pio run -e esp-wrover-kit -t upload
pio device monitor

# 또는 PowerShell에서 한 줄로 실행
pio run -e esp-wrover-kit -t upload; pio device monitor
```

### 시리얼 모니터

```powershell
# 모니터만 실행
pio device monitor

# 보드레이트 지정 (기본값: 115200)
pio device monitor -b 115200
```

## 예제 목록

총 **10개의 예제**가 제공되며, 난이도에 따라 3단계로 구분됩니다.

### 레벨 1: 기초 (Basic) - 3개 예제

| 번호 | 예제 이름 | 설명 | 난이도 |
|------|----------|------|--------|
| 1 | ✅ Info 명령어 | 디바이스 정보 읽기 | ⭐ |
| 2 | 메모리 읽기 | 일반 메모리 영역 읽기 | ⭐ |
| 3 | 메모리 쓰기 | 일반 메모리 영역 쓰기 | ⭐ |

### 레벨 2: 중급 (Intermediate) - 5개 예제

| 번호 | 예제 이름 | 설명 | 난이도 |
|------|----------|------|--------|
| 4 | 랜덤 생성 | 하드웨어 RNG 사용 | ⭐⭐ |
| 5 | 카운터 | 카운터 읽기 및 증가 | ⭐⭐ |
| 6 | 암호화 | AES-128 암호화 | ⭐⭐⭐ |
| 7 | 복호화 | AES-128 복호화 | ⭐⭐⭐ |
| 8 | 키 로드 | 키 슬롯에 키 로드 | ⭐⭐⭐ |

### 레벨 3: 고급 (Advanced) - 2개 예제

| 번호 | 예제 이름 | 설명 | 난이도 |
|------|----------|------|--------|
| 9 | 인증 | MAC 기반 인증 | ⭐⭐⭐⭐ |
| 10 | 키 생성 | 새로운 키 생성 | ⭐⭐⭐⭐ |

**상세한 예제 계획**: [docs/EXAMPLE_PLAN.md](docs/EXAMPLE_PLAN.md)

**참고**: 원래 계획은 15개였으나, 핵심 기능 중심으로 10개로 축소되었습니다.

## 예제 선택 방법

각 예제를 실행하려면 먼저 예제를 선택해야 합니다. 다음 3가지 방법 중 하나를 사용할 수 있습니다.

### 방법 1: 스크립트 사용 (가장 쉬움) ⭐⭐⭐

```powershell
# 예제 1 선택
.\scripts\select_example.ps1 1

# 빌드 및 업로드
pio run -e esp-wrover-kit -t upload
pio device monitor
```

### 방법 2: 수동으로 src/main.cpp 교체 ⭐⭐

```powershell
Copy-Item examples\01_info\main.cpp src\main.cpp -Force
pio run -e esp-wrover-kit -t upload
pio device monitor
```

### 방법 3: 프로젝트 디렉토리 지정 ⭐

```powershell
# 예제 디렉토리로 이동
cd examples\01_info
pio run -e esp-wrover-kit -t upload
pio device monitor
```

또는 프로젝트 루트에서:

```powershell
pio run -e esp-wrover-kit --project-dir examples\01_info -t upload
pio device monitor
```

**자세한 설명**: [docs/PROJECT_GUIDE.md](docs/PROJECT_GUIDE.md#예제-선택-방법)

## 프로젝트 구조

```
ESP32-CRYPTOAUTH-Example/
│
├── platformio.ini          # PlatformIO 프로젝트 설정
├── README.md               # 이 파일
│
├── lib/                    # 라이브러리
│   ├── aes132/            # AES132 라이브러리 (C)
│   ├── i2c_phys/          # I2C 물리 계층 (C++)
│   └── aes132_utils/       # 공통 유틸리티 (C++)
│
├── include/                # 공통 헤더 파일
│   ├── aes132_config.h    # 공통 설정
│   └── aes132_utils.h     # 유틸리티 함수 선언
│
├── src/                    # 현재 활성 예제
│   └── main.cpp           # 예제 코드
│
├── examples/               # 모든 예제들
│   ├── 01_info/           # 예제 1: Info 명령어
│   ├── 02_memory_read/    # 예제 2: 메모리 읽기
│   └── ...                # 나머지 예제들
│
├── scripts/                # 유틸리티 스크립트
│   └── select_example.ps1  # 예제 선택 스크립트
│
└── docs/                   # 문서
    ├── PROJECT_GUIDE.md   # 프로젝트 가이드
    ├── EXAMPLE_PLAN.md    # 예제 계획서
    └── TECHNICAL_REFERENCE.md  # 기술 참조
```

**상세한 구조 설명**: [docs/PROJECT_GUIDE.md](docs/PROJECT_GUIDE.md#프로젝트-구조)

## 문서

프로젝트의 상세한 문서는 `docs/` 디렉토리에 있습니다:

- **[PROJECT_GUIDE.md](docs/PROJECT_GUIDE.md)** - 프로젝트 전체 가이드
  - 프로젝트 구조 상세 설명
  - 예제 선택 방법
  - 빌드 및 설정 가이드

- **[EXAMPLE_PLAN.md](docs/EXAMPLE_PLAN.md)** - 예제 계획서
  - 10개 예제의 상세 계획
  - 각 예제의 목표 및 학습 내용
  - 구현 전략

- **[TECHNICAL_REFERENCE.md](docs/TECHNICAL_REFERENCE.md)** - 기술 참조
  - AES132 명령어 응답 형식
  - 에러 코드 설명
  - API 참조

## 빠른 시작

1. **환경 설정**
   ```powershell
   # PlatformIO 설치 확인
   pio --version
   ```

2. **첫 번째 예제 실행**
   ```powershell
   # Windows
   .\scripts\select_example.ps1 1
   pio run -e esp-wrover-kit -t upload
   pio device monitor
   ```

3. **시리얼 모니터 확인**
   - 보드레이트: 115200
   - 예상 출력: "ESP32 AES132 CryptoAuth Example" 및 Info 명령어 응답

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
- **해결**: PowerShell 실행 정책 확인 (`Set-ExecutionPolicy RemoteSigned`)

## 라이선스

이 프로젝트는 원본 Atmel CryptoAuth 라이브러리를 기반으로 하며, 해당 라이선스를 따릅니다.

## 참고 자료

- [ATAES132A 데이터시트](docs/ATAES132A-Data-Sheet-40002023A.pdf)
- [Atmel CryptoAuth XPro 스키매틱](docs/Atmel-CryptoAuth-XPro-Schematics.pdf)
- [PlatformIO 문서](https://docs.platformio.org/)

---
