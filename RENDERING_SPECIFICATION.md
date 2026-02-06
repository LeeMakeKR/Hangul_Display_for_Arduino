# 한글 렌더링 규칙 명세

## 개요

이 문서는 Arduino/ESP32용 한글 디스플레이 라이브러리의 렌더링 규칙을 상세히 설명합니다.

## 1. 렌더링 파이프라인

### 1.1 전체 흐름

```
UTF-8 문자열
    ↓
한글 문자 분해 (UTF-8 → 유니코드 → 초중종)
    ↓
벌 선택 (조합 규칙에 따라)
    ↓
글리프 포인터 계산
    ↓
비트맵 합성 (OR 연산)
    ↓
픽셀 콜백으로 출력
```

## 2. UTF-8 처리

### 2.1 한글 유니코드 범위
- **시작**: U+AC00 (가)
- **종료**: U+D7A3 (힣)
- **총 글자**: 11,172자

### 2.2 UTF-8 인코딩
한글은 3바이트 UTF-8로 인코딩됩니다:

```
바이트 1: 1110xxxx  (0xE0 | ((unicode >> 12) & 0x0F))
바이트 2: 10xxxxxx  (0x80 | ((unicode >> 6) & 0x3F))
바이트 3: 10xxxxxx  (0x80 | (unicode & 0x3F))
```

### 2.3 UTF-8 → 유니코드 변환
```cpp
uint16_t unicode = ((byte1 & 0x0F) << 12) | 
                   ((byte2 & 0x3F) << 6) | 
                   (byte3 & 0x3F);
```

## 3. 한글 분해 (Decomposition)

### 3.1 분해 공식

한글 유니코드는 수학적으로 분해됩니다:

```cpp
uint16_t code = unicode - 0xAC00;

cho  = code / 588;          // 초성 인덱스 (0~18)
jung = (code % 588) / 28;   // 중성 인덱스 (0~20)
jong = code % 28;           // 종성 인덱스 (0~27)
```

### 3.2 분해 예제

**"한"(U+D55C) 분해:**
```
code = 0xD55C - 0xAC00 = 0x295C = 10588

cho  = 10588 / 588 = 18  → ㅎ
jung = (10588 % 588) / 28 = 4 / 28 = 0  → ㅏ
jong = 10588 % 28 = 4  → ㄴ
```

## 4. 벌 선택 (Bul Selection)

### 4.1 벌 선택의 목적
한글의 시각적 조화를 위해 초성, 중성, 종성의 위치와 형태를 조정합니다.

### 4.2 초성 벌 선택

초성 벌은 **중성의 종류**와 **받침 유무**에 따라 결정됩니다.

#### 받침 없을 때 (CHO_BUL_NO_JONG)
```cpp
static const uint8_t CHO_BUL_NO_JONG[21] = {
    0, 0, 0, 0, 0, 0, 0, 0,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (1벌)
    1, 3, 3, 3, 1,           // ㅗㅘㅙㅚㅛ (2벌 또는 4벌)
    2, 4, 4, 4, 2,           // ㅜㅝㅞㅟㅠ (3벌 또는 5벌)
    1, 3, 0                  // ㅡㅢㅣ
};
```

#### 받침 있을 때 (CHO_BUL_WITH_JONG)
```cpp
static const uint8_t CHO_BUL_WITH_JONG[21] = {
    5, 5, 5, 5, 5, 5, 5, 5,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (6벌)
    6, 7, 7, 7, 6,           // ㅗㅘㅙㅚㅛ (7벌 또는 8벌)
    6, 7, 7, 7, 6,           // ㅜㅝㅞㅟㅠ (7벌 또는 8벌)
    6, 7, 5                  // ㅡㅢㅣ
};
```

### 4.3 중성 벌 선택

중성 벌은 **초성의 종류**와 **받침 유무**에 따라 결정됩니다.

```cpp
uint8_t getJungsungBul(uint8_t cho, bool hasJong) {
    if (cho == 0 || cho == 15) {  // ㄱ 또는 ㅋ
        return hasJong ? 2 : 0;    // 받침 있으면 3벌, 없으면 1벌
    } else {
        return hasJong ? 3 : 1;    // 받침 있으면 4벌, 없으면 2벌
    }
}
```

### 4.4 종성 벌 선택

종성 벌은 **중성의 종류**에 따라 결정됩니다.

```cpp
static const uint8_t JONG_BUL[21] = {
    0, 2, 0, 2, 1, 2, 1, 2,  // ㅏㅐㅑㅒㅓㅔㅕㅖ
    3, 0, 2, 1, 3,           // ㅗㅘㅙㅚㅛ
    3, 1, 2, 1, 3,           // ㅜㅝㅞㅟㅠ
    3, 1, 1                  // ㅡㅢㅣ
};
```

### 4.5 벌 선택 예제

