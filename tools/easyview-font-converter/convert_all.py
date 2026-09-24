#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
모든 .han 파일을 .h 파일로 변환하는 스크립트

입출력 위치는 저장소 루트를 기준으로 계산하므로 어느 작업 디렉터리에서
실행해도 같은 폴더를 사용한다.

종료 코드
  0  변환 대상 전부 성공
  1  입력 폴더 없음 / 변환 대상 없음 / 하나 이상 변환 실패
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from han_to_h import HangulFontConverter  # noqa: E402

# tools/easyview-font-converter/convert_all.py -> 저장소 루트
REPO_ROOT = Path(__file__).resolve().parents[2]
INPUT_DIR = REPO_ROOT / 'EasyView-font' / 'ko'
OUTPUT_DIR = REPO_ROOT / 'HangulDisp' / 'fonts'


def main():
    input_dir = INPUT_DIR
    output_dir = OUTPUT_DIR

    # 입력 폴더 확인
    if not input_dir.is_dir():
        print(f"오류: 입력 폴더를 찾을 수 없습니다: {input_dir}")
        return 1

    # .han 파일 목록 가져오기
    han_files = sorted(p for p in input_dir.iterdir()
                       if p.is_file() and p.suffix.lower() == '.han')

    if not han_files:
        print(f"오류: .han 파일을 찾을 수 없습니다: {input_dir}")
        return 1

    # 출력 폴더 생성
    try:
        output_dir.mkdir(parents=True, exist_ok=True)
    except OSError as e:
        print(f"오류: 출력 폴더를 만들 수 없습니다: {output_dir} ({e})")
        return 1

    print("=== 한글 폰트 일괄 변환기 ===")
    print(f"저장소 루트: {REPO_ROOT}")
    print(f"입력 폴더: {input_dir}")
    print(f"출력 폴더: {output_dir}")
    print(f"변환할 파일 수: {len(han_files)}")
    print()

    # 변환 통계
    success_count = 0
    failed_files = []  # (파일명, 원인)

    # 각 파일 변환
    for i, han_path in enumerate(han_files, 1):
        output_path = output_dir / (han_path.stem + '.h')

        print(f"[{i}/{len(han_files)}] {han_path.name} -> {output_path.name}... ",
              end='', flush=True)

        try:
            converter = HangulFontConverter(str(han_path))

            if not converter.read_font_file():
                print("실패 (파일 읽기 오류)")
                failed_files.append((han_path.name, "파일 읽기 오류(크기 규격 위반 포함)"))
                continue

            if not converter.generate_header_file(str(output_path)):
                print("실패 (헤더 생성 오류)")
                failed_files.append((han_path.name, "헤더 생성 오류"))
                continue

            print("완료")
            success_count += 1

        except Exception as e:  # 예상 못한 오류도 실패로 집계한다
            print(f"실패 ({e})")
            failed_files.append((han_path.name, f"예외: {e}"))

    # 결과 출력
    print()
    print("=== 변환 완료 ===")
    print(f"성공: {success_count}개")
    print(f"실패: {len(failed_files)}개")

    if failed_files:
        print("\n실패한 파일:")
        for name, reason in failed_files:
            print(f"  - {name}: {reason}")

    print(f"\n출력 폴더: {output_dir}")

    return 1 if failed_files else 0


if __name__ == "__main__":
    sys.exit(main())
