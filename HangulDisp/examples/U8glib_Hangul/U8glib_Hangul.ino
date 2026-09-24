/*
 * U8glib_Hangul - U8glib(v1)에서 한글 출력
 *
 * 대상 보드      : Arduino Uno / Nano (ATmega328P)
 * 대상 디스플레이: SSD1306 128x64 I2C OLED
 * 라이브러리     : U8glib 1.19.1 (olikraus/U8glib)
 *
 * U8g2가 아니라 U8glib(v1)용이다. 두 라이브러리는 API가 다르다.
 *
 * 배선 (I2C)
 *   OLED SDA -> A4, OLED SCL -> A5, VCC 3.3~5V, GND
 *
 * 확인 내용
 *   - 그림 루프(firstPage/nextPage)를 여러 번 돌아도 좌표가 누적되지 않는다
 *     (페이지마다 setCursor를 다시 부른다)
 *   - 같은 자리 글자 교체 시 잔상이 남지 않는다 (배경 지우기는 호출자 책임)
 *   - 한글은 HangulDisp, 영문·숫자는 U8glib 자체 폰트로 그린다
 */

#include <U8glib.h>

#include <HangulDisp.h>
#include <adapters/HangulU8glibAdapter.h>
#include <fonts/H01_kr.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

typedef HangulU8glibAdapter<U8GLIB_SSD1306_128X64> Adapter;

hangulDisp hangul(Adapter::pixel);

// 2초마다 바꿔 가며 같은 자리에 그린다 (잔상 확인용)
static const char* const MESSAGES[] = { "한글출력", "안녕하세요", "가각힣" };
static uint8_t messageIndex = 0;

static void drawPage() {
    // 페이지마다 같은 시작 좌표를 다시 지정한다.
    // 이 줄이 없으면 두 번째 페이지부터 커서가 이어서 전진해 글자가 밀린다.
    hangul.setCursor(0, 0);
    hangul.setTextSize(HG_SIZE_NORMAL);
    hangul.print(MESSAGES[messageIndex]);

    hangul.setCursor(0, 18);
    hangul.setTextSize(HG_SIZE_H2);   // 가로 2배
    hangul.print("한글");
    hangul.setTextSize(HG_SIZE_NORMAL);

    // 영문·숫자는 이 라이브러리가 담당하지 않는다. U8glib 폰트를 쓴다.
    u8g.setColorIndex(1);
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 60, "ASCII by U8glib");
}

void setup() {
    Adapter::attach(u8g, hangul);
    hangul.setFont(H01_kr);

    // 반전은 모노 OLED에서 지원하지 않는다. 명시적으로 확인할 수 있다.
    // Adapter::setLogicalColor(hangul, HG_COLOR_INVERT) == false
    Adapter::setLogicalColor(hangul, HG_COLOR_WHITE);
}

void loop() {
    // U8glib 그림 루프. 페이지마다 drawPage()가 통째로 다시 실행된다.
    // 배경 지우기는 U8glib가 페이지 버퍼를 비우면서 처리한다.
    u8g.firstPage();
    do {
        drawPage();
    } while (u8g.nextPage());

    delay(2000);
    messageIndex = (uint8_t)((messageIndex + 1) % 3);
}
