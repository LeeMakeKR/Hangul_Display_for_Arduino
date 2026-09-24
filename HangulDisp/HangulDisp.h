/*
 * HangulDisp.h
 *
 * HangulDisp 라이브러리 통합 헤더
 * - HangulCore 타입/상수/함수 선언
 * - 통합 디스플레이 어댑터
 *
 * 사용 방법:
 * 1. 폰트 헤더 포함 (사용할 폰트에 따라 다름, 예: fonts/H01_kr.h, fonts/H02_kr.h 등)
 * 2. hangulDisp 인스턴스 생성
 * 3. 픽셀 그리기 콜백 제공
 * 4. setFont()로 폰트 지정
 * 5. print() 함수로 UTF-8 문자열 출력
 *
 * 예제 (H01 폰트 사용시):
 * ```cpp
 * #include "HangulDisp.h"
 * #include "fonts/H01_kr.h"  // 사용할 폰트 헤더 (HangulDisp/fonts/ 폴더)
 *
 * TFT_eSPI tft;  // 또는 U8G2, GxEPD2 등
 *
 * void drawPixel(int16_t x, int16_t y, uint16_t color) {
 *     tft.drawPixel(x, y, color);
 * }
 *
 * hangulDisp hangul(drawPixel);
 * hangul.setFont(H01_kr);  // 폰트 헤더가 정의한 HangulFontInfo 변수명
 * hangul.print(10, 30, "안녕하세요 한글출력입니다", TFT_WHITE);
 * ```
 *
 * ----------------------------------------------------------------------------
 * 문자 처리 계약 (이 코어는 한글 렌더러다)
 * ----------------------------------------------------------------------------
 * | 입력                           | 처리                                       |
 * |--------------------------------|--------------------------------------------|
 * | U+AC00~U+D7A3 (현대 한글)      | 코드포인트 전체 소비, 합성 출력, 커서 전진 |
 * | 공백 0x20                      | 소비, 픽셀 없음, 커서 16*가로배율 전진     |
 * | 그 외 ASCII (CR/LF 포함)       | 소비, 픽셀 없음, 커서 이동 없음            |
 * | 유효하지만 미지원인 비ASCII    | 코드포인트 전체 소비, 픽셀·커서 변경 없음  |
 * | 잘못된 UTF-8 인코딩            | 1바이트만 소비 후 재검사, 출력 없음        |
 * | NUL 또는 전달 길이 끝에서 잘림 | 범위 안에서 종료, 범위 밖 접근 없음        |
 *
 * 영문·숫자·개행 혼합 출력은 어댑터(또는 호출자)가 기존 라이브러리의 폰트로
 * 처리한다. 이 코어는 영문 비트맵을 포함하지 않는다.
 *
 * ----------------------------------------------------------------------------
 * 폰트 데이터 책임 범위
 * ----------------------------------------------------------------------------
 * setFont()은 width/height가 16x16인지와 세 포인터가 null이 아닌지만 확인한다.
 * 원시 포인터만으로 실제 할당 길이는 확인할 수 없으므로, 사용자 정의 폰트는
 * 아래 버퍼 길이를 반드시 보장해야 한다. 이 검사가 임의 포인터까지 안전하게
 * 만들어 주지는 않는다.
 *   - choData : 20 x 8벌 x 32바이트 = 5,120바이트
 *   - jungData: 22 x 4벌 x 32바이트 = 2,816바이트
 *   - jongData: 28 x 4벌 x 32바이트 = 3,584바이트
 *   - 합계 11,520바이트 (tools/easyview-font-converter 가 생성하는 형식)
 */

#ifndef HANGUL_DISP_H
#define HANGUL_DISP_H

#include <stdint.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#endif

// C++14 이상에서는 코어 함수를 컴파일 타임 상수 평가로 검증할 수 있게 한다.
// C++11(아두이노 AVR 기본)에서는 그냥 inline 함수다. 런타임 동작은 같다.
#if defined(__cpp_constexpr) && __cpp_constexpr >= 201304L
#define HANGUL_CONSTEXPR constexpr
#else
#define HANGUL_CONSTEXPR inline
#endif

