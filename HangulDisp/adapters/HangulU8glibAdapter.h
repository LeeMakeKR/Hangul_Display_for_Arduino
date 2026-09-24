/*
 * HangulU8glibAdapter.h - U8glib(v1) 어댑터
 *
 * 이 헤더는 U8glib 헤더를 포함하지 않는다. 템플릿이므로 드라이버는
 * 스케치에서 포함하고 타입만 넘기면 된다. 코어의 필수 의존성이 아니다.
 *
 * 사용법:
 *   #include <U8glib.h>
 *   #include <HangulDisp.h>
 *   #include <adapters/HangulU8glibAdapter.h>
 *   #include <fonts/H01_kr.h>
 *
 *   U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
 *   typedef HangulU8glibAdapter<U8GLIB_SSD1306_128X64> Adapter;
 *   hangulDisp hangul(Adapter::pixel);
 *
 *   void setup() {
 *       Adapter::attach(u8g, hangul);
 *       hangul.setFont(H01_kr);
 *   }
 *
 * 역할 분담
 *   - 코어    : 픽셀 생성 (어떤 좌표를 켜야 하는지)
 *   - 어댑터  : 클리핑, 논리 색상 -> 색 인덱스 변환
 *   - 호출자  : 그림 루프(firstPage/nextPage), 배경 지우기, 화면 갱신
 *
 * 색상
 *   - 원시 색상: 값을 그대로 색 인덱스로 쓴다 (모노는 0 또는 1).
 *   - 논리 색상: BLACK -> 인덱스 0, WHITE -> 인덱스 1.
 *   - 반전(INVERT): U8glib 모노 장치에서는 지원하지 않는다.
 *     setLogicalColor()가 false를 돌려주며 색상 상태를 바꾸지 않는다.
 */

#ifndef HANGUL_U8GLIB_ADAPTER_H
#define HANGUL_U8GLIB_ADAPTER_H

#include "../HangulDisp.h"

template <class Display>
class HangulU8glibAdapter {
public:
    // 어댑터를 장치와 코어에 연결한다.
    static void attach(Display& display, hangulDisp& hangul) {
        s_display = &display;
        hangul.setColorResolver(resolveLogical);
        hangul.setTextColor(HG_COLOR_WHITE);   // 모노 기본: 켜짐(인덱스 1)
    }

    static void detach() {
        s_display = nullptr;
    }

    /**
     * 픽셀 그리기 콜백.
     * 화면 밖 좌표는 여기서 버린다(클리핑은 어댑터 책임).
     * U8glib의 좌표 타입은 부호 없는 값이라 음수 좌표를 그대로 넘기면
     * 화면 반대편에 찍히므로 반드시 걸러야 한다.
     */
    static void pixel(int16_t x, int16_t y, uint16_t color) {
        if (!s_display) return;
        if (x < 0 || y < 0) return;
        if (x >= (int16_t)s_display->getWidth()) return;
        if (y >= (int16_t)s_display->getHeight()) return;

        s_display->setColorIndex((uint8_t)color);
        s_display->drawPixel((uint8_t)x, (uint8_t)y);
    }

    // 논리 색상 -> U8glib 색 인덱스
    static uint16_t resolveLogical(HangulColor logical) {
        switch (logical) {
            case HG_COLOR_BLACK:  return 0;   // 지움
            case HG_COLOR_WHITE:  return 1;   // 켜짐
            case HG_COLOR_INVERT: return 1;   // 도달하지 않는다 (아래 참조)
        }
        return 1;
    }

    // 이 어댑터가 반전을 지원하는지. 모노 장치에서는 지원하지 않는다.
    static bool supportsInvert() { return false; }

    /**
     * 논리 색상 지정.
     * @return 지원하지 않는 색상이면 false. 이때 색상 상태는 바뀌지 않는다.
     *         (숫자 2를 그냥 넘겨 반전을 지원하는 척하지 않는다.)
     */
    static bool setLogicalColor(hangulDisp& hangul, HangulColor logical) {
        if (logical == HG_COLOR_INVERT && !supportsInvert()) {
            return false;
        }
        hangul.setTextColor(logical);
        return true;
    }

private:
    static Display* s_display;
};

template <class Display>
Display* HangulU8glibAdapter<Display>::s_display = nullptr;

#endif  // HANGUL_U8GLIB_ADAPTER_H
