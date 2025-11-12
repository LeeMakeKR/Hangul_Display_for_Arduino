# H01_kr 폰트 렌더링 예제

이 문서는 EasyView 조합형 한글 폰트를 Arduino 헤더 파일로 변환하고, 이를 사용하여 텍스트를 이미지로 렌더링한 예제입니다.

## 작업 순서

### 1. 폰트 변환 (`.han` → `.h`)

`han_to_h.py` 스크립트를 사용하여 EasyView 폰트 파일을 Arduino 헤더 파일로 변환했습니다.

```bash
python tools/han_to_h.py EasyView-font/ko/H01_kr.han
```

**출력 파일**: `EasyView-font/ko/H01_kr.h`
- 폰트 크기: 11,520 바이트 (360 글리프 × 32 바이트)
- 글리프 크기: 16×16 픽셀
- 구조: 초성 8벌, 중성 4벌, 종성 4벌

### 2. 텍스트 렌더링

`render_hangul.py` 스크립트를 작성하여 한글 텍스트를 비트맵 이미지로 렌더링했습니다.

#### 렌더링 원리
- 한글 음절을 초성, 중성, 종성으로 분해
- 중성의 모양에 따라 초성/중성/종성의 벌(bul) 선택
- 각 자모의 비트맵을 OR 연산으로 합성
- 최종 이미지로 출력

#### 실행 방법

```python
from render_hangul import HangulRenderer

renderer = HangulRenderer('EasyView-font/ko/H01_kr.han')
image = renderer.render_text('안녕하세요 한글폰트입니다', spacing=2)
image.save('output_h01.png')
```

### 3. 렌더링 결과

**텍스트**: "안녕하세요 한글폰트입니다"

**생성된 파일**:
- `output_h01.png` - 원본 크기 (232×16 픽셀)
- `output_h01_scaled.png` - 4배 확대 (928×64 픽셀)

## 폰트 구조 설명

### 조합형 한글의 벌(bul) 시스템

조합형 한글 폰트는 초성, 중성, 종성을 각각 여러 벌로 나누어 저장합니다.
벌은 한글의 구조(모음의 모양, 종성의 유무)에 따라 자모의 위치와 크기를 조정한 변형입니다.

#### 초성 벌 선택 규칙
```
중성 타입 0 (세로 모음: ㅏㅐㅑㅒㅓㅔㅕㅖㅣ) × 종성 유무:
  - 종성 없음: 벌 0
  - 종성 있음: 벌 1

중성 타입 1 (가로 모음: ㅗㅛㅜㅠㅡ) × 종성 유무:
  - 종성 없음: 벌 2
  - 종성 있음: 벌 3

중성 타입 2 (복합 모음: ㅘㅙㅚㅝㅞㅟㅢ) × 종성 유무:
  - 종성 없음: 벌 4
  - 종성 있음: 벌 5
```

### 데이터 레이아웃

```
오프셋    내용                개수      크기
------   ----------------   -------   -------
0        초성 (20자 × 8벌)   160개     5,120 바이트
5,120    중성 (22자 × 4벌)    88개     2,816 바이트
7,936    종성 (28자 × 4벌)   112개     3,584 바이트
------   ----------------   -------   -------
합계                         360개    11,520 바이트
```

## 도구 파일

### 1. `tools/han_to_h.py`
EasyView `.han` 파일을 Arduino `.h` 헤더 파일로 변환하는 스크립트

**기능**:
- 폰트 데이터를 PROGMEM 배열로 변환
- 글리프 접근을 위한 헬퍼 함수 생성
- 폰트 메타데이터 구조체 생성

### 2. `tools/render_hangul.py`
한글 텍스트를 비트맵 이미지로 렌더링하는 스크립트

**기능**:
- 한글 음절 분해 및 벌 선택
- 비트맵 합성
- PNG 이미지 출력

## 사용 예제

### 명령줄에서 사용

```bash
# 폰트 변환
python tools/han_to_h.py EasyView-font/ko/H01_kr.han

# 텍스트 렌더링
python tools/render_hangul.py EasyView-font/ko/H01_kr.han "안녕하세요" output.png
```

### Python 코드에서 사용

```python
from render_hangul import HangulRenderer

# 렌더러 생성
renderer = HangulRenderer('path/to/font.han')

# 텍스트 렌더링
image = renderer.render_text('한글 텍스트', spacing=2)

# 이미지 저장
image.save('output.png')

# 이미지 확대 (선택사항)
scaled = image.resize((image.width * 4, image.height * 4), Image.NEAREST)
scaled.save('output_scaled.png')
```

## 참고 사항

- 폰트 파일은 16×16 픽셀 고정 크기입니다.
- 한글 완성형 범위('가'~'힣')만 지원됩니다.
- 공백은 16픽셀 너비로 렌더링됩니다.
- 문자 간격은 `spacing` 매개변수로 조정할 수 있습니다.

## 필요한 패키지

```bash
pip install pillow numpy
```

## 라이선스

이 예제 코드는 EPD_Hangul 프로젝트의 일부입니다.