// AVR에서 벌 선택표는 PROGMEM(Flash)에 둔다. 그 외 환경에서는 상수 평가가
// 가능하도록 constexpr로 둔다.
#if !defined(__AVR__) && defined(__cpp_constexpr) && __cpp_constexpr >= 201304L
#define HANGUL_TABLE_QUALIFIER constexpr
#else
#define HANGUL_TABLE_QUALIFIER const
#endif

// ============================================================================
// 상수 정의
// ============================================================================

// 글리프 크기 상수
#define HANGUL_GLYPH_WIDTH  16
#define HANGUL_GLYPH_HEIGHT 16
#define HANGUL_BYTES_PER_GLYPH 32  // 16행 x 2바이트/행

// 폰트 구조 상수
#define HANGUL_CHO_COUNT    20    // 초성 개수 (빈 값 포함)
#define HANGUL_CHO_BUL      8     // 초성 벌 수
#define HANGUL_JUNG_COUNT   22    // 중성 개수 (빈 값 포함)
#define HANGUL_JUNG_BUL     4     // 중성 벌 수
#define HANGUL_JONG_COUNT   28    // 종성 개수 (빈 값 포함)
#define HANGUL_JONG_BUL     4     // 종성 벌 수

// 폰트 섹션 오프셋
#define HANGUL_CHO_OFFSET   0
#define HANGUL_JUNG_OFFSET  (HANGUL_CHO_COUNT * HANGUL_CHO_BUL)    // 160
#define HANGUL_JONG_OFFSET  (HANGUL_JUNG_OFFSET + HANGUL_JUNG_COUNT * HANGUL_JUNG_BUL)  // 248

// 총 글리프 수
#define HANGUL_TOTAL_GLYPHS (HANGUL_CHO_COUNT * HANGUL_CHO_BUL + \
                           HANGUL_JUNG_COUNT * HANGUL_JUNG_BUL + \
                           HANGUL_JONG_COUNT * HANGUL_JONG_BUL)  // 360

// 현대 한글 음절 범위
#define HANGUL_SYLLABLE_FIRST 0xAC00u
#define HANGUL_SYLLABLE_LAST  0xD7A3u

// ============================================================================
// 타입 정의
// ============================================================================

// 한글 크기 옵션
enum HangulSize {
    HG_SIZE_NORMAL = 0,
    HG_SIZE_H2     = 2,    // 가로 2배
    HG_SIZE_V2     = 3,    // 세로 2배
    HG_SIZE_X4     = 4     // 가로세로 2배
};

// 논리 색상 모드
// 주의: 이 값 자체는 디스플레이 드라이버의 색상 값이 아니다.
//       어댑터가 setColorResolver()로 드라이버 값 변환을 등록해야 한다.
enum HangulColor {
    HG_COLOR_BLACK = 0,
    HG_COLOR_WHITE = 1,
    HG_COLOR_INVERT = 2
};

// 폰트 정보 구조체
// 초기화 순서: name, width, height, choData, jungData, jongData
struct HangulFontInfo {
    const char* name;
    uint8_t width;
    uint8_t height;
    const uint8_t* choData;
    const uint8_t* jungData;
    const uint8_t* jongData;
};

// 한글 구성 요소 구조체
// 여기 담기는 값은 유니코드 분해 결과 그대로다.
// 폰트 슬롯을 찾을 때 초성/중성은 +1 해서 접근한다 (슬롯 0은 빈 글리프).
struct HangulComponents {
    uint8_t cho;      // 초성 인덱스 (0~18)
    uint8_t jung;     // 중성 인덱스 (0~20)
    uint8_t jong;     // 종성 인덱스 (0~27, 0은 받침 없음)
    uint8_t choBul;   // 초성 벌 (0~7)
    uint8_t jungBul;  // 중성 벌 (0~3)
    uint8_t jongBul;  // 종성 벌 (0~3)
};

