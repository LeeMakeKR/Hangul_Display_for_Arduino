/*
 * AimHangul_Base.h
 * 
 * 한글 디스플레이 어댑터 베이스 클래스
 * 모든 디스플레이 라이브러리 어댑터가 상속받는 공통 인터페이스
 */

#ifndef AIM_HANGUL_BASE_H
#define AIM_HANGUL_BASE_H

#include "../core/HangulCore.h"
#include "../core/HangulTypes.h"

template<typename FontNamespace>
class AimHangulBase {
protected:
    int16_t cursorX = 0;
    int16_t cursorY = 0;
    HangulSize textSize = HG_SIZE_NORMAL;
    HangulColor textColor = HG_COLOR_BLACK;

public:
    // 커서 위치 설정
    void setCursor(int16_t x, int16_t y) {
        cursorX = x;
        cursorY = y;
    }

    // 텍스트 크기 설정
    void setTextSize(HangulSize size) {
        textSize = size;
    }

    // 텍스트 색상 설정
    void setTextColor(HangulColor color) {
        textColor = color;
    }

    // 커서 위치 얻기
    int16_t getCursorX() const { return cursorX; }
    int16_t getCursorY() const { return cursorY; }

    // 폰트 정보 얻기
    const HangulFontInfo* getFontInfo() const {
        return &FontNamespace::FONT_INFO;
    }

    // 순수 가상 함수들 (파생 클래스에서 구현)
    virtual void print(const char* utf8Text) = 0;
    virtual void drawHangulChar(const HangulCore::HangulComponents& comp) = 0;
    virtual void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, 
                          int16_t w, int16_t h, uint16_t color) = 0;

protected:
    // 글리프 데이터 가져오기
    const uint8_t* getChoGlyph(uint8_t cho, uint8_t bul) const {
        return &FontNamespace::CHO_DATA[cho][bul][0];
    }

    const uint8_t* getJungGlyph(uint8_t jung, uint8_t bul) const {
        return &FontNamespace::JUNG_DATA[jung][bul][0];
    }

    const uint8_t* getJongGlyph(uint8_t jong, uint8_t bul) const {
        return &FontNamespace::JONG_DATA[jong][bul][0];
    }

    // 다음 문자 위치로 커서 이동
    void advanceCursor() {
        cursorX += HANGUL_GLYPH_WIDTH * (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4 ? 2 : 1);
    }
};

#endif // AIM_HANGUL_BASE_H