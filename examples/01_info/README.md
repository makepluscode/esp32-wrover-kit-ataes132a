# 예제 1: Info 명령어

## 목적

AES132 디바이스의 기본 정보를 읽어오는 방법을 학습합니다.

## 설명

Info 명령어는 AES132 칩의 기본 정보를 반환합니다:
- 디바이스 버전 정보
- 제조사 ID
- 하드웨어 리비전
- 구성 플래그

## 사용 방법

```bash
# 빌드
pio run -e esp-wrover-kit

# 업로드
pio run -e esp-wrover-kit --target upload

# 모니터
pio device monitor
```

## 예상 출력

```
========================================
ESP32 AES132 CryptoAuth Example
Example 01: Info Command
========================================

AES132 initialized successfully

[Info Command] SUCCESS

=== Response Packet ===
Count: 6
Status: 0x00 (Success)
Data: 00 00 78 00
======================

=== Detailed Response ===
Response count: 6

Response data (byte-by-byte):
  [0] = 0x06
  [1] = 0x00
  [2] = 0x00
  [3] = 0x00
  [4] = 0x78
  [5] = 0x00
```

## 응답 데이터 해석

- **[0] Count**: 응답 패킷의 총 바이트 수
- **[1] Status**: 명령어 실행 결과 (0x00 = 성공)
- **[2-5] Data**: 디바이스 정보 데이터

## 관련 문서

- `docs/INFO_COMMAND_RESPONSE.md`: Info 명령어 응답 상세 분석
- ATAES132A 데이터시트: Info Command 섹션

## 다음 예제

- [예제 2: 메모리 읽기](../02_memory_read/README.md) - BlockRead 명령어로 메모리 읽기

