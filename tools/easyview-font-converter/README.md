# EasyView 폰트 변환 도구

EasyView `.han` 폰트 파일을 Arduino 호환 `.h` 헤더 파일로 변환하는 도구입니다.

## 파일 목록

### han_to_h.py
단일 `.han` 파일을 `.h` 헤더로 변환합니다.

**사용법:**
```bash
python han_to_h.py <input.han> <output.h>
```

**예제:**
```bash
python han_to_h.py ../../EasyView-font/ko/H01_kr.han ../../HangulDisp/fonts/H01_kr.h
```

### convert_all.py
`EasyView-font/ko/` 폴더의 모든 `.han` 파일을 일괄 변환합니다.

**사용법:**
```bash
python convert_all.py
```

자동으로 다음 경로에서 파일을 찾아 변환합니다:
- 입력: `../../EasyView-font/ko/*.han`
- 출력: `../../HangulDisp/fonts/*.h`

## 변환 형식

### 입력 파일 (.han)
- 크기: 11,520 바이트
- 형식: EasyView 한글 폰트 바이너리
- 구조: 초성(5,120B) + 중성(2,816B) + 종성(3,584B)

### 출력 파일 (.h)
- 형식: C 헤더 파일
- PROGMEM 배열로 저장
- HangulFontInfo 구조체 생성
- 16×16 픽셀, 360개 글리프

## 폰트 데이터 구조

```
총 11,520 바이트
├─ 초성 (Cho):  0~5,119    (160 글리프 × 32바이트)
├─ 중성 (Jung): 5,120~7,935  (88 글리프 × 32바이트)
└─ 종성 (Jong): 7,936~11,519 (112 글리프 × 32바이트)
```

각 글리프: 16×16 픽셀 = 32 바이트 (MSB First)

## 자모 개수

| 구성 | 개수 | 벌 수 | 총 글리프 |
|------|------|-------|-----------|
| 초성 | 20개 | 8벌 | 160개 |
| 중성 | 22개 | 4벌 | 88개 |
| 종성 | 28개 | 4벌 | 112개 |

## 필요 사항

- Python 3.6 이상
- 입력 파일: EasyView `.han` 폰트
- 출력 경로: 쓰기 권한 필요

## 자세한 문서

변환 과정 및 폰트 구조에 대한 자세한 내용은 다음 문서를 참조하세요:
- [폰트 헤더 파일 형식 명세](../../FONT_HEADER_SPECIFICATION.md)
- [렌더링 규칙 명세](../../RENDERING_SPECIFICATION.md)
- [README_han_to_h.md](README_han_to_h.md) - 변환 도구 상세 설명
