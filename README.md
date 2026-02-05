# hangulDisp

다양한 아두이노 디스플레이 라이브러리에서 한글을 표시하기 위한 모듈화된 폰트 라이브러리

## 프로젝트 목적

U8g2, TFT_eSPI, GxEPD2, Adafruit GFX 등 아두이노의 다양한 디스플레이 라이브러리(TFT-LCD, OLED, E-Paper 등)에서 EasyView 한글 폰트를 사용할 수 있도록 하는 통합 라이브러리입니다.

## 주요 기능

- **통합 어댑터**: 모든 디스플레이에서 사용 가능한 단일 어댑터
- **다양한 디스플레이 지원**: TFT-LCD, OLED, E-Paper 등 (U8g2, TFT_eSPI, GxEPD2, M5Stack 등)
- **EasyView 폰트**: 옛한글 텍스트 뷰어의 고품질 한글 폰트 활용
- **템플릿 기반**: 폰트 변경 시 컴파일 타임에 최적화
- **메모리 효율**: PROGMEM 사용으로 플래시 메모리 활용
- **쉽고 유연한 사용**: 픽셀 그리기 콜백만 제공하면 모든 라이브러리에서 사용 가능

## 사용 방법

### 기본 사용법 (통합 어댑터)

hangulDisp은 통합 어댑터를 사용하여 모든 디스플레이 라이브러리에서 동일한 방식으로 한글을 출력할 수 있습니다.
픽셀 그리기 콜백 함수만 제공하면 됩니다.

```cpp
#include "hangulDisp.h"
#include "fonts/hangul/H01_Font.h"

// 디스플레이 라이브러리 초기화 (예: TFT_eSPI)
TFT_eSPI tft;

// 픽셀 그리기 콜백 함수
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    tft.drawPixel(x, y, color ? TFT_WHITE : TFT_BLACK);
}

// hangulDisp 인스턴스 생성
hangulDisp<Font_H01> hangul(drawPixel);

void setup() {
    tft.init();
    tft.fillScreen(TFT_BLACK);
    
    // 한글 출력
    hangul.setCursor(10, 30);
    hangul.setTextColor(HG_COLOR_WHITE);
    hangul.print("안녕하세요 아두이노!");
}
```

### U8g2 사용례 (OLED)

```cpp
#include <U8g2lib.h>
#include "hangulDisp.h"
#include "fonts/hangul/Apple00_Font.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// U8g2 픽셀 그리기 콜백
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    u8g2.setDrawColor(color);
    u8g2.drawPixel(x, y);
}

hangulDisp<Font_Apple00> hangul(drawPixel);

void setup() {
    u8g2.begin();
    u8g2.clearBuffer();
    hangul.setCursor(0, 15);
    hangul.print("한글 테스트");
    u8g2.sendBuffer();
}
```

### GxEPD2 사용례 (E-Paper)

```cpp
#include <GxEPD2_BW.h>
#include "hangulDisp.h"
#include "fonts/hangul/H01_Font.h"

GxEPD2_BW<GxEPD2_154, GxEPD2_154::HEIGHT> display(
    GxEPD2_154(SS, 8, 9, 7)
);

// GxEPD2 픽셀 그리기 콜백
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    display.drawPixel(x, y, color ? GxEPD_WHITE : GxEPD_BLACK);
}

hangulDisp<Font_H01> hangul(drawPixel);

void setup() {
    display.init();
    display.setFullWindow();
    display.firstPage();
    
    do {
        display.fillScreen(GxEPD_WHITE);
        hangul.setCursor(10, 30);
        hangul.print("전자종이 한글");
    } while (display.nextPage());
}
```

## API 참조

### 공통 메서드

```cpp
// 커서 위치 설정
hangul.setCursor(int16_t x, int16_t y);

// 텍스트 크기 설정  
hangul.setTextSize(HangulSize size);
// HG_SIZE_NORMAL (16x16)
// HG_SIZE_H2 (32x16, 가로 2배)
// HG_SIZE_V2 (16x32, 세로 2배)  
// HG_SIZE_X4 (32x32, 4배)

// 텍스트 색상 설정
hangul.setTextColor(HangulColor color);

// UTF-8 문자열 출력
hangul.print(const char* utf8Text);
```

