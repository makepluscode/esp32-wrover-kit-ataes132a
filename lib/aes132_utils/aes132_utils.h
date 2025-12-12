/**
 * @file aes132_utils.h
 * @brief AES132 공통 유틸리티 함수
 * 
 * 모든 예제에서 공통으로 사용하는 헬퍼 함수들을 제공합니다.
 */

#ifndef AES132_UTILS_H
#define AES132_UTILS_H

#include <stdint.h>
#include "aes132_comm.h"
#include "aes132_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// 초기화 함수
// ============================================================================

/**
 * @brief AES132 디바이스를 초기화합니다.
 * 
 * 이 함수는 다음 작업을 수행합니다:
 * 1. I2C 핀 설정
 * 2. I2C 인터페이스 활성화
 * 3. AES132 디바이스 선택
 * 4. 디바이스 웨이크업
 * 
 * @return AES132_FUNCTION_RETCODE_SUCCESS 성공 시
 *         그 외의 값은 오류 코드
 */
uint8_t aes132_init(void);

// ============================================================================
// 출력 유틸리티 함수
// ============================================================================

/**
 * @brief 바이트 배열을 16진수 형식으로 출력합니다.
 * 
 * @param label 출력할 레이블 (예: "Data: ")
 * @param data 출력할 데이터 배열
 * @param length 데이터 길이 (바이트 수)
 */
void print_hex(const char* label, const uint8_t* data, uint8_t length);

/**
 * @brief AES132 응답 패킷을 분석하여 출력합니다.
 * 
 * @param rx_buffer 응답 버퍼
 * @param count 응답 패킷의 Count 바이트 값
 */
void print_response(const uint8_t* rx_buffer, uint8_t count);

/**
 * @brief 에러 코드를 문자열로 변환합니다.
 * 
 * @param error_code 에러 코드
 * @return 에러 코드에 해당하는 문자열
 */
const char* get_error_string(uint8_t error_code);

/**
 * @brief 성공/실패 메시지를 출력합니다.
 * 
 * @param operation 작업 이름 (예: "Memory Read")
 * @param result 결과 코드
 */
void print_result(const char* operation, uint8_t result);

// ============================================================================
// 데이터 비교 함수
// ============================================================================

/**
 * @brief 두 바이트 배열을 비교합니다.
 * 
 * @param data1 첫 번째 데이터 배열
 * @param data2 두 번째 데이터 배열
 * @param length 비교할 길이
 * @return 0이면 같음, 그 외는 다름
 */
int compare_data(const uint8_t* data1, const uint8_t* data2, uint8_t length);



#ifdef __cplusplus
}
#endif

#endif /* AES132_UTILS_H */

