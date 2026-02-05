# hangulDisp

다양한 아두이노 디스플레이 라이브러리에서 한글을 표시하기 위한 모듈화된 폰트 라이브러리

## 프로젝트 목적

U8g2, TFT_eSPI, GxEPD2, Adafruit GFX 등 아두이노의 다양한 디스플레이 라이브러리(TFT-LCD, OLED, E-Paper 등)에서 EasyView 한글 폰트를 사용할 수 있도록 하는 통합 라이브러리입니다.

## 주요 기능

- **통합 어댑터**: 모든 디스플레이에서 사용 가능한 단일 어댑터
- **다양한 디스플레이 지원**: TFT-LCD, OLED, E-Paper 등 (U8g2, TFT_eSPI, GxEPD2, M5Stack 등)
- **EasyView 폰트**: 옛한글 텍스트 뷰어의 고품질 한글 폰트 활용
- **setFont 기반**: 런타임에 폰트 선택 가능
- **메모리 효율**: PROGMEM 사용으로 플래시 메모리 활용
- **쉽고 유연한 사용**: 픽셀 그리기 콜백만 제공하면 모든 라이브러리에서 사용 가능

## 사용 방법
## 설계 원칙

### 1. **통합 어댑터**
- 단일 hangulDisp 클래스로 모든 디스플레이 지원
- 픽셀 그리기 콜백만 제공하면 됨
- 디스플레이 독립적 구현

### 2. **모듈 통합**
- hangulDisp.h에 타입 정의와 로직 통합
- UTF-8 → 초중종성 분해, 벌 선택
- 폰트 데이터는 별도 헤더로 분리 (Font_name.h 등)

### 3. **setFont 기반 설계**
```cpp
hangulDisp hangul(drawPixel);
hangul.setFont(Font_name);
```

### 4. **메모리 효율**
- 사용하는 폰트만 링크 (링크 타임 최적화)
- PROGMEM으로 플래시 메모리 활용
- 필요한 글리프만 조합하여 렌더링

## 참고 자료

- [GxEPD2 한글 표시 방법](https://blog.naver.com/sanguru/221854830624)
- [전자책 프로젝트 - 한글 폰트](https://blog.naver.com/gilchida/222927710968)
- [옛한글 텍스트 뷰어 EasyView](EasyView-3.0.b2)