// 글리프 데이터 구조체
struct HangulGlyphSet {
    const uint8_t* cho;   // 초성 글리프 (32바이트)
    const uint8_t* jung;  // 중성 글리프 (32바이트)
    const uint8_t* jong;  // 종성 글리프 (32바이트, nullptr이면 받침 없음)
};

// UTF-8 디코딩 결과
struct HangulUtf8Char {
    uint32_t codepoint;  // valid가 true일 때만 의미가 있다
    uint8_t  length;     // 소비할 바이트 수 (무효 입력은 1, 읽을 것이 없으면 0)
    bool     valid;      // 유효한 코드포인트인지
};

// ============================================================================
// 함수 구현
// ============================================================================

namespace HangulCore {

    // 길이를 지정하지 않는 NUL 종료 문자열용 상한.
    // 디코더는 NUL을 연속 바이트로 인정하지 않으므로 NUL을 넘어서 읽지 않는다.
    static const uint16_t UTF8_NUL_TERMINATED = 0xFFFFu;

    // 초성 벌 선택 Lookup 테이블 (받침 없을 때) - 인덱스 0~20
    // 원본: { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3, 1, 2, 4, 4, 4, 2, 1, 3, 0 } (인덱스 1~21 사용)
    static HANGUL_TABLE_QUALIFIER uint8_t CHO_BUL_NO_JONG[] PROGMEM = {
        0, 0, 0, 0, 0, 0, 0, 0,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (인덱스 0~7)
        1, 3, 3, 3, 1,           // ㅗㅘㅙㅚㅛ (인덱스 8~12)
        2, 4, 4, 4, 2,           // ㅜㅝㅞㅟㅠ (인덱스 13~17)
        1, 3, 0                  // ㅡㅢㅣ (인덱스 18~20)
    };

    // 초성 벌 선택 Lookup 테이블 (받침 있을 때) - 인덱스 0~20
    // 원본: { 0, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 6, 6, 7, 7, 7, 6, 6, 7, 5 } (인덱스 1~21 사용)
    static HANGUL_TABLE_QUALIFIER uint8_t CHO_BUL_WITH_JONG[] PROGMEM = {
        5, 5, 5, 5, 5, 5, 5, 5,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (인덱스 0~7)
        6, 7, 7, 7, 6,           // ㅗㅘㅙㅚㅛ (인덱스 8~12)
        6, 7, 7, 7, 6,           // ㅜㅝㅞㅟㅠ (인덱스 13~17)
        6, 7, 5                  // ㅡㅢㅣ (인덱스 18~20)
    };

    // 종성 벌 선택 Lookup 테이블 (중성에 따라) - 인덱스 0~20
    // 원본: { 0, 0, 2, 0, 2, 1, 2, 1, 2, 3, 0, 2, 1, 3, 3, 1, 2, 1, 3, 3, 1, 1 } (인덱스 1~21 사용)
    static HANGUL_TABLE_QUALIFIER uint8_t JONG_BUL[] PROGMEM = {
        0, 2, 0, 2, 1, 2, 1, 2,  // ㅏㅐㅑㅒㅓㅔㅕㅖ (인덱스 0~7)
        3, 0, 2, 1, 3,           // ㅗㅘㅙㅚㅛ (인덱스 8~12)
        3, 1, 2, 1, 3,           // ㅜㅝㅞㅟㅠ (인덱스 13~17)
        3, 1, 1                  // ㅡㅢㅣ (인덱스 18~20)
    };

    // 벌 선택표는 AVR에서 Flash에 두고 읽는다 (번역 단위당 SRAM 63바이트 절약).
    HANGUL_CONSTEXPR uint8_t readBulTable(const uint8_t* table, uint8_t index) {
        return pgm_read_byte(table + index);
    }

