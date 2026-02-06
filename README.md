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

### 기본 사용 예제

```cpp
#include "hangulDisp.h"
#include "H01_kr.h"  // 사용할 폰트 헤더

TFT_eSPI tft;  // 또는 U8G2, GxEPD2 등

// 픽셀 그리기 콜백 함수
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    tft.drawPixel(x, y, color);
}

// hangulDisp 인스턴스 생성
hangulDisp hangul(drawPixel);

void setup() {
    tft.init();
    hangul.setFont(H01_kr);  // 폰트 설정
}

void loop() {
    hangul.print(10, 30, "안녕하세요 한글출력입니다", TFT_WHITE);
}
```

## 설계 원칙

### 1. **통합 어댑터**
- 단일 hangulDisp 클래스로 모든 디스플레이 지원
- 픽셀 그리기 콜백만 제공하면 됨
- 디스플레이 독립적 구현

### 2. **모듈 통합**
- hangulDisp.h에 타입 정의와 로직 통합
- UTF-8 → 초중종성 분해, 벌 선택
- 폰트 데이터는 별도 헤더로 분리 (Font_name.h 등)


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

#### 2. [렌더링 규칙 명세](RENDERING_SPECIFICATION.md)
한글 렌더링 파이프라인과 처리 규칙을 상세히 설명합니다.
- UTF-8 처리 및 유니코드 변환
- 한글 분해 알고리즘 (초중종 분리)
- 벌 선택 규칙 (조합 규칙)
- 글리프 포인터 계산 및 비트맵 합성
- 디스플레이 출력 및 성능 최적화

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

### 폰트 변환 도구

`tools/easyview-font-converter/` 폴더에 EasyView 폰트를 Arduino 헤더로 변환하는 도구가 있습니다:

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

## 참고 자료

- [GxEPD2 한글 표시 방법](https://blog.naver.com/sanguru/221854830624)
- [전자책 프로젝트 - 한글 폰트](https://blog.naver.com/gilchida/222927710968)
- [옛한글 텍스트 뷰어 EasyView](EasyView-3.0.b2)

