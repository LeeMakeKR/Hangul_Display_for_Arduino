/*
 * GxEPD2_Hangul - GxEPD2(전자종이)에서 한글 출력
 *
 * 대상 보드      : ESP32 DevKit (esp32dev) / Arduino Mega 2560
 * 대상 디스플레이: 1.54" 200x200 흑백 전자종이 (GxEPD2_154_D67, SSD1681)
 * 라이브러리     : GxEPD2 1.6.x (zinggjm/GxEPD2), Adafruit GFX Library 1.11.x
 *
 * 배선
 *   ESP32 : CS=5,  DC=17, RST=16, BUSY=4,  CLK=18, DIN(MOSI)=23, 3.3V, GND
 *   Mega  : CS=53, DC=8,  RST=9,  BUSY=7,  CLK=52, DIN(MOSI)=51, 3.3V, GND
 *   다른 패널을 쓰려면 아래 display 선언의 클래스와 핀만 바꾼다.
 *
 * 확인 내용
 *   - 페이지 반복(paged drawing)마다 같은 시작 좌표를 다시 지정한다
 *   - 같은 자리 글자 교체 시 배경 지우기(fillScreen)는 호출자 책임이다
 *   - 논리 색상은 어댑터가 GxEPD_BLACK/GxEPD_WHITE로 변환한다
 *   - 원시 색상은 변환 없이 그대로 전달된다
 *   - 반전(INVERT)은 BW 패널에서 지원하지 않으며 false로 보고된다
 */

#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>

#include <HangulDisp.h>
#include <adapters/HangulGxEPD2Adapter.h>
#include <fonts/H01_kr.h>

// 보드별 핀 배치
#if defined(ARDUINO_ARCH_AVR)
#define HG_EPD_CS   53
#define HG_EPD_DC    8
#define HG_EPD_RST   9
#define HG_EPD_BUSY  7
#else
#define HG_EPD_CS    5
#define HG_EPD_DC   17
#define HG_EPD_RST  16
#define HG_EPD_BUSY  4
#endif

// 1.54" 200x200 흑백 패널
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
    GxEPD2_154_D67(/*CS=*/ HG_EPD_CS, /*DC=*/ HG_EPD_DC,
                   /*RST=*/ HG_EPD_RST, /*BUSY=*/ HG_EPD_BUSY));

typedef HangulGxEPD2Adapter<GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> > Adapter;

hangulDisp hangul(Adapter::pixel);

// 화면 전체를 한 번 그린다. 페이지 루프 안에서 좌표를 매번 다시 잡는다.
static void drawScreen(const char* line1, const char* line2) {
    display.setFullWindow();
    display.firstPage();
    do {
        // 배경 지우기: 호출자 책임. 이 줄이 없으면 이전 내용이 남는다.
        display.fillScreen(GxEPD_WHITE);

        // 페이지마다 같은 시작 좌표. 이 줄이 없으면 두 번째 페이지부터
        // 커서가 이어서 전진해 글자가 오른쪽으로 밀린다.
        hangul.setCursor(10, 20);
        hangul.setTextSize(HG_SIZE_NORMAL);
        hangul.print(line1);

        hangul.setCursor(10, 50);
        hangul.setTextSize(HG_SIZE_X4);   // 가로세로 2배
        hangul.print(line2);
        hangul.setTextSize(HG_SIZE_NORMAL);
    } while (display.nextPage());
}

void setup() {
    Serial.begin(115200);

    display.init(115200);
    display.setRotation(0);

    Adapter::attach(display, hangul, GxEPD_BLACK, GxEPD_WHITE);
    hangul.setFont(H01_kr);

    if (!hangul.isFontReady()) {
        Serial.println(F("폰트 설정 실패"));
        return;
    }

    // 논리 색상 경로: 어댑터가 드라이버 값으로 변환한다
    Adapter::setLogicalColor(hangul, HG_COLOR_BLACK);
    drawScreen("안녕하세요", "한글");

    delay(3000);

    // 같은 자리에 다른 글자를 그린다 (잔상·좌표 누적 확인)
    drawScreen("반갑습니다", "출력");

    delay(3000);

    // 원시 색상 경로: 값이 변환 없이 드라이버로 전달된다
    hangul.setTextColor((uint16_t)GxEPD_BLACK);
    drawScreen("원시색상", "시험");

    // 반전은 이 패널에서 지원하지 않는다
    if (!Adapter::setLogicalColor(hangul, HG_COLOR_INVERT)) {
        Serial.println(F("이 어댑터는 반전(INVERT)을 지원하지 않습니다"));
    }

    display.hibernate();
}

void loop() {
    // 전자종이는 갱신 비용이 크므로 setup()에서만 그린다
}