    // 초성 벌 선택 로직
    HANGUL_CONSTEXPR uint8_t getChosungBul(uint8_t jung, bool hasJong) {
        return (jung > 20)
            ? (uint8_t)0                                    // 범위 초과 방지
            : readBulTable(hasJong ? CHO_BUL_WITH_JONG : CHO_BUL_NO_JONG, jung);
    }

    // 중성 벌 선택 로직
    // 초성이 ㄱ(인덱스 0) 또는 ㅋ(인덱스 15)이면 1벌/3벌, 그 외는 2벌/4벌
    HANGUL_CONSTEXPR uint8_t getJungsungBul(uint8_t cho, bool hasJong) {
        return (cho == 0 || cho == 15)
            ? (hasJong ? (uint8_t)2 : (uint8_t)0)
            : (hasJong ? (uint8_t)3 : (uint8_t)1);
    }

    // 종성 벌 선택 로직
    HANGUL_CONSTEXPR uint8_t getJongsungBul(uint8_t jung) {
        return (jung > 20) ? (uint8_t)0 : readBulTable(JONG_BUL, jung);
    }

    // ------------------------------------------------------------------
    // UTF-8 디코딩
    // ------------------------------------------------------------------

    HANGUL_CONSTEXPR bool isUtf8Continuation(uint8_t b) {
        return (b & 0xC0) == 0x80;
    }

    /**
     * UTF-8 한 글자를 디코딩한다. 문자열 전체를 복사하거나 할당하지 않는다.
     *
     * @param p         읽기 시작 위치
     * @param available p에서 읽어도 되는 바이트 수.
     *                  NUL 종료 문자열은 UTF8_NUL_TERMINATED를 넘긴다.
     *                  (NUL은 연속 바이트가 아니므로 NUL을 넘어서 읽지 않는다.)
     *
     * 검증 항목: 1~4바이트 길이, 연속 바이트, 최단 인코딩, surrogate 제외,
     *            U+10FFFF 상한. 4바이트 값도 16비트로 잘리지 않는다.
     *
     * 무효 입력이면 valid=false, length=1을 돌려준다(1바이트만 소비).
     * 읽을 것이 없으면 length=0이다.
     */
    HANGUL_CONSTEXPR HangulUtf8Char decodeUtf8(const char* p, uint16_t available) {
        HangulUtf8Char result = { 0u, 0, false };

        if (!p || available == 0) {
            return result;
        }

        // 무효로 판정되면 1바이트만 소비한다.
        result.length = 1;

        const uint8_t lead = (uint8_t)p[0];
        uint8_t continuationCount = 0;
        uint32_t codepoint = 0;
        uint32_t minCodepoint = 0;

        if (lead < 0x80) {                     // 0xxxxxxx : ASCII
            result.codepoint = lead;
            result.valid = true;
            return result;
        } else if ((lead & 0xE0) == 0xC0) {    // 110xxxxx : 2바이트
            continuationCount = 1;
            codepoint = (uint32_t)(lead & 0x1F);
            minCodepoint = 0x80u;
        } else if ((lead & 0xF0) == 0xE0) {    // 1110xxxx : 3바이트
            continuationCount = 2;
            codepoint = (uint32_t)(lead & 0x0F);
            minCodepoint = 0x800u;
        } else if ((lead & 0xF8) == 0xF0) {    // 11110xxx : 4바이트
            continuationCount = 3;
            codepoint = (uint32_t)(lead & 0x07);
            minCodepoint = 0x10000u;
        } else {                               // 단독 연속 바이트 또는 0xF8~0xFF
            return result;
        }

        for (uint8_t i = 1; i <= continuationCount; i++) {
            if (i >= available) {              // 전달 길이 밖 - 읽지 않는다
                return result;
            }
            const uint8_t b = (uint8_t)p[i];   // NUL이면 아래 검사에서 걸린다
            if (!isUtf8Continuation(b)) {
                return result;
            }
            codepoint = (codepoint << 6) | (uint32_t)(b & 0x3F);
        }

        if (codepoint < minCodepoint) {                         // 최단 인코딩 위반
            return result;
        }
        if (codepoint >= 0xD800u && codepoint <= 0xDFFFu) {     // surrogate
            return result;
        }
        if (codepoint > 0x10FFFFu) {                            // 상한 초과
            return result;
        }

        result.codepoint = codepoint;
        result.length = (uint8_t)(continuationCount + 1);
        result.valid = true;
        return result;
    }

