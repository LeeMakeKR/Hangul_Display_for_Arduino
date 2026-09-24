# Hangul Display for Arduino

다양한 아두이노 디스플레이 라이브러리에서 한글을 표시하기 위한 모듈화된 폰트 라이브러리

## 프로젝트 목적

U8g2, TFT_eSPI, GxEPD2, Adafruit GFX 등 아두이노의 다양한 디스플레이 라이브러리(TFT-LCD, OLED, E-Paper 등)에서 EasyView 한글 폰트를 사용할 수 있도록 하는 통합 라이브러리입니다.

## 주요 기능

- **통합 어댑터**: 모든 디스플레이에서 사용 가능한 단일 어댑터
- **다양한 디스플레이 지원**: TFT-LCD, OLED, E-Paper 등 (U8g2, TFT_eSPI, GxEPD2, M5Stack 등)
- **EasyView 폰트**: 옛한글 텍스트 뷰어의 고품질 한글 폰트 활용 (135개 폰트 지원)
- **setFont 기반**: 런타임에 폰트 선택 가능
- **메모리 효율**: PROGMEM 사용으로 플래시 메모리 활용
- **쉽고 유연한 사용**: 픽셀 그리기 콜백만 제공하면 모든 라이브러리에서 사용 가능

## 빠른 시작

### 설치

`HangulDisp/` 폴더를 통째로 아두이노 라이브러리 폴더에 복사합니다.
(Windows: `문서\Arduino\libraries\HangulDisp\`)

### 기본 사용 예제

```cpp
#include <TFT_eSPI.h>
#include <HangulDisp.h>
#include <fonts/H01_kr.h>   // 사용할 폰트 헤더 (변수명 = 파일명)

TFT_eSPI tft;  // 또는 U8glib, U8g2, GxEPD2 등

// 픽셀 그리기 콜백 함수
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    tft.drawPixel(x, y, color);
}

// hangulDisp 인스턴스 생성
hangulDisp hangul(drawPixel);

void setup() {
    tft.init();
    tft.fillScreen(TFT_BLACK);

    hangul.setFont(H01_kr);           // 폰트 설정
    if (!hangul.isFontReady()) return;  // 16x16이 아니거나 포인터가 비면 거부된다

    hangul.print(10, 30, "안녕하세요", (uint16_t)TFT_WHITE);
}

void loop() {}
```

### 이 라이브러리가 그리는 것과 그리지 않는 것

| 입력 | 처리 |
|------|------|
| 현대 한글 U+AC00~U+D7A3 | 합성해서 그리고 커서를 전진 |
| 공백 | 그리지 않고 커서만 `16 x 가로배율` 전진 |
| 그 외 ASCII (영문·숫자·CR·LF) | 그리지 않고 커서도 움직이지 않음 |
| 한자·자모·이모지 등 미지원 문자 | 그리지 않고 커서도 움직이지 않음 |
| 잘못된 UTF-8 | 1바이트 버리고 다음부터 다시 읽음 |

**영문·숫자·개행은 이 라이브러리가 처리하지 않습니다.** 쓰던 디스플레이
라이브러리의 폰트로 그리면 됩니다. 자세한 내용은 [사용방법.md](사용방법.md) 참조.

### 어댑터와 예제

| 예제 | 내용 |
|------|------|
| [SelfTest](HangulDisp/examples/SelfTest/) | 디스플레이 없이 도는 자가진단 (시리얼 출력) |
| [U8glib_Hangul](HangulDisp/examples/U8glib_Hangul/) | Uno + SSD1306 128x64 OLED |
| [GxEPD2_Hangul](HangulDisp/examples/GxEPD2_Hangul/) | ESP32/Mega + 1.54" 200x200 전자종이 |

`HangulDisp/adapters/` 의 어댑터는 클리핑과 색상 변환을 맡습니다. 드라이버 헤더를
포함하지 않는 템플릿이라 코어의 필수 의존성이 되지 않습니다.

## 설계 원칙

### 1. **통합 어댑터**
- 단일 hangulDisp 클래스로 모든 디스플레이 지원
- 픽셀 그리기 콜백만 제공하면 됨
- 디스플레이 독립적 구현

### 2. **모듈 통합**
- HangulDisp.h에 타입 정의와 로직 통합
- UTF-8 → 초중종성 분해, 벌 선택
- 폰트 데이터는 별도 헤더로 분리 (`fonts/H01_kr.h` 등)


### 3. **메모리 효율**
- 사용하는 폰트만 링크 (링크 타임 최적화)
- PROGMEM으로 플래시 메모리 활용
- 필요한 글리프만 조합하여 렌더링

## 폰트 데이터 구조

각 한글 폰트는 16×16 픽셀 비트맵으로 구성되며, 총 360개의 글리프를 포함합니다:

| 구성 | 개수 | 벌 수 | 총 글리프 | 크기 |
|------|------|-------|-----------|------|
| 초성 | 20개 | 8벌 | 160개 | 5,120 바이트 |
| 중성 | 22개 | 4벌 | 88개 | 2,816 바이트 |
| 종성 | 28개 | 4벌 | 112개 | 3,584 바이트 |
| **합계** | **70개** | - | **360개** | **11,520 바이트** |

### 렌더링 흐름

```
UTF-8 문자열 (3바이트)
    ↓
