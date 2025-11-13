# AimHangul Library Structure

이 문서는 AimHangul 라이브러리의 구조와 각 구성 요소의 역할을 설명합니다.

## 폴더 구조

```
AimHangul/
├── AimHangul.h                   # 메인 헤더 파일
│
├── core/                         # 핵심 로직 모듈
│   ├── HangulCore.h             # 한글 분해/조합 로직
│   └── HangulTypes.h            # 공통 타입 정의
│
├── fonts/                        # 폰트 데이터
│   ├── ASCFont.h                # ASCII 폰트 (기존)
│   ├── KSFont.h                 # KS 폰트 (기존)
│   └── hangul/                  # 한글 폰트들
│       ├── H01_Font.h           # H01 폰트
│       ├── Apple00_Font.h       # Apple00 폰트
│       └── [FontName]_Font.h    # 기타 폰트들
│
├── adapters/                     # 디스플레이 어댑터
│   ├── AimHangul_Base.h         # 공통 베이스 클래스
│   ├── AimHangul_GxEPD2.h       # GxEPD2 어댑터
│   ├── AimHangul_U8g2.h         # U8g2 어댑터
│   ├── AimHangul_TFT_eSPI.h     # TFT_eSPI 어댑터
│   ├── AimHangul_Adafruit_GFX.h # Adafruit GFX 어댑터
│   └── AimHangul_M5Stack.h      # M5Stack 어댑터
│
└── examples/                     # 사용 예제
    ├── GxEPD2_Example/
    ├── U8g2_Example/
    └── TFT_eSPI_Example/
```

## 구성 요소별 역할

### 1. core/ - 핵심 로직

#### HangulCore.h
- UTF-8 한글 문자 분해 (초성, 중성, 종성)
- 벌(Bul) 선택 로직
- 라이브러리 독립적인 순수 한글 처리 로직

#### HangulTypes.h
- 공통 상수 및 열거형 정의
- 폰트 정보 구조체
- 크기, 색상 옵션 정의

### 2. fonts/ - 폰트 데이터

#### 한글 폰트 (hangul/)
- 각 폰트는 독립된 네임스페이스
- PROGMEM 사용으로 플래시 메모리 저장
- 초성 8벌, 중성 4벌, 종성 4벌 데이터

#### 기존 폰트
- ASCFont.h: ASCII 문자 폰트
- KSFont.h: 기존 KS 완성형 한글 폰트

### 3. adapters/ - 디스플레이 어댑터

#### AimHangul_Base.h
- 모든 어댑터가 상속받는 공통 인터페이스
- 커서 위치, 텍스트 크기/색상 관리
- 글리프 데이터 접근 메서드

#### 개별 어댑터
- 각 디스플레이 라이브러리에 특화된 구현
- 템플릿 기반으로 폰트 선택 가능
- 해당 라이브러리의 API 사용

## 개발 가이드

### 새로운 폰트 추가

1. `.han` 파일을 `tools/han_to_h.py`로 변환
2. `fonts/hangul/[FontName]_Font.h` 생성
3. 네임스페이스는 `Font_[FontName]` 형식

### 새로운 어댑터 추가

1. `AimHangul_Base.h` 상속
2. 순수 가상 함수들 구현:
   - `print(const char* utf8Text)`
   - `drawHangulChar(const HangulComponents& comp)`
   - `drawBitmap(...)`

### 사용법

```cpp
// 1. 필요한 헤더 포함
#include "AimHangul/fonts/hangul/H01_Font.h"
#include "AimHangul/adapters/AimHangul_GxEPD2.h"

// 2. 어댑터 객체 생성 (템플릿으로 폰트 선택)
AimHangul_GxEPD2<Font_H01> hangul(&display);

// 3. 한글 출력
hangul.setCursor(10, 30);
hangul.print("안녕하세요!");
```

## 설계 원칙

1. **모듈 분리**: 로직, 데이터, 출력 완전 분리
2. **템플릿 활용**: 컴파일 타임 폰트 선택
3. **메모리 효율**: 사용하는 폰트만 링크
4. **확장성**: 새 폰트/어댑터 쉽게 추가 가능
5. **호환성**: 기존 코드와의 호환성 유지