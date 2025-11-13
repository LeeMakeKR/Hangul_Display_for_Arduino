/*
 * H01_Font.h
 * 
 * H01 한글 폰트 데이터
 * 
 * 폰트 크기: 16x16 픽셀
 * 인코딩: EasyView 조합형 한글
 * 
 * 구조:
 * - CHO_DATA[20][8][32]: 초성 20자 × 8벌 × 32바이트
 * - JUNG_DATA[22][4][32]: 중성 22자 × 4벌 × 32바이트  
 * - JONG_DATA[28][4][32]: 종성 28자 × 4벌 × 32바이트
 */

#ifndef H01_FONT_H
#define H01_FONT_H

#include <Arduino.h>
#include "../core/HangulTypes.h"

namespace Font_H01 {

    // 폰트 정보
    const char* FONT_NAME = "H01";
    const uint8_t GLYPH_WIDTH = 16;
    const uint8_t GLYPH_HEIGHT = 16;
    const bool HAS_ASCII = false;

    // 초성 데이터 (20자 × 8벌 × 32바이트)
    // 인덱스 0: 빈 값, 1~19: ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ
    extern const uint8_t CHO_DATA[HANGUL_CHO_COUNT][HANGUL_CHO_BUL][HANGUL_BYTES_PER_GLYPH] PROGMEM;

    // 중성 데이터 (22자 × 4벌 × 32바이트)
    // 인덱스 0,1: 빈 값, 2~21: ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ
    extern const uint8_t JUNG_DATA[HANGUL_JUNG_COUNT][HANGUL_JUNG_BUL][HANGUL_BYTES_PER_GLYPH] PROGMEM;

    // 종성 데이터 (28자 × 4벌 × 32바이트)  
    // 인덱스 0: 빈 값(받침없음), 1~27: ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ
    extern const uint8_t JONG_DATA[HANGUL_JONG_COUNT][HANGUL_JONG_BUL][HANGUL_BYTES_PER_GLYPH] PROGMEM;

    // 폰트 정보 구조체
    const HangulFontInfo FONT_INFO = {
        FONT_NAME,
        GLYPH_WIDTH,
        GLYPH_HEIGHT,
        HAS_ASCII,
        (const uint8_t*)CHO_DATA,
        (const uint8_t*)JUNG_DATA,
        (const uint8_t*)JONG_DATA
    };

} // namespace Font_H01

#endif // H01_FONT_H