# 한글 폰트 변환기 (han_to_h.py)

EasyView 조합형 한글 폰트 바이너리 파일(`.han`)을 Arduino/ESP32용 C 헤더 파일(`.h`)로 변환하는 Python 스크립트입니다.

## 사용법

### 기본 사용법

```bash
python han_to_h.py <input.han> [output.h]
```

### 예제

```bash
# 출력 파일명 자동 생성 (입력 파일과 같은 폴더에 생성)
python han_to_h.py ../EasyView-font/ko/Apple_kr.han

# 출력 파일명 지정
python han_to_h.py ../EasyView-font/ko/Apple_kr.han ../hangulDisp/fonts/hangul/Apple_kr.h
```

## 출력 헤더 파일 구조

생성된 `.h` 파일은 다음과 같은 구조를 가집니다:

### 1. 폰트 상수 정의

```cpp
#define APPLE_KR_WIDTH 16                    // 글리프 너비 (픽셀)
#define APPLE_KR_HEIGHT 16                   // 글리프 높이 (픽셀)
#define APPLE_KR_BYTES_PER_GLYPH 32         // 글리프당 바이트 수
#define APPLE_KR_TOTAL_GLYPHS 360           // 총 글리프 수

#define APPLE_KR_CHO_OFFSET 0               // 초성 섹션 시작 인덱스
#define APPLE_KR_JUNG_OFFSET 160            // 중성 섹션 시작 인덱스
#define APPLE_KR_JONG_OFFSET 248            // 종성 섹션 시작 인덱스

#define APPLE_KR_CHO_COUNT 20               // 초성 문자 수
#define APPLE_KR_CHO_BUL 8                  // 초성 벌 수
#define APPLE_KR_JUNG_COUNT 22              // 중성 문자 수
#define APPLE_KR_JUNG_BUL 4                 // 중성 벌 수
#define APPLE_KR_JONG_COUNT 28              // 종성 문자 수
#define APPLE_KR_JONG_BUL 4                 // 종성 벌 수
```

### 2. 비트맵 데이터

```cpp
const uint8_t Apple_kr_Bitmaps[] PROGMEM = {
  // 11,520 바이트의 폰트 비트맵 데이터
  // 각 글리프는 32바이트 (16x16 픽셀, MSB first)
  0x00, 0x00, 0x00, 0x00, ...
};
```

### 3. 폰트 정보 구조체

```cpp
typedef struct {
  const uint8_t *bitmap;      // 비트맵 데이터 포인터
  uint8_t width;              // 글리프 너비
  uint8_t height;             // 글리프 높이
  uint8_t bytesPerGlyph;      // 글리프당 바이트 수
  uint16_t totalGlyphs;       // 총 글리프 수
  uint8_t choOffset;          // 초성 오프셋
  uint8_t jungOffset;         // 중성 오프셋
  uint16_t jongOffset;        // 종성 오프셋
} HangulFontInfo;

const HangulFontInfo Apple_kr_Info PROGMEM = {
  Apple_kr_Bitmaps, 16, 16, 32, 360, 0, 160, 248
};
```

### 4. 헬퍼 함수

#### 글리프 인덱스로 데이터 가져오기
```cpp
const uint8_t* Apple_kr_getGlyph(uint16_t index);
// index: 0~359 (총 360개 글리프)
```

#### 초성 글리프 가져오기
```cpp
const uint8_t* Apple_kr_getCho(uint8_t bul, uint8_t index);
// bul: 0~7 (초성 8벌)
// index: 0~19 (초성 20자)
```

#### 중성 글리프 가져오기
```cpp
const uint8_t* Apple_kr_getJung(uint8_t bul, uint8_t index);
// bul: 0~3 (중성 4벌)
// index: 0~21 (중성 22자)
```

#### 종성 글리프 가져오기
```cpp
const uint8_t* Apple_kr_getJong(uint8_t bul, uint8_t index);
// bul: 0~3 (종성 4벌)
// index: 0~27 (종성 28자)
```

## Arduino 사용 예제

### 1. 기본 사용법

```cpp
#include "Apple_kr.h"

void setup() {
  // 초기화 코드
}

void loop() {
  // 초성 'ㄱ'의 1벌 글리프 가져오기
  const uint8_t* cho_glyph = Apple_kr_getCho(0, 1);
  
  // 16x16 픽셀 그리기
  if (cho_glyph != nullptr) {
    for (int y = 0; y < 16; y++) {
      uint16_t row = (cho_glyph[y*2] << 8) | cho_glyph[y*2 + 1];
      for (int x = 0; x < 16; x++) {
        if (row & (0x8000 >> x)) {
          // 픽셀 그리기
          drawPixel(x, y, WHITE);
        }
      }
    }
  }
}
```

### 2. 한글 조합 예제