    // ------------------------------------------------------------------
    // 한글 판별 / 분해
    // ------------------------------------------------------------------

    HANGUL_CONSTEXPR bool isHangulCodepoint(uint32_t codepoint) {
        return codepoint >= HANGUL_SYLLABLE_FIRST && codepoint <= HANGUL_SYLLABLE_LAST;
    }

    HANGUL_CONSTEXPR HangulComponents emptyComponents() {
        return HangulComponents{ 0, 0, 0, 0, 0, 0 };
    }

    /**
     * 코드포인트를 초중종성과 벌로 분해한다.
     * @return 현대 한글 음절이면 true. false면 out은 의미 없는 0 값이다.
     */
    HANGUL_CONSTEXPR bool tryDecomposeCodepoint(uint32_t codepoint, HangulComponents& out) {
        out = emptyComponents();
        if (!isHangulCodepoint(codepoint)) {
            return false;
        }

        const uint16_t code = (uint16_t)(codepoint - HANGUL_SYLLABLE_FIRST);

        out.cho = (uint8_t)(code / 588);          // 초성 (0~18)
        out.jung = (uint8_t)((code % 588) / 28);  // 중성 (0~20)
        out.jong = (uint8_t)(code % 28);          // 종성 (0~27, 0은 받침없음)

        const bool hasJong = (out.jong > 0);
        out.choBul = getChosungBul(out.jung, hasJong);
        out.jungBul = getJungsungBul(out.cho, hasJong);
        if (hasJong) {
            out.jongBul = getJongsungBul(out.jung);
        }
        return true;
    }

    /**
     * UTF-8 3바이트를 분해한다. 성공 여부를 명시적으로 돌려준다.
     * (0,0,0)은 정상적인 '가'이므로 실패 표식이 될 수 없다.
     * @return 정확히 3바이트로 끝나는 유효한 현대 한글이면 true
     */
    HANGUL_CONSTEXPR bool tryDecompose(uint8_t byte1, uint8_t byte2, uint8_t byte3,
                                       HangulComponents& out) {
        const char buffer[3] = { (char)byte1, (char)byte2, (char)byte3 };
        const HangulUtf8Char decoded = decodeUtf8(buffer, 3);
        if (!decoded.valid || decoded.length != 3) {
            out = emptyComponents();
            return false;
        }
        return tryDecomposeCodepoint(decoded.codepoint, out);
    }

    // UTF-8 한글 문자인지 확인 (유효한 3바이트 현대 한글만 true)
    HANGUL_CONSTEXPR bool isHangul(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
        HangulComponents unused = emptyComponents();
        return tryDecompose(byte1, byte2, byte3, unused);
    }

    /**
     * 호환 래퍼. 실패해도 0으로 채운 구조체를 돌려주므로
     * 반환값만으로 성공 여부를 판정하면 안 된다 ('가'와 구분되지 않는다).
     * 새 코드는 tryDecompose()를 쓸 것.
     */
    HANGUL_CONSTEXPR HangulComponents decompose(uint8_t byte1, uint8_t byte2, uint8_t byte3) {
        HangulComponents comp = emptyComponents();
        tryDecompose(byte1, byte2, byte3, comp);
        return comp;
    }

    // ------------------------------------------------------------------
    // 문자열 탐색
    // ------------------------------------------------------------------

    /**
     * NUL 종료 문자열에서 다음 한글 문자의 시작 주소를 찾는다.
     * 잘못된 바이트는 1바이트씩 건너뛰며, NUL을 넘어서 읽지 않는다.
     */
    HANGUL_CONSTEXPR const char* findNextHangul(const char* utf8String) {
        if (!utf8String) return nullptr;

        const char* p = utf8String;
        while (*p) {
            const HangulUtf8Char decoded = decodeUtf8(p, UTF8_NUL_TERMINATED);
            if (decoded.length == 0) break;
            if (decoded.valid && isHangulCodepoint(decoded.codepoint)) {
                return p;
            }
            p += decoded.length;
        }
        return nullptr;
    }

