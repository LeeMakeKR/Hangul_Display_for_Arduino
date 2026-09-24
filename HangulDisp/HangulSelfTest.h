/*
 * HangulSelfTest.h - hangulDisp 런타임 자가진단 (D03, D04, D08)
 *
 * 선택 사항인 헤더다. 포함하지 않으면 코드 크기에 영향이 없다.
 * 캡처 콜백을 붙여 좌표·색상·호출 수·커서를 확인한다.
 *
 * 같은 검사를 두 곳에서 쓴다.
 *   - tests/host/test_core_runtime.cpp : 호스트에서 실행
 *   - examples/SelfTest/SelfTest.ino   : 실제 보드에서 시리얼로 결과 출력
 *
 * 전부 inline이며 동적 할당·화면 버퍼를 쓰지 않는다.
 */

#ifndef HANGUL_SELF_TEST_H
#define HANGUL_SELF_TEST_H

#include "HangulDisp.h"

namespace HangulSelfTest {

// 검사 결과
struct Result {
    uint16_t passed;
    uint16_t failed;
};

// 검사 한 건의 결과 보고 콜백 (nullptr이면 보고하지 않는다)
// AVR에서 name은 PROGMEM 문자열 포인터다 (HG_TEST_NAME 참조).
typedef void (*ReportFn)(const char* name, bool ok);

// ---------------------------------------------------------------------------
// 캡처 콜백: 픽셀 호출 수, 경계 상자, 색상을 기록한다 (약 12바이트)
// ---------------------------------------------------------------------------
struct CaptureState {
    uint16_t count;
    int16_t minX;
    int16_t minY;
    int16_t maxX;
    int16_t maxY;
    uint16_t lastColor;
    uint16_t rows[HANGUL_GLYPH_HEIGHT];   // (0,0) 기준 16x16 비트맵 (32바이트)
};

// 상수 초기화라 가드 변수가 생기지 않는다 (AVR에서도 안전)
inline CaptureState& capture() {
    static CaptureState state = { 0, 0, 0, 0, 0, 0, { 0 } };
    return state;
}

inline void captureReset() {
    CaptureState& s = capture();
    s.count = 0;
    s.minX = 32767; s.minY = 32767;
    s.maxX = -32768; s.maxY = -32768;
    s.lastColor = 0xFFFF;
    for (uint8_t i = 0; i < HANGUL_GLYPH_HEIGHT; i++) s.rows[i] = 0;
}

inline void capturePixel(int16_t x, int16_t y, uint16_t color) {
    CaptureState& s = capture();
    s.count++;
    if (x < s.minX) s.minX = x;
    if (y < s.minY) s.minY = y;
    if (x > s.maxX) s.maxX = x;
    if (y > s.maxY) s.maxY = y;
    s.lastColor = color;
    if (x >= 0 && x < HANGUL_GLYPH_WIDTH && y >= 0 && y < HANGUL_GLYPH_HEIGHT) {
        s.rows[y] |= (uint16_t)(0x8000u >> x);
    }
}

inline int16_t captureWidth()  { return (int16_t)(capture().maxX - capture().minX + 1); }
inline int16_t captureHeight() { return (int16_t)(capture().maxY - capture().minY + 1); }

// 논리 색상 변환기 (어댑터 역할 대역)
inline uint16_t testColorResolver(HangulColor logical) {
    switch (logical) {
        case HG_COLOR_BLACK:  return 0x0000;
        case HG_COLOR_WHITE:  return 0xFFFF;
        case HG_COLOR_INVERT: return 0x00AA;
    }
    return 0x0000;
}

// AVR에서는 검사 이름을 SRAM이 아니라 Flash에 둔다.
// 이때 report()가 받는 포인터는 PROGMEM 문자열이므로
// Serial.println((const __FlashStringHelper*)name) 으로 출력해야 한다.
#if defined(__AVR__)
#define HG_TEST_NAME(s) PSTR(s)
#else
#define HG_TEST_NAME(s) (s)
#endif

#define HG_CHECK(name, cond)                                do {                                                        const bool ok_ = (cond);                                if (ok_) result.passed++; else result.failed++;         if (report) report(HG_TEST_NAME(name), ok_);        } while (0)


// ---------------------------------------------------------------------------
// 행 단위 합성 동등성 시험 (5단계 최적화 전후 동작 보존)
//
// 수정 전 코어는 픽셀마다 구성요소 3개를 따로 읽어 OR 했고,
// 수정 후 코어는 행마다 한 번씩 읽어 OR 한다.
// 아래 reference*()가 수정 전 방식 그대로 기대값을 만들고,
// 실제 출력(캡처 결과)과 비트 단위로 비교한다.
// ---------------------------------------------------------------------------

// 수정 전 getPixelFromGlyph()와 같은 계산 (픽셀 단위 접근)
inline bool referencePixel(const uint8_t* glyph, uint8_t row, uint8_t col) {
    if (!glyph) return false;
    const uint16_t rowData = (uint16_t)(((uint16_t)pgm_read_byte(glyph + row * 2) << 8) |
                                        (uint16_t)pgm_read_byte(glyph + row * 2 + 1));
    return (rowData & (uint16_t)(0x8000u >> col)) != 0;
}

inline const uint8_t* referenceGlyph(const uint8_t* base, uint8_t index,
                                     uint8_t bul, uint8_t indexCount) {
    if (!base) return nullptr;
    const uint16_t glyphIndex = (uint16_t)((uint16_t)bul * indexCount + index);
    return base + (uint16_t)(glyphIndex * HANGUL_BYTES_PER_GLYPH);
}

/**
 * 음절 하나를 그려서 픽셀 단위 기준 구현과 결과가 같은지 확인한다.
 * @return 같으면 true
 */
inline bool sameAsReference(const HangulFontInfo& font, hangulDisp& disp, uint32_t codepoint) {
    HangulComponents comp = HangulCore::emptyComponents();
    if (!HangulCore::tryDecomposeCodepoint(codepoint, comp)) return false;

    const uint8_t* cho = referenceGlyph(font.choData, (uint8_t)(comp.cho + 1),
                                        comp.choBul, HANGUL_CHO_COUNT);
    const uint8_t* jung = referenceGlyph(font.jungData, (uint8_t)(comp.jung + 1),
                                         comp.jungBul, HANGUL_JUNG_COUNT);
    const uint8_t* jong = (comp.jong > 0)
        ? referenceGlyph(font.jongData, comp.jong, comp.jongBul, HANGUL_JONG_COUNT)
        : nullptr;

    // 실제 출력
    const char encoded[3] = {
        (char)(uint8_t)(0xE0u | (codepoint >> 12)),
        (char)(uint8_t)(0x80u | ((codepoint >> 6) & 0x3Fu)),
        (char)(uint8_t)(0x80u | (codepoint & 0x3Fu))
    };
    captureReset();
    disp.setTextSize(HG_SIZE_NORMAL);
    disp.setCursor(0, 0);
    disp.print(encoded, 3);

    if (disp.getCursorX() != HANGUL_GLYPH_WIDTH) return false;

    // 기대값: 픽셀 단위로 세 구성요소를 각각 읽어 OR (수정 전 방식)
    for (uint8_t row = 0; row < HANGUL_GLYPH_HEIGHT; row++) {
        uint16_t expected = 0;
        for (uint8_t col = 0; col < HANGUL_GLYPH_WIDTH; col++) {
            const bool on = referencePixel(cho, row, col) ||
                            referencePixel(jung, row, col) ||
                            referencePixel(jong, row, col);
            if (on) expected |= (uint16_t)(0x8000u >> col);
        }
        if (capture().rows[row] != expected) return false;
    }
    return true;
}

/**
 * 현대 한글 음절 전체(또는 step 간격 표본)에 대해 동등성을 확인한다.
 * @param step 1이면 11,172자 전부. Uno에서는 수십 초 걸릴 수 있다.
 * @return 어긋난 첫 코드포인트. 전부 같으면 0.
 */
inline uint32_t runGlyphSweep(const HangulFontInfo& font, uint16_t step) {
    if (step == 0) step = 1;
    hangulDisp disp(capturePixel);
    disp.setFont(font);
    if (!disp.isFontReady()) return HANGUL_SYLLABLE_FIRST;

    for (uint32_t cp = HANGUL_SYLLABLE_FIRST; cp <= HANGUL_SYLLABLE_LAST; cp += step) {
        if (!sameAsReference(font, disp, cp)) {
            return cp;
        }
    }
    return 0;
}

/**
 * 모든 런타임 검사를 수행한다.
 * @param font 켜진 픽셀이 있는 정상 16x16 폰트 (예: H01_kr)
 */
inline Result runAll(const HangulFontInfo& font, ReportFn report) {
    Result result = { 0, 0 };

    // UTF-8 시험 입력 (여분 패딩이 없는 배열로 둔다)
    static const char GA[4]        = { (char)0xEA, (char)0xB0, (char)0x80, 0 };  // 가
    static const char GAK[4]       = { (char)0xEA, (char)0xB0, (char)0x81, 0 };  // 각
    static const char HIH[4]       = { (char)0xED, (char)0x9E, (char)0xA3, 0 };  // 힣
    static const char HANJA[4]     = { (char)0xE6, (char)0xBC, (char)0xA2, 0 };  // 漢
    static const char JAMO[4]      = { (char)0xE3, (char)0x84, (char)0xB1, 0 };  // ㄱ
    static const char EMOJI[5]     = { (char)0xF0, (char)0x9F, (char)0x98, (char)0x80, 0 };
    static const char GA_EMOJI_GAK[11] = { (char)0xEA, (char)0xB0, (char)0x80,
                                           (char)0xF0, (char)0x9F, (char)0x98, (char)0x80,
                                           (char)0xEA, (char)0xB0, (char)0x81, 0 };
    static const char EMOJI_GAK[8] = { (char)0xF0, (char)0x9F, (char)0x98, (char)0x80,
                                       (char)0xEA, (char)0xB0, (char)0x81, 0 };
    static const char TRUNC_C2[2]  = { (char)0xC2, 0 };
    static const char TRUNC_EAB0[3]= { (char)0xEA, (char)0xB0, 0 };
    static const char LONE_CONT[2] = { (char)0x80, 0 };
    static const char EA30_80[4]   = { (char)0xEA, (char)0x30, (char)0x80, 0 };
    static const char OVERLONG[3]  = { (char)0xC0, (char)0xAF, 0 };
    static const char SURROGATE[4] = { (char)0xED, (char)0xA0, (char)0x80, 0 };
    static const char OVER_MAX[5]  = { (char)0xF4, (char)0x90, (char)0x80, (char)0x80, 0 };
    static const char TRUNC_4[3]   = { (char)0xF0, (char)0x9F, 0 };
    static const char GAK_RAW[3]   = { (char)0xEA, (char)0xB0, (char)0x81 };  // NUL 없음

    hangulDisp disp(capturePixel);
    disp.setFont(font);

    // -----------------------------------------------------------------
    // 1. 정상 한글 출력
    // -----------------------------------------------------------------
    HG_CHECK("정상 폰트가 준비 상태", disp.isFontReady());

    captureReset();
    disp.setCursor(10, 20);
    disp.print(GA);
    HG_CHECK("'가' 픽셀이 출력됨", capture().count > 0);
    HG_CHECK("'가' 픽셀이 16x16 안에 있음",
             capture().minX >= 10 && capture().maxX <= 25 &&
             capture().minY >= 20 && capture().maxY <= 35);
    HG_CHECK("'가' 커서가 16 전진", disp.getCursorX() == 26 && disp.getCursorY() == 20);

    captureReset();
    disp.setCursor(0, 0);
    disp.print(HIH);
    HG_CHECK("'힣' 픽셀이 출력됨", capture().count > 0);
    HG_CHECK("'힣' 커서가 16 전진", disp.getCursorX() == 16);

    // 받침이 있으면 받침 글리프도 합성된다 -> 세로 범위가 더 아래까지 간다
    captureReset();
    disp.setCursor(0, 0);
    disp.print(GA);
    const int16_t gaMaxY = capture().maxY;
    captureReset();
    disp.setCursor(0, 0);
    disp.print(GAK);
    HG_CHECK("'각'이 '가'보다 아래까지 그려짐(받침 합성)", capture().maxY > gaMaxY);

    // -----------------------------------------------------------------
    // 2. 미지원 문자 - 소비만 하고 출력·커서 변경 없음
    // -----------------------------------------------------------------
    const char* unsupported[3] = { HANJA, JAMO, EMOJI };
    for (uint8_t i = 0; i < 3; i++) {
        captureReset();
        disp.setCursor(5, 5);
        disp.print(unsupported[i]);
        HG_CHECK("미지원 문자: 픽셀 없음", capture().count == 0);
        HG_CHECK("미지원 문자: 커서 불변", disp.getCursorX() == 5);
    }

    // -----------------------------------------------------------------
    // 3. 잘못된 인코딩 - 출력 없음, 무한 반복 없음(반환 자체가 통과 조건)
    // -----------------------------------------------------------------
    const char* broken[8] = { TRUNC_C2, TRUNC_EAB0, LONE_CONT, EA30_80,
                              OVERLONG, SURROGATE, OVER_MAX, TRUNC_4 };
    for (uint8_t i = 0; i < 8; i++) {
        captureReset();
        disp.setCursor(7, 7);
        disp.print(broken[i]);
        HG_CHECK("잘못된 인코딩: 픽셀 없음", capture().count == 0);
        HG_CHECK("잘못된 인코딩: 커서 불변", disp.getCursorX() == 7);
    }

    // EA 30 80 : 0x30('0')은 ASCII로 재검사되지만 한글이 그려지면 안 된다
    captureReset();
    disp.setCursor(0, 0);
    disp.print(EA30_80);
    HG_CHECK("EA 30 80: 한글 출력 없음", capture().count == 0);

    // -----------------------------------------------------------------
    // 4. 혼합 문자열과 탐색
    // -----------------------------------------------------------------
    captureReset();
    disp.setCursor(0, 0);
    disp.print(GA_EMOJI_GAK);   // 가 + 이모지 + 각
    HG_CHECK("'가😀각': 한글 2자만 전진", disp.getCursorX() == 32);

    HG_CHECK("findNextHangul(이모지+각) 위치",
             HangulCore::findNextHangul(EMOJI_GAK) == EMOJI_GAK + 4);
    HG_CHECK("findNextHangul(이모지만)은 nullptr",
             HangulCore::findNextHangul(EMOJI) == nullptr);
    HG_CHECK("findNextHangul(가+이모지+각) 위치",
             HangulCore::findNextHangul(GA_EMOJI_GAK) == GA_EMOJI_GAK);

    // -----------------------------------------------------------------
    // 5. 길이 지정 API - 전달 길이 밖을 읽지 않는다
    // -----------------------------------------------------------------
    captureReset();
    disp.setCursor(0, 0);
    disp.print(GAK_RAW, 2);     // 3바이트 중 2바이트만 허용
    HG_CHECK("길이 2 제한: 출력 없음", capture().count == 0);
    HG_CHECK("길이 2 제한: 커서 불변", disp.getCursorX() == 0);

    captureReset();
    disp.setCursor(0, 0);
    disp.print(GAK_RAW, 3);
    HG_CHECK("길이 3: 정상 출력", capture().count > 0 && disp.getCursorX() == 16);

    // -----------------------------------------------------------------
    // 6. null / 빈 문자열
    // -----------------------------------------------------------------
    captureReset();
    disp.setCursor(3, 3);
    disp.print((const char*)nullptr);
    disp.print("");
    disp.print((const char*)nullptr, 10);
    HG_CHECK("null/빈 문자열: 픽셀 없음", capture().count == 0);
    HG_CHECK("null/빈 문자열: 커서 불변", disp.getCursorX() == 3);

    // -----------------------------------------------------------------
    // 7. ASCII 계약
    // -----------------------------------------------------------------
    captureReset();
    disp.setTextSize(HG_SIZE_NORMAL);
    disp.setCursor(0, 0);
    disp.print(" ");
    HG_CHECK("공백: 픽셀 없이 16 전진", capture().count == 0 && disp.getCursorX() == 16);

    captureReset();
    disp.setCursor(0, 0);
    disp.print("\r\nA1z");
    HG_CHECK("CR/LF/영숫자: 픽셀·커서 변화 없음",
             capture().count == 0 && disp.getCursorX() == 0);

    disp.setTextSize(HG_SIZE_H2);
    disp.setCursor(0, 0);
    disp.print(" ");
    HG_CHECK("공백: 가로 2배에서 32 전진", disp.getCursorX() == 32);
    disp.setTextSize(HG_SIZE_NORMAL);

    // -----------------------------------------------------------------
    // 8. 배율
    // -----------------------------------------------------------------
    captureReset();
    disp.setTextSize(HG_SIZE_NORMAL);
    disp.setCursor(0, 0);
    disp.print(GAK);
    const uint16_t normalCount = capture().count;
    const int16_t normalW = captureWidth();
    const int16_t normalH = captureHeight();
    HG_CHECK("기본 배율: 커서 16", disp.getCursorX() == 16);

    captureReset();
    disp.setTextSize(HG_SIZE_H2);
    disp.setCursor(0, 0);
    disp.print(GAK);
    HG_CHECK("가로 2배: 픽셀 2배", capture().count == (uint16_t)(normalCount * 2));
    HG_CHECK("가로 2배: 폭 2배, 높이 동일",
             captureWidth() >= normalW && captureHeight() == normalH);
    HG_CHECK("가로 2배: 커서 32", disp.getCursorX() == 32);

    captureReset();
    disp.setTextSize(HG_SIZE_V2);
    disp.setCursor(0, 0);
    disp.print(GAK);
    HG_CHECK("세로 2배: 픽셀 2배", capture().count == (uint16_t)(normalCount * 2));
    HG_CHECK("세로 2배: 커서 16", disp.getCursorX() == 16);

    captureReset();
    disp.setTextSize(HG_SIZE_X4);
    disp.setCursor(0, 0);
    disp.print(GAK);
    HG_CHECK("4배: 픽셀 4배", capture().count == (uint16_t)(normalCount * 4));
    HG_CHECK("4배: 커서 32", disp.getCursorX() == 32);
    disp.setTextSize(HG_SIZE_NORMAL);

    // -----------------------------------------------------------------
    // 9. 색상 경로 (원시 / 논리 / 변환기)
    // -----------------------------------------------------------------
    captureReset();
    disp.setCursor(0, 0);
    disp.setTextColor((uint16_t)0x1234);
    disp.print(GA);
    HG_CHECK("원시 색상은 그대로 전달", capture().lastColor == 0x1234);
    HG_CHECK("원시 색상 모드 표시", disp.isRawColor());

    captureReset();
    disp.setCursor(0, 0);
    disp.setTextColor(HG_COLOR_WHITE);
    disp.print(GA);
    HG_CHECK("변환기 없으면 논리 코드 전달", capture().lastColor == 1);
    HG_CHECK("논리 색상 모드 표시", !disp.isRawColor());

    disp.setColorResolver(testColorResolver);
    captureReset();
    disp.setCursor(0, 0);
    disp.setTextColor(HG_COLOR_WHITE);
    disp.print(GA);
    HG_CHECK("변환기가 드라이버 값으로 변환", capture().lastColor == 0xFFFF);

    captureReset();
    disp.setCursor(0, 0);
    disp.setTextColor(HG_COLOR_INVERT);
    disp.print(GA);
    HG_CHECK("변환기가 반전 값 변환", capture().lastColor == 0x00AA);

    captureReset();
    disp.setCursor(0, 0);
    disp.setTextColor((uint16_t)0x4321);
    disp.print(GA);
    HG_CHECK("변환기가 있어도 원시 값은 그대로", capture().lastColor == 0x4321);
    disp.setColorResolver(nullptr);
    disp.setTextColor((uint16_t)1);

    // -----------------------------------------------------------------
    // 10. printHangulChar 직접 호출
    // -----------------------------------------------------------------
    captureReset();
    disp.setCursor(0, 0);
    disp.printHangulChar(0xEA, 0xB0, 0x80);
    HG_CHECK("printHangulChar 정상 출력", capture().count > 0 && disp.getCursorX() == 16);

    captureReset();
    disp.setCursor(0, 0);
    disp.printHangulChar(0xEA, 0xB0, 0x00);   // 잘린 입력
    disp.printHangulChar(0xE6, 0xBC, 0xA2);   // 한자
    disp.printHangulChar(0x41, 0x42, 0x43);   // ASCII
    HG_CHECK("printHangulChar 무효 입력: 출력·커서 없음",
             capture().count == 0 && disp.getCursorX() == 0);

    // -----------------------------------------------------------------
    // 11. 폰트 무효화 (D08)
    // -----------------------------------------------------------------
    HangulFontInfo wrongSize = font;
    wrongSize.width = 8;
    wrongSize.height = 8;
    disp.setFont(wrongSize);
    HG_CHECK("8x8 폰트는 거부됨", !disp.isFontReady());

    captureReset();
    disp.setCursor(4, 4);
    disp.print(GA);
    disp.printHangulChar(0xEA, 0xB0, 0x80);
    HG_CHECK("무효 폰트: 픽셀 없음", capture().count == 0);
    HG_CHECK("무효 폰트: 커서 불변", disp.getCursorX() == 4);

    HangulFontInfo nullPtrFont = font;
    nullPtrFont.jongData = nullptr;
    disp.setFont(font);                 // 먼저 정상 폰트로 되돌린 뒤
    HG_CHECK("정상 폰트 재설정 성공", disp.isFontReady());
    disp.setFont(nullPtrFont);          // null 포인터 폰트로 덮어쓴다
    HG_CHECK("null 포인터 폰트는 거부됨", !disp.isFontReady());

    captureReset();
    disp.setCursor(9, 9);
    disp.print(GAK);
    HG_CHECK("null 포인터 폰트: 픽셀 없음", capture().count == 0);
    HG_CHECK("null 포인터 폰트: 커서 불변", disp.getCursorX() == 9);

    // 폰트를 되돌려 놓는다
    disp.setFont(font);

    // -----------------------------------------------------------------
    // 12. null 콜백 (D04)
    //     켜진 픽셀이 있는 폰트를 써야 결함을 잡을 수 있다.
    // -----------------------------------------------------------------
    hangulDisp noCallback(nullptr);
    noCallback.setFont(font);
    HG_CHECK("콜백 없어도 폰트 자체는 유효", noCallback.isFontReady());
    HG_CHECK("콜백 없으면 그릴 준비는 안 됨", !noCallback.isReadyToDraw());

    noCallback.setCursor(2, 2);
    noCallback.printHangulChar(0xEA, 0xB0, 0x80);   // 직접 호출
    noCallback.print(GA);                            // 문자열 경로
    noCallback.print(GAK_RAW, 3);                    // 길이 지정 경로
    HG_CHECK("null 콜백: 커서 불변 (충돌 없이 반환)", noCallback.getCursorX() == 2);

    return result;
}

#undef HG_CHECK
#undef HG_TEST_NAME

}  // namespace HangulSelfTest

#endif  // HANGUL_SELF_TEST_H
