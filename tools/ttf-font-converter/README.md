# TTF 폰트 변환기 (Einstein Bacon Machine 사용 매뉴얼)

이 문서는 Einstein Bacon Machine을 사용해 TTF/OTF 폰트를 조합형 한글 비트맵 폰트로
변환하는 절차를 요약합니다. 프로그램의 상세 기능은 아래 링크를 기준으로 합니다.

## 준비

- 프로그램 다운로드: https://zikbakguri.itch.io/einstein-bacon-machine
- DirectX 드라이버 필요 (D3DX compile error가 발생하면 설치 필요)

## 기본 작업 흐름

1) 글꼴 설정
- 프로그램의 "글꼴" 메뉴에서 폰트 옵션을 설정하고 TTF 파일을 불러옵니다.

2) 글자 선택 및 마스크 편집
- 변환할 글자를 마우스로 선택합니다.
- 좌측 "글자 마스크" 패널에서 마우스 드래그로 마스크 영역을 지정합니다.
- SHIFT: 영역 추가, ALT: 영역 제거
- 편집 후 "마스크 적용" 버튼을 눌러 반영합니다.

3) 미리보기
- "프로젝트" 메뉴에서 "미리보기"로 폰트 결과를 확인합니다.
- ESC로 미리보기 종료

4) 내보내기
- "프로젝트" 메뉴에서 "글꼴 내보내기"를 선택합니다.
- .png 또는 .h 헤더 파일로 출력 가능합니다.

## 주의 사항

- 프로그램 특성상 창을 최소화하거나 다른 화면으로 전환하면 드물게 텍스처가 해제되어
  오류가 발생할 수 있습니다. 작업 중에는 자주 저장하세요.

## HangulDisp에 적용 방법

- 내보낸 .h 헤더를 프로젝트에 추가합니다.
- 코드에서 해당 폰트 헤더를 include 합니다.
- HangulDisp 인스턴스에 폰트를 지정합니다.

```cpp
#include "HangulDisp.h"
#include "MyFont.h"

void setup() {
    hangul.setFont(MyFont);
}
```

- 폰트가 HangulDisp 포맷 규격(16x16, 1bpp, 360 글리프)을 만족하는지 확인하세요.
- 규격은 FONT_HEADER_SPECIFICATION.md에 정리되어 있습니다.

## 프로젝트 수정 계획 (문서 기록용)

아래 계획은 변환기에서 생성된 폰트를 HangulDisp에서 직접 사용하기 위한 작업 방향입니다.
코드는 아직 수정하지 않고, 설계 단계의 기록으로만 유지합니다.

1) 변환기 출력 포맷 정리
- Einstein Bacon Machine이 생성하는 헤더 포맷(예: DKB844 벌식, ASCII 포함)을 문서화합니다.
- HangulDisp용 최소 요구사항(글리프 크기, 비트 순서, 벌 배열 순서)을 정리합니다.

2) easyview-font-converter 확장 방향
- 기존 EasyView 전용 출력 구조와 신규 포맷을 공존시키는 방안을 문서화합니다.
- 출력 포맷 선택 옵션을 추가할지 여부를 결정합니다.

3) HangulDisp.h 확장 방향
- 새로운 폰트 구조체(예: DKB844/ASCII 포함)를 수용할 수 있도록 타입/로딩 로직 확장을 설계합니다.
- 기존 HangulFontInfo 포맷과 호환성을 유지할지 결정합니다.

4) 렌더링 규칙 정합성 확인
- DKB844 벌 선택 규칙이 현재 렌더링 테이블과 다를 경우, 별도 테이블을 분리할지 결정합니다.
- ASCII 렌더링(8x16) 지원 여부 및 출력 경로를 정의합니다.

5) 문서 업데이트
- FONT_HEADER_SPECIFICATION.md에 신규 포맷 사양을 추가하거나 별도 문서로 분리합니다.
- RENDERING_SPECIFICATION.md에 새로운 벌 규칙/렌더링 경로를 추가합니다.

## 참고 링크

- https://zikbakguri.itch.io/einstein-bacon-machine
- https://github.com/TandyRum1024/hangul-johab-render-gms
