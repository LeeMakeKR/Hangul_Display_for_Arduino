/*
 * SelfTest - HangulDisp 코어 자가진단
 *
 * 디스플레이 없이 동작한다. 캡처 콜백으로 픽셀 좌표·색상·호출 수·커서를
 * 확인하고 결과를 시리얼로 출력한다.
 *
 * 확인 항목
 *  - 정상 한글 합성과 커서 전진, 배율 4종
 *  - 미지원 문자(한자·자모·이모지)를 '가'로 그리지 않는지
 *  - 잘못된 UTF-8에서 범위 밖 읽기·무한 반복이 없는지
 *  - null 콜백, 잘못된 크기 폰트, null 포인터 폰트에서 아무것도 하지 않는지
 *  - 원시 색상과 논리 색상 두 경로
 *
 * 보드: 시리얼이 있는 아무 보드 (Uno/Nano/ESP32 등)
 * 시리얼 모니터: 115200bps, UTF-8
 */

#include <HangulDisp.h>
#include <HangulSelfTest.h>
#include <fonts/H01_kr.h>

static void reportCheck(const char* name, bool ok) {
    Serial.print(ok ? F("  [PASS] ") : F("  [FAIL] "));
#if defined(__AVR__)
    // AVR에서는 검사 이름이 Flash에 있다 (SRAM 절약)
    Serial.println((const __FlashStringHelper*)name);
#else
    Serial.println(name);
#endif
}

void setup() {
    Serial.begin(115200);
    delay(200);

    Serial.println();
    Serial.println(F("=== HangulDisp 자가진단 ==="));
    Serial.print(F("폰트: "));
    Serial.println(H01_kr.name);

    const HangulSelfTest::Result result = HangulSelfTest::runAll(H01_kr, reportCheck);

    Serial.println();
    Serial.print(F("통과: "));
    Serial.print(result.passed);
    Serial.print(F("  실패: "));
    Serial.println(result.failed);
    Serial.println(result.failed == 0 ? F("결과: 전부 통과") : F("결과: 실패 있음"));

    // 행 단위 합성이 픽셀 단위 기준 구현과 같은 결과를 내는지 확인한다.
    // step=1이면 11,172자 전부. Uno에서는 수십 초 걸리므로 기본은 표본 검사.
    Serial.println();
    Serial.println(F("행 단위 합성 동등성 시험 (표본 간격 37)..."));
    const uint32_t mismatch = HangulSelfTest::runGlyphSweep(H01_kr, 37);
    if (mismatch == 0) {
        Serial.println(F("  전부 일치"));
    } else {
        Serial.print(F("  불일치 코드포인트: U+"));
        Serial.println(mismatch, HEX);
    }
}

void loop() {
    // 자가진단은 setup()에서 한 번만 수행한다
}
