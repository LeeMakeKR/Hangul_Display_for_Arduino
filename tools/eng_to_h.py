#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
EasyView 영문 폰트(.eng) to Arduino 헤더 파일(.h) 변환기
"""

import sys
import os
from datetime import datetime

class EnglishFontConverter:
    """영문 폰트 변환 클래스"""
    
    # 폰트 구조 상수
    GLYPH_WIDTH = 16
    GLYPH_HEIGHT = 16
    BYTES_PER_GLYPH = 32  # 16행 × 2바이트/행
    
    # ASCII 폰트 구성
    ASCII_COUNT = 128  # ASCII 문자 수
    
    # 예상 파일 크기
    EXPECTED_FILE_SIZE = ASCII_COUNT * BYTES_PER_GLYPH  # 4,096 바이트
    
    def __init__(self, input_file):
        """초기화
        
        Args:
            input_file: 입력 .eng 파일 경로
        """
        self.input_file = input_file
        self.font_name = os.path.splitext(os.path.basename(input_file))[0]
        self.font_data = None
        
    def read_font_file(self):
        """폰트 파일 읽기"""
        try:
            with open(self.input_file, 'rb') as f:
                self.font_data = f.read()
            
            if len(self.font_data) != self.EXPECTED_FILE_SIZE:
                print(f"경고: 파일 크기가 예상과 다릅니다. (예상: {self.EXPECTED_FILE_SIZE}, 실제: {len(self.font_data)})")
            
            print(f"폰트 파일 읽기 완료: {len(self.font_data)} 바이트")
            return True
            
        except FileNotFoundError:
            print(f"오류: 파일을 찾을 수 없습니다: {self.input_file}")
            return False
        except Exception as e:
            print(f"오류: 파일 읽기 실패: {e}")
            return False
    
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
    
    def generate_header_file(self, output_file=None):
        """헤더 파일 생성
        
        Args:
            output_file: 출력 .h 파일 경로 (None이면 자동 생성)
            
        Returns:
            성공 여부
        """
        if self.font_data is None:
            print("오류: 폰트 데이터가 로드되지 않았습니다.")
            return False
        
        if output_file is None:
            output_file = os.path.join(
                os.path.dirname(self.input_file),
                f"{self.font_name}.h"
            )
        
        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                # 헤더 주석
                f.write(f"/**\n")
                f.write(f" * {self.font_name} - English ASCII Font for Arduino/ESP32\n")
                f.write(f" * \n")
                f.write(f" * Converted from EasyView font file: {os.path.basename(self.input_file)}\n")
                f.write(f" * Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f" * \n")
                f.write(f" * Font Structure:\n")
                f.write(f" * - Glyph Size: {self.GLYPH_WIDTH}x{self.GLYPH_HEIGHT} pixels\n")
                f.write(f" * - Bytes per Glyph: {self.BYTES_PER_GLYPH} bytes\n")
                f.write(f" * - Total Glyphs: {self.ASCII_COUNT} (ASCII 0-127)\n")
                f.write(f" * - Total Size: {len(self.font_data)} bytes\n")
                f.write(f" */\n\n")
                
                # Include guard
                guard_name = f"{self.font_name.upper()}_H"
                f.write(f"#ifndef {guard_name}\n")
                f.write(f"#define {guard_name}\n\n")
                
                # HangulDisp 헤더
                f.write(f"#include \"hangulDisp.h\"\n\n")
                
                # 비트맵 데이터
                f.write(f"// Font bitmap data (MSB first, 16x16 pixels, 32 bytes per glyph)\n")
                f.write(f"const uint8_t {self.font_name}_Bitmaps[] PROGMEM = {{\n")
                
                formatted_data = self.format_byte_array(self.font_data, 12)
                f.write(formatted_data)
                
                f.write(f"\n}};\n\n")
                
                # HangulFontInfo 인스턴스 생성 (ASCII 전용)
                f.write(f"// Font info instance\n")
                f.write(f"const HangulFontInfo {self.font_name} = {{\n")
                f.write(f"  \"{self.font_name.replace('_en', '')}\",")
                f.write(" " * (76 - len(f"\"{self.font_name.replace('_en', '')}\",") - 2))
                f.write(f"// name\n")
                f.write(f"  {self.GLYPH_WIDTH},")
                f.write(" " * (76 - len(f"{self.GLYPH_WIDTH},") - 2))
                f.write(f"// width\n")
                f.write(f"  {self.GLYPH_HEIGHT},")
                f.write(" " * (76 - len(f"{self.GLYPH_HEIGHT},") - 2))
                f.write(f"// height\n")
                f.write(f"  true,")
                f.write(" " * (76 - len(f"true,") - 2))
                f.write(f"// hasAscii\n")
                f.write(f"  {self.font_name}_Bitmaps,")
                f.write(" " * (76 - len(f"{self.font_name}_Bitmaps,") - 2))
                f.write(f"// asciiData (used as choData)\n")
                f.write(f"  nullptr,")
                f.write(" " * (76 - len(f"nullptr,") - 2))
                f.write(f"// jungData\n")
                f.write(f"  nullptr")
                f.write(" " * (77 - len(f"nullptr") - 2))
                f.write(f"// jongData\n")
                f.write(f"}};\n\n")
                
                # Include guard 종료
                f.write(f"#endif // {guard_name}\n")
            
            print(f"헤더 파일 생성 완료: {output_file}")
            print(f"파일 크기: ~{os.path.getsize(output_file)} 바이트")
            return True
            
        except Exception as e:
            print(f"오류: 헤더 파일 생성 실패: {e}")
            return False


def main():
    """메인 함수"""
    if len(sys.argv) < 2:
        print("사용법: python eng_to_h.py <input.eng> [output.h]")
        print("예제: python eng_to_h.py Normal_en.eng")
        print("      python eng_to_h.py Normal_en.eng Normal_en.h")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    print(f"=== 영문 폰트 변환기 ===")
    print(f"입력 파일: {input_file}")
    
    converter = EnglishFontConverter(input_file)
    
    # 폰트 파일 읽기
    if not converter.read_font_file():
        sys.exit(1)
    
    # 헤더 파일 생성
    if not converter.generate_header_file(output_file):
        sys.exit(1)
    
    print("\n변환 완료!")


if __name__ == "__main__":
    main()
