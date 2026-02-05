/*
 * hangulDisp.h
 * 
 * hangulDisp 라이브러리 통합 헤더
 * - HangulCore 타입/상수/함수 선언
 * - 통합 디스플레이 어댑터
 * 
 * 사용 방법:
 * 1. 폰트 헤더 포함
 * 2. hangulDisp<폰트> 인스턴스 생성
 * 3. 픽셀 그리기 콜백 제공
 * 4. print() 함수로 UTF-8 문자열 출력
 * 
 * 예제:
 * ```cpp
 * #include "hangulDisp.h"
 * #include "fonts/hangul/H01_Font.h"
 * 
 * TFT_eSPI tft;  // 또는 U8G2, GxEPD2 등
 * 
 * void drawPixel(int16_t x, int16_t y, uint16_t color) {
 *     tft.drawPixel(x, y, color ? TFT_WHITE : TFT_BLACK);
 * }
 * 
 * hangulDisp<Font_H01> hangul(drawPixel);
 * hangul.setCursor(10, 30);
 * hangul.print("안녕하세요!");
 * ```
 */

#ifndef HANGUL_DISP_H
#define HANGUL_DISP_H

#include <stdint.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#endif

// ============================================================================
// 상수 정의
// ============================================================================

// 글리프 크기 상수
#define HANGUL_GLYPH_WIDTH  16
#define HANGUL_GLYPH_HEIGHT 16
#define HANGUL_BYTES_PER_GLYPH 32  // 16행 x 2바이트/행

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

// ============================================================================
// 타입 정의
// ============================================================================

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

// 한글 구성 요소 구조체
struct HangulComponents {
    uint8_t cho;      // 초성 인덱스 (1~19, 0은 빈 값)
    uint8_t jung;     // 중성 인덱스 (1~21, 0은 빈 값)
    uint8_t jong;     // 종성 인덱스 (0~27, 0은 받침 없음)
    uint8_t choBul;   // 초성 벌 (0~7)
    uint8_t jungBul;  // 중성 벌 (0~3)
    uint8_t jongBul;  // 종성 벌 (0~3)
};

// 글리프 데이터 구조체
struct HangulGlyphSet {
    const uint8_t* cho;   // 초성 글리프 (32바이트)
    const uint8_t* jung;  // 중성 글리프 (32바이트)
    const uint8_t* jong;  // 종성 글리프 (32바이트, nullptr이면 받침 없음)
};

// ============================================================================
// 함수 선언
// ============================================================================

namespace HangulCore {

    // UTF-8 한글 문자 분해 (3바이트 -> 초중종성 + 벌)
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

// ============================================================================
// 통합 어댑터
// ============================================================================

// 픽셀 그리기 콜백 함수 타입
typedef void (*PixelDrawCallback)(int16_t x, int16_t y, uint16_t color);

template<typename FontNamespace>
class hangulDisp {
private:
    int16_t cursorX;
    int16_t cursorY;
    HangulSize textSize;
    HangulColor textColor;
    PixelDrawCallback drawPixelCallback;

public:
    // 생성자 - 픽셀 그리기 콜백 함수 필요
    explicit hangulDisp(PixelDrawCallback callback)
        : cursorX(0), cursorY(0),
          textSize(HG_SIZE_NORMAL),
          textColor(HG_COLOR_BLACK),
          drawPixelCallback(callback) {}

    // 설정 메서드
    void setCursor(int16_t x, int16_t y) {
        cursorX = x;
        cursorY = y;
    }

    void setTextSize(HangulSize size) {
        textSize = size;
    }

    void setTextColor(HangulColor color) {
        textColor = color;
    }

    int16_t getCursorX() const { return cursorX; }
    int16_t getCursorY() const { return cursorY; }

    // UTF-8 문자열 출력
    void print(const char* utf8Text) {
        if (!utf8Text || !drawPixelCallback) return;
        
        const char* p = utf8Text;
        while (*p) {
            if ((*p & 0xE0) == 0xE0) {
                if (p[1] && p[2]) {
                    printHangulChar((uint8_t)p[0], (uint8_t)p[1], (uint8_t)p[2]);
                    p += 3;
                } else {
                    break;
                }
            } else if ((*p & 0x80) == 0) {
                // TODO: ASCII 폰트 지원 추가
                cursorX += 8;
                p++;
            } else if ((*p & 0xE0) == 0xC0) {
                p += 2;
            } else {
                p++;
            }
        }
    }

    // 단일 한글 문자 출력 (UTF-8 3바이트)
    void printHangulChar(uint8_t b1, uint8_t b2, uint8_t b3) {
        HangulComponents comp = HangulCore::decompose(b1, b2, b3);
        HangulGlyphSet glyphs = getGlyphSet(comp);
        drawCombinedGlyph(cursorX, cursorY, glyphs);
        advanceCursor();
    }

private:
    HangulGlyphSet getGlyphSet(const HangulComponents& comp) {
        HangulGlyphSet glyphs;
        glyphs.cho = FontNamespace::CHO_DATA[comp.cho][comp.choBul];
        glyphs.jung = FontNamespace::JUNG_DATA[comp.jung][comp.jungBul];
        if (comp.jong > 0) {
            glyphs.jong = FontNamespace::JONG_DATA[comp.jong][comp.jongBul];
        } else {
            glyphs.jong = nullptr;
        }
        return glyphs;
    }

    void drawCombinedGlyph(int16_t x, int16_t y, const HangulGlyphSet& glyphs) {
        uint8_t scaleX = (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4) ? 2 : 1;
        uint8_t scaleY = (textSize == HG_SIZE_V2 || textSize == HG_SIZE_X4) ? 2 : 1;
        
        for (int16_t row = 0; row < HANGUL_GLYPH_HEIGHT; row++) {
            for (int16_t col = 0; col < HANGUL_GLYPH_WIDTH; col++) {
                bool pixel = false;
                
                if (glyphs.cho) {
                    pixel |= getPixelFromGlyph(glyphs.cho, row, col);
                }
                if (glyphs.jung) {
                    pixel |= getPixelFromGlyph(glyphs.jung, row, col);
                }
                if (glyphs.jong) {
                    pixel |= getPixelFromGlyph(glyphs.jong, row, col);
                }
                
                if (pixel) {
                    uint16_t color = (textColor == HG_COLOR_WHITE) ? 1 : 0;
                    for (uint8_t sy = 0; sy < scaleY; sy++) {
                        for (uint8_t sx = 0; sx < scaleX; sx++) {
                            drawPixelCallback(x + col * scaleX + sx,
                                            y + row * scaleY + sy,
                                            color);
                        }
                    }
                }
            }
        }
    }

    bool getPixelFromGlyph(const uint8_t* glyph, int16_t row, int16_t col) {
        uint16_t rowData = ((uint16_t)pgm_read_byte(glyph + row * 2) << 8) |
                           pgm_read_byte(glyph + row * 2 + 1);
        return (rowData & (0x8000 >> col)) != 0;
    }

    void advanceCursor() {
        uint8_t scaleX = (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4) ? 2 : 1;
        cursorX += HANGUL_GLYPH_WIDTH * scaleX;
    }
};

#endif // HANGUL_DISP_H
