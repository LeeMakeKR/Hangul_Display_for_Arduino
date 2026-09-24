/*
 * HangulGxEPD2Adapter.h - GxEPD2(전자종이) 어댑터
 *
 * 이 헤더는 GxEPD2 헤더를 포함하지 않는다. 템플릿이므로 드라이버는
 * 스케치에서 포함하고 타입만 넘기면 된다. 코어의 필수 의존성이 아니다.
 *
 * 사용법:
 *   #include <GxEPD2_BW.h>
 *   #include <HangulDisp.h>
 *   #include <adapters/HangulGxEPD2Adapter.h>
 *   #include <fonts/H01_kr.h>
 *
 *   GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(...);
 *   typedef HangulGxEPD2Adapter<decltype(display)> Adapter;
 *   hangulDisp hangul(Adapter::pixel);
 *
 *   Adapter::attach(display, hangul);
 *   hangul.setFont(H01_kr);
 *
 * 페이지 반복 주의
 *   GxEPD2는 같은 내용을 여러 번 그린다(paged drawing). 커서는 코어의 상태이므로
 *   페이지마다 같은 시작 좌표를 다시 지정해야 한다. 그렇지 않으면 두 번째
 *   페이지부터 커서가 이어서 전진해 글자가 밀린다.
 *
 *     display.setFullWindow();
 *     display.firstPage();
 *     do {
 *         display.fillScreen(GxEPD_WHITE);   // 배경 지우기: 호출자 책임
 *         hangul.setCursor(10, 20);           // 페이지마다 같은 시작 좌표
 *         hangul.print("안녕하세요");
 *     } while (display.nextPage());
 *
 * 역할 분담
 *   - 코어    : 픽셀 생성
 *   - 어댑터  : 클리핑, 논리 색상 -> 드라이버 색상 변환
 *   - 호출자  : 페이지 루프, 배경 지우기, 화면 갱신
 *
 * 색상
 *   - 원시 색상: GxEPD_BLACK/GxEPD_WHITE 같은 값을 그대로 전달한다.
 *   - 논리 색상: BLACK -> blackValue, WHITE -> whiteValue (기본 0x0000/0xFFFF).
 *   - 반전(INVERT): 전자종이 BW 패널에서는 지원하지 않는다.
 *     setLogicalColor()가 false를 돌려주며 색상 상태를 바꾸지 않는다.
 */

#ifndef HANGUL_GXEPD2_ADAPTER_H
#define HANGUL_GXEPD2_ADAPTER_H

#include "../HangulDisp.h"

template <class Display>
class HangulGxEPD2Adapter {
public:
    /**
     * 어댑터를 장치와 코어에 연결한다.
     * @param blackValue 드라이버의 검정 값 (GxEPD_BLACK)
     * @param whiteValue 드라이버의 흰색 값 (GxEPD_WHITE)
     */
    static void attach(Display& display, hangulDisp& hangul,
                       uint16_t blackValue = 0x0000, uint16_t whiteValue = 0xFFFF) {
        s_display = &display;
        s_black = blackValue;
        s_white = whiteValue;
        hangul.setColorResolver(resolveLogical);
        hangul.setTextColor(HG_COLOR_BLACK);
    }

    static void detach() {
        s_display = nullptr;
    }

    /**
     * 픽셀 그리기 콜백.
     * 화면 밖 좌표는 여기서 버린다(클리핑은 어댑터 책임).
     * 색상 값은 변환 없이 드라이버로 그대로 넘긴다.
     */
    static void pixel(int16_t x, int16_t y, uint16_t color) {
        if (!s_display) return;
        if (x < 0 || y < 0) return;
        if (x >= s_display->width() || y >= s_display->height()) return;

        s_display->drawPixel(x, y, color);
    }

    // 논리 색상 -> 드라이버 색상
    static uint16_t resolveLogical(HangulColor logical) {
        switch (logical) {
            case HG_COLOR_BLACK:  return s_black;
            case HG_COLOR_WHITE:  return s_white;
            case HG_COLOR_INVERT: return s_black;   // 도달하지 않는다 (아래 참조)
        }
        return s_black;
    }

    // 이 어댑터가 반전을 지원하는지. BW 전자종이에서는 지원하지 않는다.
    static bool supportsInvert() { return false; }

    /**
     * 논리 색상 지정.
     * @return 지원하지 않는 색상이면 false. 이때 색상 상태는 바뀌지 않는다.
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
    static uint16_t s_black;
    static uint16_t s_white;
};

template <class Display> Display* HangulGxEPD2Adapter<Display>::s_display = nullptr;
template <class Display> uint16_t HangulGxEPD2Adapter<Display>::s_black = 0x0000;
template <class Display> uint16_t HangulGxEPD2Adapter<Display>::s_white = 0xFFFF;

#endif  // HANGUL_GXEPD2_ADAPTER_H