    /**
     * 길이를 지정한 버퍼에서 다음 한글 문자의 시작 주소를 찾는다.
     * 전달한 길이 밖은 읽지 않는다.
     */
    HANGUL_CONSTEXPR const char* findNextHangul(const char* utf8Data, uint16_t length) {
        if (!utf8Data) return nullptr;

        const char* p = utf8Data;
        uint16_t remaining = length;
        while (remaining > 0) {
            const HangulUtf8Char decoded = decodeUtf8(p, remaining);
            if (decoded.length == 0) break;
            if (decoded.valid && isHangulCodepoint(decoded.codepoint)) {
                return p;
            }
            p += decoded.length;
            remaining = (uint16_t)(remaining - decoded.length);
        }
        return nullptr;
    }

} // namespace HangulCore

// ============================================================================
// 통합 어댑터
// ============================================================================

// 픽셀 그리기 콜백 함수 타입
typedef void (*PixelDrawCallback)(int16_t x, int16_t y, uint16_t color);

// 논리 색상 -> 드라이버 색상 변환 콜백.
// 어댑터가 등록하면 논리 색상이 이 함수를 통해 드라이버 값으로 바뀐다.
typedef uint16_t (*LogicalColorResolver)(HangulColor logical);

class hangulDisp {
private:
    int16_t cursorX;
    int16_t cursorY;
    HangulSize textSize;
    HangulColor textColor;
    uint16_t textColorRaw;
    bool useRawColor;
    PixelDrawCallback drawPixelCallback;
    LogicalColorResolver colorResolver;
    const uint8_t* choData;
    const uint8_t* jungData;
    const uint8_t* jongData;
    bool fontReady;

public:
    // 생성자 - 픽셀 그리기 콜백 함수 필요
    explicit hangulDisp(PixelDrawCallback callback)
        : cursorX(0), cursorY(0),
          textSize(HG_SIZE_NORMAL),
          textColor(HG_COLOR_BLACK),
          textColorRaw(0),
          useRawColor(false),
          drawPixelCallback(callback),
          colorResolver(nullptr),
          choData(nullptr),
          jungData(nullptr),
          jongData(nullptr),
          fontReady(false) {}

    // ------------------------------------------------------------------
    // 설정
    // ------------------------------------------------------------------

    void setCursor(int16_t x, int16_t y) {
        cursorX = x;
        cursorY = y;
    }

    void setTextSize(HangulSize size) {
        textSize = size;
    }

    // 논리 색상 지정. 드라이버 값 변환은 setColorResolver()가 담당한다.
    void setTextColor(HangulColor color) {
        textColor = color;
        useRawColor = false;
    }

    // 원시 색상 지정. 값은 콜백까지 그대로 전달된다.
    void setTextColor(uint16_t color) {
        textColorRaw = color;
        useRawColor = true;
    }

    // 논리 색상 -> 드라이버 색상 변환기 등록 (어댑터가 사용)
    void setColorResolver(LogicalColorResolver resolver) {
        colorResolver = resolver;
    }

    int16_t getCursorX() const { return cursorX; }
    int16_t getCursorY() const { return cursorY; }

    bool isRawColor() const { return useRawColor; }
    HangulColor getLogicalColor() const { return textColor; }
    uint16_t getRawColor() const { return textColorRaw; }