유니코드 변환 (U+AC00~U+D7A3)
    ↓
초중종 분해 (수학적 계산)
    ↓
벌 선택 (조합 규칙)
    ↓
글리프 포인터 계산
    ↓
비트맵 OR 합성
    ↓
픽셀 출력
```

## 지원 폰트

총 **135개**의 EasyView 한글 폰트를 지원합니다:
- H01_kr ~ H07_kr (기본 폰트)
- Apple_kr, Goth_kr, Roman_kr 등 (다양한 스타일)
- 모든 폰트는 `HangulDisp/fonts/` 폴더에 있습니다

## 📚 기술 문서

### 상세 명세서

프로젝트의 기술 명세는 다음 문서에서 확인할 수 있습니다:

#### 1. [폰트 헤더 파일 형식 명세](FONT_HEADER_SPECIFICATION.md)
폰트 `.h` 헤더 파일의 구조와 형식을 상세히 설명합니다.
- 비트맵 데이터 레이아웃 (11,520 바이트)
- HangulFontInfo 구조체 명세
- 글리프 비트맵 형식 (16×16 픽셀, 32 바이트)
- 자모 인덱스 규칙 및 벌 시스템
- 메모리 최적화 및 변환 도구

#### 2. 렌더링 규칙
렌더링 파이프라인과 문자 처리 계약은 코어 헤더
[HangulDisp/HangulDisp.h](HangulDisp/HangulDisp.h) 상단 주석에 표로 정리되어 있습니다.
- UTF-8 디코딩 (1~4바이트, 최단 인코딩, surrogate, 상한 검증)
- 한글 분해 알고리즘 (초중종 분리)
- 벌 선택 규칙 (조합 규칙)
- 글리프 포인터 계산 및 행 단위 비트맵 OR 합성
- 색상·클리핑·페이지 루프의 역할 분담

#### 3. [개발로그](개발로그.md)
수정 이력, 측정 수치, 시험 구성, 미수행 항목을 기록합니다.

### 문서 업데이트 계획

폰트 변환 방식이 변경될 경우, 다음 문서에 변경 사항을 반드시 반영합니다:

- README.md: 지원 폰트/변환 도구/사용 흐름 업데이트
- FONT_HEADER_SPECIFICATION.md: 신규 헤더 포맷 또는 변형 규격 추가
- HangulDisp/HangulDisp.h 상단 주석: 벌 규칙, 렌더링 경로, 문자 처리 계약 변화 반영
- 개발로그.md: 변경 이유와 측정 수치 기록

### 빠른 참조

**폰트 구조:**
```
총 11,520 바이트
├─ 초성: 0~5119 (5,120 B)
├─ 중성: 5120~7935 (2,816 B)
└─ 종성: 7936~11519 (3,584 B)
```

**각 글리프:** 16×16 픽셀 = 32 바이트 (MSB First)

## 도구

### 1. EasyView 폰트 변환 도구

`tools/easyview-font-converter/` - EasyView 폰트를 Arduino 헤더로 변환

- **han_to_h.py**: 단일 `.han` 파일을 `.h` 헤더로 변환
- **convert_all.py**: 전체 폰트 일괄 변환
- **README_han_to_h.md**: 변환 도구 상세 사용법

```bash
# 단일 폰트 변환
python tools/easyview-font-converter/han_to_h.py input.han output.h

# 전체 폰트 변환
cd tools/easyview-font-converter
python convert_all.py
```

### 2. TTF 폰트 변환 도구

`tools/ttf-font-converter/` - TrueType 폰트를 조합형 한글 비트맵으로 변환

이 폴더에는 변환 스크립트가 아니라 **Einstein Bacon Machine** (외부 GUI 프로그램)
사용 절차가 정리되어 있습니다. 자세한 내용은
[TTF 변환 도구 README](tools/ttf-font-converter/README.md)를 참조하세요.

### 3. 시험

```bash
# 변환기·폰트 헤더·코어 시험 전체
python -B tests/run_all_tests.py

# 예제·용량 빌드 (PlatformIO)
pio run -d tests/pio -e selftest_uno
pio run -d tests/pio -e u8glib_uno
pio run -d tests/pio -e gxepd2_mega
```

## 참고 자료

- [GxEPD2 한글 표시 방법](https://blog.naver.com/sanguru/221854830624)
- [전자책 프로젝트 - 한글 폰트](https://blog.naver.com/gilchida/222927710968)
- [마이크로파이썬 한글 사용하기](https://blog.naver.com/gilchida/224073231896)
- 옛한글 텍스트 뷰어 EasyView (폰트 원본 출처)
