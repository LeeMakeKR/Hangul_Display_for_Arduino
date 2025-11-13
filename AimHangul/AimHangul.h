/*
 * AimHangul.h
 * 
 * AimHangul 라이브러리 메인 헤더
 * 모든 구성 요소를 포함하는 통합 헤더
 */

#ifndef AIM_HANGUL_H
#define AIM_HANGUL_H

// 핵심 모듈
#include "core/HangulCore.h"
#include "core/HangulTypes.h"

// 기본 폰트들
#include "fonts/ASCFont.h"
#include "fonts/KSFont.h"

// 한글 폰트들 (필요한 것만 포함)
// #include "fonts/hangul/H01_Font.h"
// #include "fonts/hangul/Apple00_Font.h"

// 어댑터들 (필요한 것만 포함)
#include "adapters/AimHangul_Base.h"
// #include "adapters/AimHangul_GxEPD2.h"
// #include "adapters/AimHangul_U8g2.h"
// #include "adapters/AimHangul_TFT_eSPI.h"

/*
사용 예제:

1. 필요한 폰트와 어댑터 포함:
   #include "AimHangul/fonts/hangul/H01_Font.h"
   #include "AimHangul/adapters/AimHangul_GxEPD2.h"

2. 객체 생성:
   AimHangul_GxEPD2<Font_H01> hangul(&display);

3. 한글 출력:
   hangul.setCursor(10, 30);
   hangul.print("안녕하세요!");
*/

#endif // AIM_HANGUL_H