## 폰트 정보

이 라이브러리는 옛한글 텍스트 뷰어(EasyView)의 조합형 한글 폰트를 활용합니다.

### 폰트 구조
- **크기**: 16×16 픽셀
- **인코딩**: EasyView 조합형 한글
- **구성**: 초성 8벌 + 중성 4벌 + 종성 4벌
- **총 글리프**: 360개 (20×8 + 22×4 + 28×4)

### 사용 가능한 폰트
- **H01**: 기본 고딕체
- **Apple00**: 애플고딕 스타일
- **Goth00**: 고딕체
- **기타**: EasyView-font/ko/ 폴더의 .han 파일들

### 폰트 추가 방법
1. `.han` 파일을 `tools/han_to_h.py`로 변환
2. 생성된 `.h` 파일을 `fonts/hangul/` 폴더에 배치
3. 네임스페이스 이름으로 사용 (예: `Font_CustomFont`)

## 도구

### 폰트 변환 도구
```bash
# .han 파일을 .h 헤더로 변환
python tools/han_to_h.py EasyView-font/ko/H01_kr.han

# 모든 폰트 렌더링 (테스트용)
python tools/render_hangul.py
```

## 개발 로드맵

- [x] 기본 구조 설계 및 구현
- [x] 통합 헤더 구성 (타입 + 로직 + 어댑터)
- [x] 통합 어댑터 구현 (hangulDisp)
- [ ] 폰트 변환 도구 개선
- [ ] ASCII 폰트 지원 추가
- [ ] 성능 최적화
- [ ] 문서화 완료

## 기여 방법

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 `LICENSE` 파일을 참조하세요.

## 크레딧

- EasyView 옛한글 텍스트 뷰어의 한글 폰트 활용
- U8g2, TFT_eSPI, GxEPD2 등 오픈소스 디스플레이 라이브러리들

## 라이브러리 구조

```
hangulDisp.h                       # 통합 헤더 (타입 + 로직 + 어댑터)

fonts/                             # 폰트 데이터
├── ASCFont.h                      # ASCII 폰트
├── KSFont.h                       # 기존 KS 폰트
└── hangul/                        # 한글 폰트들
    ├── H01_Font.h                 # H01 폰트 데이터
    ├── Apple00_Font.h             # Apple00 폰트 데이터
    └── ...                        # 기타 폰트들

EasyView-font/                     # 원본 폰트 파일
├── ko/                            # 한글 폰트 (.han 파일)
└── en/                            # 영문 폰트 (.eng 파일)

tools/                             # 폰트 변환 도구
├── han_to_h.py                    # .han → .h 변환 도구
└── render_hangul.py               # 폰트 렌더링 도구
```

## 설계 원칙

### 1. **통합 어댑터**
- 단일 hangulDisp 클래스로 모든 디스플레이 지원
- 픽셀 그리기 콜백만 제공하면 됨
- 디스플레이 독립적 구현

### 2. **모듈 통합**
- hangulDisp.h에 타입 정의와 로직 통합
- UTF-8 → 초중종성 분해, 벌 선택
- 폰트 데이터는 네임스페이스로 분리 (Font_H01, Font_Apple00 등)

### 3. **템플릿 기반 설계**
```cpp
// 폰트 선택은 템플릿 매개변수로
hangulDisp<Font_H01> hangul(drawPixel);      // H01 폰트
hangulDisp<Font_Apple00> hangul(drawPixel);  // Apple00 폰트
```

### 4. **메모리 효율**
- 사용하는 폰트만 링크 (링크 타임 최적화)
- PROGMEM으로 플래시 메모리 활용
- 필요한 글리프만 조합하여 렌더링

## 참고 자료

- [GxEPD2 한글 표시 방법](https://blog.naver.com/sanguru/221854830624)
- [전자책 프로젝트 - 한글 폰트](https://blog.naver.com/gilchida/222927710968)
- [옛한글 텍스트 뷰어 EasyView](EasyView-3.0.b2)

