/*
 * AimHangul_U8g2.h
 * 
 * U8g2 라이브러리용 한글 디스플레이 어댑터
 */

#ifndef AIM_HANGUL_U8G2_H
#define AIM_HANGUL_U8G2_H

#include <U8g2lib.h>
#include "AimHangul_Base.h"

template<typename FontNamespace>
class AimHangul_U8g2 : public AimHangulBase<FontNamespace> {
private:
    U8G2* u8g2;

public:
    explicit AimHangul_U8g2(U8G2* display) : u8g2(display) {}

    void print(const char* utf8Text) override {
        // UTF-8 문자열 처리 로직 구현 예정
        // TODO: HangulCore를 사용하여 문자 분해 후 렌더링
    }

    void drawHangulChar(const HangulCore::HangulComponents& comp) override {
        // 초중종성 조합하여 한 글자 출력
        // TODO: 구현 예정
    }

    void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, 
                   int16_t w, int16_t h, uint16_t color) override {
        // U8g2의 drawXBM 또는 drawBitmap 사용
        // TODO: 구현 예정
    }
};

#endif // AIM_HANGUL_U8G2_H