    /**
     * 폰트 지정.
     * 16x16이 아니거나 필수 포인터가 없으면 폰트를 무효화하고 그대로 반환한다.
     * 이후 출력 호출은 콜백도 부르지 않고 커서도 움직이지 않는다.
     * 성공 여부는 isFontReady()로 확인한다.
     */
    void setFont(const HangulFontInfo& font) {
        if (font.width != HANGUL_GLYPH_WIDTH ||
            font.height != HANGUL_GLYPH_HEIGHT ||
            font.choData == nullptr ||
            font.jungData == nullptr ||
            font.jongData == nullptr) {
            clearFont();
            return;
        }
        choData = font.choData;
        jungData = font.jungData;
        jongData = font.jongData;
        fontReady = true;
    }

    // 폰트를 무효화한다 (데이터 포인터도 비운다)
    void clearFont() {
        choData = nullptr;
        jungData = nullptr;
        jongData = nullptr;
        fontReady = false;
    }

    // 폰트가 유효하게 설정되었는지
    bool isFontReady() const { return fontReady; }

    // 출력 가능한 상태인지 (폰트와 콜백이 모두 준비됨)
    bool isReadyToDraw() const { return fontReady && drawPixelCallback != nullptr; }

    // ------------------------------------------------------------------
    // 출력
    // ------------------------------------------------------------------

    // UTF-8 NUL 종료 문자열 출력. NUL을 넘어서 읽지 않는다.
    void print(const char* utf8Text) {
        if (!utf8Text || !isReadyToDraw()) return;

        const char* p = utf8Text;
        while (*p) {
            const HangulUtf8Char decoded =
                HangulCore::decodeUtf8(p, HangulCore::UTF8_NUL_TERMINATED);
            if (decoded.length == 0) break;
            emitCodepoint(decoded);
            p += decoded.length;
        }
    }

    // UTF-8 길이 지정 출력. 전달한 길이 밖을 읽지 않는다.
    void print(const char* utf8Data, uint16_t length) {
        if (!utf8Data || !isReadyToDraw()) return;

        const char* p = utf8Data;
        uint16_t remaining = length;
        while (remaining > 0) {
            const HangulUtf8Char decoded = HangulCore::decodeUtf8(p, remaining);
            if (decoded.length == 0) break;
            emitCodepoint(decoded);
            p += decoded.length;
            remaining = (uint16_t)(remaining - decoded.length);
        }
    }

    // 좌표와 원시 색상을 포함한 출력
    void print(int16_t x, int16_t y, const char* utf8Text, uint16_t color) {
        setCursor(x, y);
        setTextColor(color);
        print(utf8Text);
    }

    // 좌표와 논리 색상을 포함한 출력
    void print(int16_t x, int16_t y, const char* utf8Text, HangulColor color) {
        setCursor(x, y);
        setTextColor(color);
        print(utf8Text);
    }

    /**
     * 단일 한글 문자 출력 (UTF-8 3바이트).
     * 무효 입력, 준비되지 않은 폰트, null 콜백에서는
     * 글리프 접근·픽셀 출력·커서 이동을 모두 하지 않는다.
     */
    void printHangulChar(uint8_t b1, uint8_t b2, uint8_t b3) {
        if (!isReadyToDraw()) return;

        HangulComponents comp = HangulCore::emptyComponents();
        if (!HangulCore::tryDecompose(b1, b2, b3, comp)) return;

        drawSyllable(comp);
    }

private:
    // 디코딩 결과 하나를 화면에 반영한다 (문자 처리 계약 구현)
    void emitCodepoint(const HangulUtf8Char& decoded) {
        if (!decoded.valid) {
            return;  // 잘못된 인코딩: 출력 없음 (호출자가 1바이트 소비)
        }

        HangulComponents comp = HangulCore::emptyComponents();
        if (HangulCore::tryDecomposeCodepoint(decoded.codepoint, comp)) {
            drawSyllable(comp);
            return;
        }

        if (decoded.codepoint == 0x20) {   // 공백: 배율에 맞춰 전진만
            advanceCursor();
            return;
        }

        // 그 외 ASCII(CR/LF 포함)와 미지원 비ASCII: 소비만 하고 아무것도 하지 않는다
    }

