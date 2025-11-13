/*
 * HangulCore.h
 * 
 * 한글 처리 핵심 로직 (라이브러리 독립적)
 * - UTF-8 한글 문자열을 초성, 중성, 종성으로 분해
 * - 각 자소의 벌(Bul) 선택 로직
 * - 조합형 한글 처리
 */

#ifndef HANGUL_CORE_H
#define HANGUL_CORE_H

#include <stdint.h>

namespace HangulCore {

    // 한글 구성 요소 구조체
    struct HangulComponents {
        uint8_t cho;      // 초성 인덱스 (1~19, 0은 빈 값)
        uint8_t jung;     // 중성 인덱스 (1~21, 0은 빈 값)
        uint8_t jong;     // 종성 인덱스 (0~27, 0은 받침 없음)
        uint8_t choBul;   // 초성 벌 (0~7)
        uint8_t jungBul;  // 중성 벌 (0~3)
        uint8_t jongBul;  // 종성 벌 (0~3)
    };

    // UTF-8 한글 문자 분해 (3바이트 → 초중종성 + 벌)
    HangulComponents decompose(uint8_t byte1, uint8_t byte2, uint8_t byte3);
    
    // 초성 벌 선택 로직
    uint8_t getChosungBul(uint8_t jung, bool hasJong);
    
    // 중성 벌 선택 로직  
    uint8_t getJungsungBul(uint8_t cho, bool hasJong);
    
    // 종성 벌 선택 로직
    uint8_t getJongsungBul(uint8_t jung);
    
    // UTF-8 한글 문자인지 확인
    bool isHangul(uint8_t byte1, uint8_t byte2, uint8_t byte3);
    
    // UTF-8 문자열에서 다음 한글 문자 위치 찾기
    const char* findNextHangul(const char* utf8String);
    
    // 디버그용: 분해 결과 출력
    void printComponents(const HangulComponents& comp);

} // namespace HangulCore

#endif // HANGUL_CORE_H