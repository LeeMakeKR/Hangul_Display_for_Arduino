/*
 * hangulDisp.h
 * 
 * hangulDisp 라이브러리 통합 헤더
 * - HangulCore 타입/상수/함수 선언
 * - 통합 디스플레이 어댑터
 * 
 * 사용 방법:
 * 1. 폰트 헤더 포함 (사용할 폰트에 따라 다름, 예: H01_FONT.h, H02_FONT.h 등)
 * 2. hangulDisp 인스턴스 생성
 * 3. 픽셀 그리기 콜백 제공
 * 4. setFont()로 폰트 지정
 * 5. print() 함수로 UTF-8 문자열 출력
 * 
 * 예제 (H01 폰트 사용시):
 * ```cpp
 * #include "hangulDisp.h"
 * #include "H01_FONT.h"  // 사용할 폰트 wrapper 파일
 * 
 * TFT_eSPI tft;  // 또는 U8G2, GxEPD2 등
 * 
 * void drawPixel(int16_t x, int16_t y, uint16_t color) {
 *     tft.drawPixel(x, y, color);
 * }
 * 
 * hangulDisp hangul(drawPixel);
 * hangul.setFont(H01_FONT);  // wrapper에서 정의한 HangulFontInfo 변수명
 * hangul.print(10, 30, "안녕하세요 한글출력입니다", TFT_WHITE);
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
// 함수 구현
// ============================================================================

namespace HangulCore {

    // 초성 벌 선택 Lookup 테이블 (받침 없을 때)
    // 원본: { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 } (인덱스 1~21 사용)
    static const uint8_t CHO_BUL_NO_JONG[] = {
        0, 0, 0, 0, 0, 0, 0, 0,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (인덱스 0~7)
        1, 3, 3, 3, 1,           // ㅗㅘㅙㅚㅛ (인덱스 8~12)
        2, 4, 4, 4, 2,           // ㅜㅝㅞㅟㅠ (인덱스 13~17)
        1, 3, 0                  // ㅡㅢㅣ (인덱스 18~20)
    };
    
    // 초성 벌 선택 Lookup 테이블 (받침 있을 때)
    // 원본: { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 } (인덱스 1~21 사용)
    static const uint8_t CHO_BUL_WITH_JONG[] = {
        5, 5, 5, 5, 5, 5, 5, 5,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (인덱스 0~7)
        6, 7, 7, 7, 6,           // ㅗㅘㅙㅚㅛ (인덱스 8~12)
        6, 7, 7, 7, 6,           // ㅜㅝㅞㅟㅠ (인덱스 13~17)
        6, 7, 5                  // ㅡㅢㅣ (인덱스 18~20)
    };
    
    // 종성 벌 선택 Lookup 테이블 (중성에 따라)
    // 원본: { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 } (인덱스 1~21 사용)
    static const uint8_t JONG_BUL[] = {
        0, 2, 0, 2, 1, 2, 1, 2,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (인덱스 0~7)
        3, 0, 2, 1, 3,           // ㅗㅘㅙㅚㅛ (인덱스 8~12)
        3, 1, 2, 1, 3,           // ㅜㅝㅞㅟㅠ (인덱스 13~17)
        3, 1, 1                  // ㅡㅢㅣ (인덱스 18~20)
    };

    // 초성 벌 선택 로직
    inline uint8_t getChosungBul(uint8_t jung, bool hasJong) {
        if (jung >= 21) return 0;  // 범위 초과 방지
        return hasJong ? CHO_BUL_WITH_JONG[jung] : CHO_BUL_NO_JONG[jung];
    }
    
    // 중성 벌 선택 로직
    inline uint8_t getJungsungBul(uint8_t cho, bool hasJong) {
        // 초성이 ㄱ(인덱스 0) 또는 ㅋ(인덱스 15)인 경우
        if (cho == 0 || cho == 15) {
            return hasJong ? 2 : 0;  // 받침 있으면 3벌, 없으면 1벌
        } else {
            return hasJong ? 3 : 1;  // 받침 있으면 4벌, 없으면 2벌
        }
    }
    
    // 종성 벌 선택 로직
    inline uint8_t getJongsungBul(uint8_t jung) {
        if (jung >= 21) return 0;  // 범위 초과 방지
        return JONG_BUL[jung];
    }
    
    // UTF-8 한글 문자인지 확인
    inline bool isHangul(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
        // UTF-8 한글 범위: 0xEA~0xED
        if ((byte1 & 0xE0) != 0xE0) return false;
        
        uint16_t unicode = ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
        return (unicode >= 0xAC00 && unicode <= 0xD7A3);
    }
    
    // UTF-8 한글 문자 분해 (3바이트 -> 초중종성 + 벌)
    inline HangulComponents decompose(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
        HangulComponents comp = {0, 0, 0, 0, 0, 0};
        
        if (!isHangul(byte1, byte2, byte3)) {
            return comp;
        }
        
        // UTF-8 -> 유니코드 변환
        uint16_t unicode = ((byte1 & 0x0F) << 12) | ((byte2 & 0x3F) << 6) | (byte3 & 0x3F);
        uint16_t code = unicode - 0xAC00;
        
        // 초중종 분해
        comp.cho = code / 588;           // 초성 (0~18)
        comp.jung = (code % 588) / 28;   // 중성 (0~20)
        comp.jong = code % 28;           // 종성 (0~27, 0은 받침없음)
        
        // 벌 선택
        bool hasJong = (comp.jong > 0);
        comp.choBul = getChosungBul(comp.jung, hasJong);
        comp.jungBul = getJungsungBul(comp.cho, hasJong);
        if (hasJong) {
            comp.jongBul = getJongsungBul(comp.jung);
        }
        
        return comp;
    }
    
    // UTF-8 문자열에서 다음 한글 문자 위치 찾기
    inline const char* findNextHangul(const char* utf8String) {
        if (!utf8String) return nullptr;
        
        const char* p = utf8String;
        while (*p) {
            if ((*p & 0xE0) == 0xE0 && p[1] && p[2]) {
                if (isHangul((uint8_t)p[0], (uint8_t)p[1], (uint8_t)p[2])) {
                    return p;
                }
                p += 3;
            } else if ((*p & 0x80) == 0) {
                p++;
            } else if ((*p & 0xE0) == 0xC0) {
                p += 2;
            } else {
                p++;
            }
        }
        return nullptr;
    }

} // namespace HangulCore

// ============================================================================
// 통합 어댑터
// ============================================================================

// 픽셀 그리기 콜백 함수 타입
typedef void (*PixelDrawCallback)(int16_t x, int16_t y, uint16_t color);

class hangulDisp {
private:
    int16_t cursorX;
    int16_t cursorY;
    HangulSize textSize;
    HangulColor textColor;
    uint16_t textColorRaw;
    bool useRawColor;
    PixelDrawCallback drawPixelCallback;
    const uint8_t* choData;
    const uint8_t* jungData;
    const uint8_t* jongData;
    bool fontReady;

public:
    // 생성자 - 픽셀 그리기 콜백 함수 필요
        explicit hangulDisp(PixelDrawCallback callback)
        : cursorX(0), cursorY(0),
          textSize(HG_SIZE_NORMAL),
          textColor(HG_COLOR_BLACK),
                    textColorRaw(0),
                    useRawColor(false),
                    drawPixelCallback(callback),
                    choData(nullptr),
                    jungData(nullptr),
                    jongData(nullptr),
                    fontReady(false) {}

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
        useRawColor = false;
    }

    void setTextColor(uint16_t color) {
        textColorRaw = color;
        useRawColor = true;
    }

    int16_t getCursorX() const { return cursorX; }
    int16_t getCursorY() const { return cursorY; }

    void setFont(const HangulFontInfo& font) {
        choData = font.choData;
        jungData = font.jungData;
        jongData = font.jongData;
        fontReady = (choData && jungData && jongData);
    }

    // UTF-8 문자열 출력
    void print(const char* utf8Text) {
        if (!utf8Text || !drawPixelCallback || !fontReady) return;
        
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

    // 좌표와 색상을 포함한 출력 (16-bit 컬러 또는 흑백 코드)
    void print(int16_t x, int16_t y, const char* utf8Text, uint16_t color) {
        setCursor(x, y);
        setTextColor(color);
        print(utf8Text);
    }

    // 좌표와 색상을 포함한 출력 (HangulColor)
    void print(int16_t x, int16_t y, const char* utf8Text, HangulColor color) {
        setCursor(x, y);
        setTextColor(color);
        print(utf8Text);
    }

    // 단일 한글 문자 출력 (UTF-8 3바이트)
    void printHangulChar(uint8_t b1, uint8_t b2, uint8_t b3) {
        if (!fontReady) {
            return;
        }
        HangulComponents comp = HangulCore::decompose(b1, b2, b3);
        HangulGlyphSet glyphs = getGlyphSet(comp);
        drawCombinedGlyph(cursorX, cursorY, glyphs);
        advanceCursor();
    }

private:
    HangulGlyphSet getGlyphSet(const HangulComponents& comp) {
        HangulGlyphSet glyphs;
        glyphs.cho = getGlyphPointer(choData, comp.cho, comp.choBul, HANGUL_CHO_BUL);
        glyphs.jung = getGlyphPointer(jungData, comp.jung, comp.jungBul, HANGUL_JUNG_BUL);
        if (comp.jong > 0) {
            glyphs.jong = getGlyphPointer(jongData, comp.jong, comp.jongBul, HANGUL_JONG_BUL);
        } else {
            glyphs.jong = nullptr;
        }
        return glyphs;
    }

    const uint8_t* getGlyphPointer(const uint8_t* base, uint8_t index, uint8_t bul, uint8_t bulCount) const {
        if (!base) {
            return nullptr;
        }
        uint16_t glyphIndex = static_cast<uint16_t>(index) * bulCount + bul;
        uint32_t offset = static_cast<uint32_t>(glyphIndex) * HANGUL_BYTES_PER_GLYPH;
        return base + offset;
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
                    uint16_t color = resolveColor();
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

    uint16_t resolveColor() const {
        if (useRawColor) {
            return textColorRaw;
        }
        if (textColor == HG_COLOR_INVERT) {
            return 2;
        }
        return (textColor == HG_COLOR_WHITE) ? 1 : 0;
    }
};

#endif // HANGUL_DISP_H
