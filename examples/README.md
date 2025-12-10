# AES132 예제 모음

이 폴더에는 AES132 CryptoAuth 칩의 다양한 기능을 학습할 수 있는 예제들이 포함되어 있습니다.

## 예제 목록

총 **10개의 예제**가 제공되며, 난이도에 따라 3단계로 구분됩니다.

### 레벨 1: 기초 (Basic) - 3개 예제
1. ✅ [예제 1: Info 명령어](01_info/) - 디바이스 정보 읽기
2. ✅ [예제 2: 메모리 읽기](02_memory_read/) - BlockRead 명령어로 메모리 읽기
3. ✅ [예제 3: 메모리 쓰기](03_memory_write/) - 표준 EEPROM 쓰기 방식으로 메모리 쓰기

### 레벨 2: 중급 (Intermediate) - 5개 예제
4. ✅ [예제 4: 랜덤 생성](04_random/) - 하드웨어 RNG 사용
5. ✅ [예제 5: 카운터](05_counter/) - 카운터 읽기 및 증가
6. ✅ [예제 6: 암호화](06_encrypt/) - AES-128 암호화
7. ✅ [예제 7: 복호화](07_decrypt/) - AES-128 복호화
8. [예제 8: 키 로드](08_key_load/) - 키 슬롯에 키 로드

### 레벨 3: 고급 (Advanced) - 2개 예제
9. [예제 9: 인증](09_authentication/) - MAC 기반 인증
10. [예제 10: 키 생성](10_key_create/) - 새로운 키 생성

## 사용 방법

### 특정 예제 빌드 및 실행

```bash
# 예제 디렉토리로 이동
cd examples/01_info

# 빌드
pio run -e esp-wrover-kit

# 업로드
pio run -e esp-wrover-kit --target upload

# 모니터
pio device monitor
```

### 또는 프로젝트 루트에서

```bash
# 빌드
pio run -e esp-wrover-kit --project-dir examples/01_info

# 업로드
pio run -e esp-wrover-kit --project-dir examples/01_info --target upload

# 모니터
pio device monitor --project-dir examples/01_info
```

## 공통 파일

모든 예제는 다음 공통 파일을 사용합니다:

- `include/aes132_config.h`: 공통 설정 (핀, 주소 등)
- `include/aes132_utils.h`: 공통 유틸리티 함수
- `lib/aes132/`: AES132 라이브러리
- `lib/i2c_phys/`: I2C 물리 계층

## 학습 순서

예제는 쉬운 것부터 어려운 것까지 순서대로 학습하는 것을 권장합니다:

1. **기초 단계**: 예제 1-3을 통해 기본 개념 학습
2. **중급 단계**: 예제 4-8을 통해 암호화 및 키 관리 학습
3. **고급 단계**: 예제 9-10을 통해 실제 보안 기능 구현

## 문제 해결

### 빌드 오류
- `include/aes132_utils.h`를 찾을 수 없다면 `platformio.ini`에 `-Iinclude` 플래그가 있는지 확인하세요.

### I2C 통신 오류
- 하드웨어 연결 확인 (SDA: GPIO 21, SCL: GPIO 22)
- I2C 주소 확인 (기본값: 0xA0)
- 풀업 저항 확인

### 디바이스 응답 없음
- AES132 디바이스가 전원 공급되고 있는지 확인
- Wake-up 명령어가 실행되었는지 확인
- 시리얼 모니터 보드레이트 확인 (115200)

## 관련 문서

- [예제 계획서](../docs/EXAMPLE_PLAN.md)
- [프로젝트 가이드](../docs/PROJECT_GUIDE.md)
- [기술 참조](../docs/TECHNICAL_REFERENCE.md)