```cpp
#include "Apple_kr.h"

// 벌 선택 테이블 (폰트변환레퍼런스.md 참조)
const uint8_t cho_bul_table[] = {
  // 중성 인덱스별 초성 벌 (받침 없음)
  // 구현 필요
};

void drawHangul(int x, int y, uint8_t cho, uint8_t jung, uint8_t jong) {
  // 1. 벌 수 계산
  uint8_t cho_bul = calculateChoBul(jung, jong);
  uint8_t jung_bul = calculateJungBul(cho, jong);
  uint8_t jong_bul = calculateJongBul(jung);
  
  // 2. 글리프 가져오기
  const uint8_t* cho_glyph = Apple_kr_getCho(cho_bul, cho);
  const uint8_t* jung_glyph = Apple_kr_getJung(jung_bul, jung);
  const uint8_t* jong_glyph = jong > 0 ? Apple_kr_getJong(jong_bul, jong) : nullptr;
  
  // 3. 조합하여 그리기
  drawGlyph(x, y, cho_glyph);
  drawGlyph(x, y, jung_glyph);  // OR 연산으로 합성
  if (jong_glyph != nullptr) {
    drawGlyph(x, y, jong_glyph);  // OR 연산으로 합성
  }
}

void drawGlyph(int x, int y, const uint8_t* glyph) {
  if (glyph == nullptr) return;
  
  for (int row = 0; row < 16; row++) {
    uint16_t rowData = (glyph[row*2] << 8) | glyph[row*2 + 1];
    for (int col = 0; col < 16; col++) {
      if (rowData & (0x8000 >> col)) {
        drawPixel(x + col, y + row, WHITE);
      }
    }
  }
}
```

### 3. UTF-8 문자열 출력 예제

```cpp
#include "Apple_kr.h"

// UTF-8 → 유니코드 변환
uint16_t utf8ToUnicode(uint8_t b1, uint8_t b2, uint8_t b3) {
  return ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
}

// 유니코드 → 초성/중성/종성 분리
void unicodeToJamo(uint16_t unicode, uint8_t* cho, uint8_t* jung, uint8_t* jong) {
  uint16_t val = unicode - 0xAC00;
  *jong = val % 28;
  *jung = (val / 28) % 21;
  *cho = val / (28 * 21);
}

// UTF-8 문자열 출력
void printString(int x, int y, const char* str) {
  int posX = x;
  int i = 0;
  
  while (str[i] != '\0') {
    if ((str[i] & 0x80) == 0) {
      // ASCII (1바이트)
      drawAscii(posX, y, str[i]);
      posX += 8;
      i++;
    } else if ((str[i] & 0xE0) == 0xC0) {
      // 2바이트 UTF-8 (건너뛰기)
      i += 2;
    } else if ((str[i] & 0xF0) == 0xE0) {
      // 3바이트 UTF-8 (한글)
      uint16_t unicode = utf8ToUnicode(str[i], str[i+1], str[i+2]);
      
      uint8_t cho, jung, jong;
      unicodeToJamo(unicode, &cho, &jung, &jong);
      
      // 변환 테이블 적용 (폰트변환레퍼런스.md 참조)
      // cho = cho_rdix[cho];
      // jung = jung_rdix[jung];
      // jong = jong_rdix[jong];
      
      drawHangul(posX, y, cho, jung, jong);
      posX += 16;
      i += 3;
    } else {
      // 4바이트 이상 (건너뛰기)
      i++;
    }
  }
}
```

## 폰트 구조

### 글리프 배치

| 섹션 | 인덱스 범위 | 구성 | 설명 |
|------|------------|------|------|
| 초성 | 0~159 | 20자 × 8벌 | ㄱ ㄲ ㄴ ㄷ ㄸ ㄹ ㅁ ㅂ ㅃ ㅅ ㅆ ㅇ ㅈ ㅉ ㅊ ㅋ ㅌ ㅍ ㅎ (19자 + 빈자리) |
| 중성 | 160~247 | 22자 × 4벌 | ㅏ ㅐ ㅑ ㅒ ㅓ ㅔ ㅕ ㅖ ㅗ ㅘ ㅙ ㅚ ㅛ ㅜ ㅝ ㅞ ㅟ ㅠ ㅡ ㅢ ㅣ (21자 + 빈자리) |
| 종성 | 248~359 | 28자 × 4벌 | ㄱ ㄲ ㄳ ㄴ ㄵ ㄶ ㄷ ㄹ ㄺ ㄻ ㄼ ㄽ ㄾ ㄿ ㅀ ㅁ ㅂ ㅄ ㅅ ㅆ ㅇ ㅈ ㅊ ㅋ ㅌ ㅍ ㅎ (27자 + 빈자리) |

### 벌식 시스템

- **초성 8벌**: 중성의 종류와 받침 유무에 따라 다른 형태
- **중성 4벌**: 초성의 종류(ㄱ/ㅋ)와 받침 유무에 따라 다른 형태
- **종성 4벌**: 중성의 종류에 따라 다른 형태

자세한 벌 선택 규칙은 `폰트변환레퍼런스.md` 문서를 참조하세요.

## 참고 문서

- `폰트변환레퍼런스.md`: 한글 폰트 구조 및 변환 과정 상세 설명
- `FreeSans9pt7b.h`: GFX 폰트 포맷 참조 예제

## 라이선스

이 변환기는 EasyView 폰트 파일을 Arduino/ESP32에서 사용 가능한 형식으로 변환합니다.
원본 폰트의 라이선스를 확인하시기 바랍니다.

## 버전

- v1.0.0 (2025-10-22): 초기 버전
  - .han 파일 → .h 파일 변환
  - 360개 글리프 (초성 8벌, 중성 4벌, 종성 4벌)
  - PROGMEM 최적화
  - 헬퍼 함수 제공
