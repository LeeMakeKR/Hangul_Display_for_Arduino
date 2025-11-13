/*
 * HangulTypes.h
 * 
 * 한글 처리를 위한 공통 타입 정의 및 상수
 */

#ifndef HANGUL_TYPES_H
#define HANGUL_TYPES_H

#include <stdint.h>

// 글리프 크기 상수
#define HANGUL_GLYPH_WIDTH  16
#define HANGUL_GLYPH_HEIGHT 16
#define HANGUL_BYTES_PER_GLYPH 32  // 16행 × 2바이트/행

// 폰트 구조 상수
#define HANGUL_CHO_COUNT    20    // 초성 개수 (빈 값 포함)
#define HANGUL_CHO_BUL      8     // 초성 벌 수
#define HANGUL_JUNG_COUNT   22    // 중성 개수 (빈 값 포함)
#define HANGUL_JUNG_BUL     4     // 중성 벌 수
#define HANGUL_JONG_COUNT   28    // 종성 개수 (빈 값 포함)
#define HANGUL_JONG_BUL     4     // 종성 벌 수

// 폰트 섹션 오프셋
#define HANGUL_CHO_OFFSET   0
#define HANGUL_JUNG_OFFSET  (HANGUL_CHO_COUNT * HANGUL_CHO_BUL)    // 160
#define HANGUL_JONG_OFFSET  (HANGUL_JUNG_OFFSET + HANGUL_JUNG_COUNT * HANGUL_JUNG_BUL)  // 248

// 총 글리프 수
#define HANGUL_TOTAL_GLYPHS (HANGUL_CHO_COUNT * HANGUL_CHO_BUL + \
                           HANGUL_JUNG_COUNT * HANGUL_JUNG_BUL + \
                           HANGUL_JONG_COUNT * HANGUL_JONG_BUL)  // 360

// 한글 크기 옵션
enum HangulSize {
    HG_SIZE_NORMAL = 0,
    HG_SIZE_H2     = 2,    // 가로 2배
    HG_SIZE_V2     = 3,    // 세로 2배  
    HG_SIZE_X4     = 4     // 가로세로 2배
};

// 색상 모드
enum HangulColor {
    HG_COLOR_BLACK = 0,
    HG_COLOR_WHITE = 1,
    HG_COLOR_INVERT = 2
};

// 폰트 정보 구조체
struct HangulFontInfo {
    const char* name;
    uint8_t width;
    uint8_t height;
    bool hasAscii;
    const uint8_t* choData;
    const uint8_t* jungData; 
    const uint8_t* jongData;
};

#endif // HANGUL_TYPES_H