    void drawSyllable(const HangulComponents& comp) {
        const HangulGlyphSet glyphs = getGlyphSet(comp);
        drawCombinedGlyph(cursorX, cursorY, glyphs);
        advanceCursor();
    }

    HangulGlyphSet getGlyphSet(const HangulComponents& comp) const {
        HangulGlyphSet glyphs = { nullptr, nullptr, nullptr };
        // 초성과 중성은 폰트 슬롯 0이 빈 글리프이므로 +1 해서 접근한다.
        // 종성은 분해 결과 0이 "받침 없음"이므로 그대로 쓴다.
        glyphs.cho = getGlyphPointer(choData, (uint8_t)(comp.cho + 1), comp.choBul, HANGUL_CHO_COUNT);
        glyphs.jung = getGlyphPointer(jungData, (uint8_t)(comp.jung + 1), comp.jungBul, HANGUL_JUNG_COUNT);
        if (comp.jong > 0) {
            glyphs.jong = getGlyphPointer(jongData, comp.jong, comp.jongBul, HANGUL_JONG_COUNT);
        }
        return glyphs;
    }

    const uint8_t* getGlyphPointer(const uint8_t* base, uint8_t index, uint8_t bul,
                                   uint8_t indexCount) const {
        if (!base) {
            return nullptr;
        }
        // 폰트 데이터는 벌 우선 정렬: (벌 * 자음/모음개수) + 인덱스
        const uint16_t glyphIndex = (uint16_t)((uint16_t)bul * indexCount + index);
        const uint16_t offset = (uint16_t)(glyphIndex * HANGUL_BYTES_PER_GLYPH);
        return base + offset;
    }

    // 행 단위로 읽어 OR 합성한다.
    // 한글 1자당 소스 수준 바이트 읽기: 받침 없음 64회, 받침 있음 96회.
    void drawCombinedGlyph(int16_t x, int16_t y, const HangulGlyphSet& glyphs) {
        const uint8_t scaleX = (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4) ? 2 : 1;
        const uint8_t scaleY = (textSize == HG_SIZE_V2 || textSize == HG_SIZE_X4) ? 2 : 1;
        const uint16_t color = resolveColor();

        for (int16_t row = 0; row < HANGUL_GLYPH_HEIGHT; row++) {
            uint16_t bits = 0;
            if (glyphs.cho)  bits |= readGlyphRow(glyphs.cho, row);
            if (glyphs.jung) bits |= readGlyphRow(glyphs.jung, row);
            if (glyphs.jong) bits |= readGlyphRow(glyphs.jong, row);

            if (bits == 0) continue;

            for (int16_t col = 0; col < HANGUL_GLYPH_WIDTH; col++) {
                if ((bits & (uint16_t)(0x8000u >> col)) == 0) continue;
                for (uint8_t sy = 0; sy < scaleY; sy++) {
                    for (uint8_t sx = 0; sx < scaleX; sx++) {
                        drawPixelCallback((int16_t)(x + col * scaleX + sx),
                                          (int16_t)(y + row * scaleY + sy),
                                          color);
                    }
                }
            }
        }
    }

    static uint16_t readGlyphRow(const uint8_t* glyph, int16_t row) {
        return (uint16_t)(((uint16_t)pgm_read_byte(glyph + row * 2) << 8) |
                          (uint16_t)pgm_read_byte(glyph + row * 2 + 1));
    }

    void advanceCursor() {
        const uint8_t scaleX = (textSize == HG_SIZE_H2 || textSize == HG_SIZE_X4) ? 2 : 1;
        cursorX = (int16_t)(cursorX + HANGUL_GLYPH_WIDTH * scaleX);
    }

    uint16_t resolveColor() const {
        if (useRawColor) {
            return textColorRaw;   // 원시 값은 그대로 전달한다
        }
        if (colorResolver) {
            return colorResolver(textColor);
        }
        // 변환기가 없으면 논리 코드를 그대로 넘긴다 (드라이버 값이 아님).
        return (uint16_t)textColor;
    }
};

#endif // HANGUL_DISP_H