**"한" 렌더링:**
```
초성: ㅎ (인덱스 18)
중성: ㅏ (인덱스 0)
종성: ㄴ (인덱스 4)

받침 있음: true

choBul  = CHO_BUL_WITH_JONG[0] = 5   (6벌)
jungBul = (18 != 0 && 18 != 15) && true → 3  (4벌)
jongBul = JONG_BUL[0] = 0            (1벌)
```

## 5. 글리프 포인터 계산

### 5.1 계산 공식

```cpp
glyphIndex = (벌 번호 × 자모 개수) + 자모 인덱스
byteOffset = glyphIndex × 32
```

### 5.2 초성 포인터
```cpp
uint16_t glyphIndex = choBul * 20 + (cho + 1);
uint32_t offset = glyphIndex * 32;
const uint8_t* choGlyph = choData + offset;
```

**주의**: 초성과 중성은 인덱스에 +1을 해야 합니다 (0번이 빈 값이므로).

### 5.3 중성 포인터
```cpp
uint16_t glyphIndex = jungBul * 22 + (jung + 1);
uint32_t offset = glyphIndex * 32;
const uint8_t* jungGlyph = jungData + offset;
```

### 5.4 종성 포인터
```cpp
if (jong > 0) {
    uint16_t glyphIndex = jongBul * 28 + jong;
    uint32_t offset = glyphIndex * 32;
    const uint8_t* jongGlyph = jongData + offset;
}
```

**주의**: 종성은 인덱스 그대로 사용합니다 (0이 받침 없음).

## 6. 비트맵 합성

### 6.1 OR 연산을 통한 합성

초성, 중성, 종성의 비트맵은 **OR 연산**으로 합성됩니다:

```cpp
for (int row = 0; row < 16; row++) {
    for (int col = 0; col < 16; col++) {
        bool pixel = false;
        
        if (choGlyph) {
            pixel |= getPixelFromGlyph(choGlyph, row, col);
        }
        if (jungGlyph) {
            pixel |= getPixelFromGlyph(jungGlyph, row, col);
        }
        if (jongGlyph) {
            pixel |= getPixelFromGlyph(jongGlyph, row, col);
        }
        
        if (pixel) {
            drawPixel(x + col, y + row, color);
        }
    }
}
```

### 6.2 픽셀 추출

```cpp
bool getPixelFromGlyph(const uint8_t* glyph, int row, int col) {
    uint16_t rowData = (pgm_read_byte(glyph + row * 2) << 8) |
                       pgm_read_byte(glyph + row * 2 + 1);
    return (rowData & (0x8000 >> col)) != 0;
}
```

## 7. 디스플레이 출력

### 7.1 픽셀 콜백 구조

```cpp
typedef void (*PixelDrawCallback)(int16_t x, int16_t y, uint16_t color);
```

사용자는 이 함수 포인터를 제공하여 다양한 디스플레이 라이브러리를 지원합니다.

### 7.2 콜백 예제

```cpp
// TFT_eSPI 사용
void drawPixelCallback(int16_t x, int16_t y, uint16_t color) {
    tft.drawPixel(x, y, color);
}

// U8G2 사용 (흑백)
void drawPixelCallback(int16_t x, int16_t y, uint16_t color) {
    if (color) u8g2.drawPixel(x, y);
}
```

## 8. 확대 기능 (Scaling)

### 8.1 크기 옵션

```cpp
enum HangulSize {
    HG_SIZE_NORMAL = 0,  // 16×16
    HG_SIZE_H2     = 2,  // 32×16 (가로 2배)
    HG_SIZE_V2     = 3,  // 16×32 (세로 2배)
    HG_SIZE_X4     = 4   // 32×32 (가로세로 2배)
};
```

### 8.2 확대 렌더링

```cpp
uint8_t scaleX = (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4) ? 2 : 1;
uint8_t scaleY = (textSize == HG_SIZE_V2 || textSize == HG_SIZE_X4) ? 2 : 1;

for (int row = 0; row < 16; row++) {
    for (int col = 0; col < 16; col++) {
        if (pixel) {
            for (uint8_t sy = 0; sy < scaleY; sy++) {
                for (uint8_t sx = 0; sx < scaleX; sx++) {
                    drawPixel(x + col * scaleX + sx,
                            y + row * scaleY + sy,
                            color);
                }
            }
        }
    }
}
```

## 9. 색상 처리

### 9.1 색상 모드

```cpp
enum HangulColor {
    HG_COLOR_BLACK = 0,   // 검정 (흑백 디스플레이용)
    HG_COLOR_WHITE = 1,   // 흰색 (흑백 디스플레이용)
    HG_COLOR_INVERT = 2   // 반전
};
```

### 9.2 색상 해석

