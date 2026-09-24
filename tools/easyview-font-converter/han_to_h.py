#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
EasyView 조합형 한글 폰트(.han) to Arduino 헤더 파일(.h) 변환기

입력 규격(엄격):
  - .han 원본은 정확히 11,520바이트(360글리프 x 32바이트)여야 한다.
  - 크기가 다르면 읽기와 헤더 생성 모두 실패한다. 패딩하거나 자르지 않는다.
  - 검증 실패 시 출력 파일을 생성하거나 기존 파일을 덮어쓰지 않는다.
"""

import os
import sys
import tempfile
from datetime import datetime


class HangulFontConverter:
    """한글 폰트 변환 클래스"""

    # 폰트 구조 상수
    GLYPH_WIDTH = 16
    GLYPH_HEIGHT = 16
    BYTES_PER_GLYPH = 32  # 16행 x 2바이트/행

    # 섹션 구성
    CHO_COUNT = 20   # 초성 20자 (맨 앞 비어있음 포함)
    CHO_BUL = 8      # 초성 8벌
    JUNG_COUNT = 22  # 중성 22자 (맨 앞 비어있음 포함)
    JUNG_BUL = 4     # 중성 4벌
    JONG_COUNT = 28  # 종성 28자 (맨 앞 비어있음 포함)
    JONG_BUL = 4     # 종성 4벌

    # 오프셋
    CHO_OFFSET = 0
    JUNG_OFFSET = CHO_COUNT * CHO_BUL  # 160
    JONG_OFFSET = JUNG_OFFSET + JUNG_COUNT * JUNG_BUL  # 248

    # 총 글리프 수
    TOTAL_GLYPHS = CHO_COUNT * CHO_BUL + JUNG_COUNT * JUNG_BUL + JONG_COUNT * JONG_BUL  # 360

    # 예상 파일 크기 (이 값과 정확히 같아야 한다)
    EXPECTED_FILE_SIZE = TOTAL_GLYPHS * BYTES_PER_GLYPH  # 11,520 바이트

    # 생성 헤더에서 주석이 시작하는 열
    COMMENT_COLUMN = 76

    # 코어 헤더 파일 이름 (실제 파일명과 대소문자까지 일치해야 한다)
    CORE_HEADER = "HangulDisp.h"

    # 초성 이름 (인덱스 0은 비어있음)
    CHO_NAMES = [
        "empty", "g", "gg", "n", "d", "dd", "r", "m", "b", "bb",
        "s", "ss", "ng", "j", "jj", "ch", "k", "t", "p", "h"
    ]

    # 중성 이름 (인덱스 0은 비어있음)
    JUNG_NAMES = [
        "empty", "a", "ae", "ya", "yae", "eo", "e", "yeo", "ye",
        "o", "wa", "wae", "oe", "yo", "u", "wo", "we", "wi", "yu",
        "eu", "ui", "i"
    ]

    # 종성 이름 (인덱스 0은 비어있음)
    JONG_NAMES = [
        "empty", "g", "gg", "gs", "n", "nj", "nh", "d", "r", "rg",
        "rm", "rb", "rs", "rt", "rp", "rh", "m", "b", "bs", "s",
        "ss", "ng", "j", "ch", "k", "t", "p", "h"
    ]

    def __init__(self, input_file):
        """초기화

        Args:
            input_file: 입력 .han 파일 경로
        """
        self.input_file = input_file
        self.font_name = os.path.splitext(os.path.basename(input_file))[0]
        self.font_data = None

    # ------------------------------------------------------------------
    # 입력 검증
    # ------------------------------------------------------------------

    def read_font_file(self):
        """폰트 파일 읽기

        정확히 EXPECTED_FILE_SIZE 바이트인 경우에만 성공한다.
        실패하면 font_data를 무효(None) 상태로 되돌려 이전 성공 데이터가
        재사용되지 않게 한다.
        """
        # 실패 시 이전 데이터가 남지 않도록 먼저 무효화한다.
        self.font_data = None

        try:
            with open(self.input_file, 'rb') as f:
                data = f.read()
        except FileNotFoundError:
            print(f"오류: 파일을 찾을 수 없습니다: {self.input_file}")
            return False
        except OSError as e:
            print(f"오류: 파일 읽기 실패: {e}")
            return False

        if len(data) != self.EXPECTED_FILE_SIZE:
            print(f"오류: 폰트 파일 크기가 올바르지 않습니다. "
                  f"(필요: {self.EXPECTED_FILE_SIZE}바이트, 실제: {len(data)}바이트) "
                  f"- {self.input_file}")
            return False

        self.font_data = data
        print(f"폰트 파일 읽기 완료: {len(self.font_data)} 바이트")
        return True

    def has_valid_font_data(self):
        """현재 보유한 폰트 데이터가 규격에 맞는지 확인"""
        if self.font_data is None:
            print("오류: 폰트 데이터가 로드되지 않았습니다.")
            return False
        if len(self.font_data) != self.EXPECTED_FILE_SIZE:
            print(f"오류: 폰트 데이터 크기가 올바르지 않습니다. "
                  f"(필요: {self.EXPECTED_FILE_SIZE}바이트, 실제: {len(self.font_data)}바이트)")
            return False
        return True

    # ------------------------------------------------------------------
    # 글리프 접근 / 포맷팅
    # ------------------------------------------------------------------

    def get_glyph_data(self, glyph_index):
        """특정 글리프의 데이터 추출

        Args:
            glyph_index: 글리프 인덱스 (0~359)

        Returns:
            32바이트의 글리프 데이터 (데이터가 없거나 범위를 벗어나면 None)
        """
        if self.font_data is None:
            return None
        if glyph_index < 0 or glyph_index >= self.TOTAL_GLYPHS:
            return None

        offset = glyph_index * self.BYTES_PER_GLYPH
        return self.font_data[offset:offset + self.BYTES_PER_GLYPH]

    def format_byte_array(self, data, bytes_per_line=12):
        """바이트 배열을 C 형식으로 포맷팅

        Args:
            data: 바이트 데이터
            bytes_per_line: 한 줄당 바이트 수

        Returns:
            포맷팅된 문자열
        """
        lines = []
        for i in range(0, len(data), bytes_per_line):
            chunk = data[i:i + bytes_per_line]
            hex_values = ', '.join(f'0x{b:02X}' for b in chunk)
            lines.append(f'  {hex_values}')

        return ',\n'.join(lines)

    def _field_line(self, text, comment):
        """구조체 초기화 항목 한 줄을 주석 열에 맞춰 만든다."""
        line = '  ' + text
        pad = max(1, self.COMMENT_COLUMN - len(line))
        return f"{line}{' ' * pad}// {comment}\n"

    def _build_header_text(self):
        """헤더 파일 전체 내용을 문자열로 만든다."""
        guard_name = f"{self.font_name.upper()}_H"
        display_name = self.font_name.replace('_kr', '')
        out = []

        # 헤더 주석
        out.append("/**\n")
        out.append(f" * {self.font_name} - Korean Hangul Font for Arduino/ESP32\n")
        out.append(" * \n")
        out.append(f" * Converted from EasyView font file: {os.path.basename(self.input_file)}\n")
        out.append(f" * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        out.append(" * \n")
        out.append(" * Font Structure:\n")
        out.append(f" * - Glyph Size: {self.GLYPH_WIDTH}x{self.GLYPH_HEIGHT} pixels\n")
        out.append(f" * - Bytes per Glyph: {self.BYTES_PER_GLYPH} bytes\n")
        out.append(f" * - Total Glyphs: {self.TOTAL_GLYPHS}\n")
        out.append(f" * - Total Size: {len(self.font_data)} bytes\n")
        out.append(" * \n")
        out.append(" * Glyph Layout:\n")
        out.append(f" * - Cho (초성): {self.CHO_OFFSET}~{self.JUNG_OFFSET - 1} "
                   f"({self.CHO_COUNT} chars x {self.CHO_BUL} bul)\n")
        out.append(f" * - Jung (중성): {self.JUNG_OFFSET}~{self.JONG_OFFSET - 1} "
                   f"({self.JUNG_COUNT} chars x {self.JUNG_BUL} bul)\n")
        out.append(f" * - Jong (종성): {self.JONG_OFFSET}~{self.TOTAL_GLYPHS - 1} "
                   f"({self.JONG_COUNT} chars x {self.JONG_BUL} bul)\n")
        out.append(" * \n")
        out.append(" * 이 파일은 tools/easyview-font-converter/han_to_h.py가 생성한다.\n")
        out.append(" * 직접 수정하지 말고 변환기를 고친 뒤 다시 생성할 것.\n")
        out.append(" */\n\n")

        # Include guard
        out.append(f"#ifndef {guard_name}\n")
        out.append(f"#define {guard_name}\n\n")

        # 코어 헤더 (대소문자 구분 파일시스템에서도 맞아야 한다)
        out.append(f"#include \"{self.CORE_HEADER}\"\n\n")

        # 비트맵 데이터
        out.append("// Font bitmap data (MSB first, 16x16 pixels, 32 bytes per glyph)\n")
        out.append(f"const uint8_t {self.font_name}_Bitmaps[] PROGMEM = {{\n")
        out.append(self.format_byte_array(self.font_data, 12))
        out.append("\n};\n\n")

        # HangulFontInfo 인스턴스
        # 필드 순서: name, width, height, choData, jungData, jongData
        out.append("// Font info instance\n")
        out.append(f"const HangulFontInfo {self.font_name} = {{\n")
        out.append(self._field_line(f"\"{display_name}\",", "name"))
        out.append(self._field_line(f"{self.GLYPH_WIDTH},", "width"))
        out.append(self._field_line(f"{self.GLYPH_HEIGHT},", "height"))
        out.append(self._field_line(
            f"{self.font_name}_Bitmaps + (HANGUL_CHO_OFFSET * HANGUL_BYTES_PER_GLYPH),",
            "choData"))
        out.append(self._field_line(
            f"{self.font_name}_Bitmaps + (HANGUL_JUNG_OFFSET * HANGUL_BYTES_PER_GLYPH),",
            "jungData"))
        out.append(self._field_line(
            f"{self.font_name}_Bitmaps + (HANGUL_JONG_OFFSET * HANGUL_BYTES_PER_GLYPH)",
            "jongData"))
        out.append("};\n\n")

        out.append(f"#endif // {guard_name}\n")

        return ''.join(out)

    # ------------------------------------------------------------------
    # 출력
    # ------------------------------------------------------------------

    def generate_header_file(self, output_file=None):
        """헤더 파일 생성

        데이터 검증을 먼저 수행하고, 통과한 경우에만 출력 파일을 만든다.
        같은 폴더의 임시 파일에 기록한 뒤 교체하므로, 쓰기 도중 실패해도
        기존 헤더가 잘리거나 손상되지 않는다.

        Args:
            output_file: 출력 .h 파일 경로 (None이면 자동 생성)

        Returns:
            성공 여부
        """
        if not self.has_valid_font_data():
            return False

        if output_file is None:
            output_file = os.path.join(
                os.path.dirname(self.input_file),
                f"{self.font_name}.h"
            )

        content = self._build_header_text()

        out_dir = os.path.dirname(os.path.abspath(output_file))
        tmp_path = None
        try:
            os.makedirs(out_dir, exist_ok=True)
            fd, tmp_path = tempfile.mkstemp(prefix=f".{self.font_name}.",
                                            suffix=".h.tmp", dir=out_dir)
            with os.fdopen(fd, 'w', encoding='utf-8', newline='\n') as f:
                f.write(content)
            os.replace(tmp_path, output_file)
            tmp_path = None
        except OSError as e:
            print(f"오류: 헤더 파일 생성 실패: {e}")
            if tmp_path and os.path.exists(tmp_path):
                try:
                    os.remove(tmp_path)
                except OSError:
                    pass
            return False

        print(f"헤더 파일 생성 완료: {output_file}")
        print(f"파일 크기: ~{os.path.getsize(output_file)} 바이트")
        return True


def main():
    """메인 함수"""
    if len(sys.argv) < 2:
        print("사용법: python han_to_h.py <input.han> [output.h]")
        print("예제: python han_to_h.py Apple_kr.han")
        print("      python han_to_h.py Apple_kr.han Apple_kr.h")
        print(f"입력 파일은 정확히 {HangulFontConverter.EXPECTED_FILE_SIZE}바이트여야 합니다.")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None

    print("=== 한글 폰트 변환기 ===")
    print(f"입력 파일: {input_file}")

    converter = HangulFontConverter(input_file)

    # 폰트 파일 읽기
    if not converter.read_font_file():
        sys.exit(1)

    # 헤더 파일 생성
    if not converter.generate_header_file(output_file):
        sys.exit(1)

    print("\n변환 완료!")


if __name__ == "__main__":
    main()