```cpp
uint16_t resolveColor() const {
    if (useRawColor) {
        return textColorRaw;  // 16비트 RGB565 직접 사용
    }
    if (textColor == HG_COLOR_INVERT) {
        return 2;  // 반전 모드
    }
    return (textColor == HG_COLOR_WHITE) ? 1 : 0;
}
```

## 10. 커서 이동

### 10.1 자동 커서 이동

한글 문자를 렌더링한 후 커서는 자동으로 이동합니다:

```cpp
void advanceCursor() {
    uint8_t scaleX = (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4) ? 2 : 1;
    cursorX += 16 * scaleX;
}
```

### 10.2 ASCII 공백 처리

ASCII 문자는 현재 한글 폰트 전용이므로 16픽셀 공백으로 처리됩니다:

```cpp
if (isAscii) {
    cursorX += 16;  // 한글 너비만큼 공백
}
```

## 11. 성능 최적화

### 11.1 플래시 메모리 직접 읽기

```cpp
uint8_t byte = pgm_read_byte(pointer);
```

AVR 플랫폼에서는 `PROGMEM` 데이터를 직접 읽을 수 없으므로 `pgm_read_byte()`를 사용합니다.

### 11.2 불필요한 픽셀 스킵

```cpp
if (!pixel) continue;  // 0인 픽셀은 그리지 않음
```

## 12. 렌더링 규칙 요약

| 단계 | 입력 | 출력 | 비고 |
|------|------|------|------|
| **1. UTF-8 파싱** | 3바이트 | 유니코드 | 한글 범위 검증 |
| **2. 한글 분해** | 유니코드 | 초, 중, 종 인덱스 | 수학적 분해 |
| **3. 벌 선택** | 초중종 인덱스 | 벌 번호 | 룩업 테이블 사용 |
| **4. 글리프 계산** | 인덱스 + 벌 | 포인터 | `(벌×개수)+인덱스` |
| **5. 비트맵 합성** | 3개 글리프 | 합성 비트맵 | OR 연산 |
| **6. 픽셀 출력** | 합성 비트맵 | 화면 | 콜백 함수 |

## 13. 에러 처리

### 13.1 유효성 검사

```cpp
// 폰트 미설정
if (!fontReady) return;

// 유니코드 범위 초과
if (unicode < 0xAC00 || unicode > 0xD7A3) return;

// 벌 인덱스 범위 초과
if (jung >= 21) return 0;
```

### 13.2 안전한 포인터 처리

```cpp
if (choGlyph) {
    pixel |= getPixelFromGlyph(choGlyph, row, col);
}
```

## 14. 디버깅 팁

### 14.1 분해 확인

```cpp
Serial.print("Cho: "); Serial.println(comp.cho);
Serial.print("Jung: "); Serial.println(comp.jung);
Serial.print("Jong: "); Serial.println(comp.jong);
```

### 14.2 벌 확인

```cpp
Serial.print("ChoBul: "); Serial.println(comp.choBul);
Serial.print("JungBul: "); Serial.println(comp.jungBul);
Serial.print("JongBul: "); Serial.println(comp.jongBul);
```

### 14.3 글리프 오프셋 확인

```cpp
uint32_t offset = (choBul * 20 + cho + 1) * 32;
Serial.print("Cho Offset: "); Serial.println(offset);
```

## 15. 제한사항

### 15.1 현재 제한
- **ASCII 미지원**: 한글 전용 폰트
- **고정 크기**: 16×16 픽셀만 지원
- **비트맵 전용**: 벡터 폰트 미지원

### 15.2 향후 개선 가능 사항
- ASCII/영문 폰트 지원
- 가변 크기 폰트
- 안티앨리어싱

## 16. 예제 시나리오

### 16.1 "안녕" 렌더링

```
문자 1: "안" (U+C548)
  분해: ㅇ(10) + ㅏ(0) + ㄴ(4)
  벌: cho=6, jung=3, jong=0
  위치: (0, 0)

문자 2: "녕" (U+B155)
  분해: ㄴ(2) + ㅕ(6) + ㅇ(11)
  벌: cho=5, jung=3, jong=1
  위치: (16, 0)
```

### 16.2 "한글" 렌더링

```
문자 1: "한" (U+D55C)
  분해: ㅎ(18) + ㅏ(0) + ㄴ(4)
  벌: cho=5, jung=3, jong=0
  
문자 2: "글" (U+AE00)
  분해: ㄱ(0) + ㅡ(18) + ㄹ(8)
  벌: cho=6, jung=2, jong=3
```

## 17. 참고 문헌

- **한글 유니코드**: KS X 1001, KS X 1026-1
- **EasyView 폰트**: EasyView 5.0 한글 폰트 형식
- **벌 시스템**: 한글 타이포그래피 표준

---

**문서 버전**: 1.0  
**최종 수정**: 2026년 2월 6일  
**작성자**: Hangul Display Library Team
