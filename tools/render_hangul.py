#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
한글 조합형 폰트를 사용하여 텍스트를 이미지로 렌더링하는 도구
"""

import struct
import numpy as np
from PIL import Image, ImageDraw

class HangulRenderer:
    """한글 렌더러 클래스"""
    
    # 폰트 구조 상수 (han_to_h.py와 동일)
    GLYPH_WIDTH = 16
    GLYPH_HEIGHT = 16
    BYTES_PER_GLYPH = 32
    
    CHO_COUNT = 20
    CHO_BUL = 8
    JUNG_COUNT = 22
    JUNG_BUL = 4
    JONG_COUNT = 28
    JONG_BUL = 4
    
    CHO_OFFSET = 0
    JUNG_OFFSET = CHO_COUNT * CHO_BUL  # 160
    JONG_OFFSET = JUNG_OFFSET + JUNG_COUNT * JUNG_BUL  # 248
    
    # 초성 테이블 (19자)
    CHO_TABLE = [
        'ㄱ', 'ㄲ', 'ㄴ', 'ㄷ', 'ㄸ', 'ㄹ', 'ㅁ', 'ㅂ', 'ㅃ', 'ㅅ',
        'ㅆ', 'ㅇ', 'ㅈ', 'ㅉ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ'
    ]
    
    # 중성 테이블 (21자)
    JUNG_TABLE = [
        'ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅗ', 'ㅘ',
        'ㅙ', 'ㅚ', 'ㅛ', 'ㅜ', 'ㅝ', 'ㅞ', 'ㅟ', 'ㅠ', 'ㅡ', 'ㅢ', 'ㅣ'
    ]
    
    # 종성 테이블 (27자, 0=없음)
    JONG_TABLE = [
        '', 'ㄱ', 'ㄲ', 'ㄳ', 'ㄴ', 'ㄵ', 'ㄶ', 'ㄷ', 'ㄹ', 'ㄺ',
        'ㄻ', 'ㄼ', 'ㄽ', 'ㄾ', 'ㄿ', 'ㅀ', 'ㅁ', 'ㅂ', 'ㅄ', 'ㅅ',
        'ㅆ', 'ㅇ', 'ㅈ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ'
    ]
    
    # 중성의 벌 선택 테이블 (중성 모양에 따라)
    # 0: ㅏㅐㅑㅒㅓㅔㅕㅖㅣ (세로 모음)
    # 1: ㅗㅛㅜㅠㅡ (가로 모음)
    # 2: ㅘㅙㅚㅝㅞㅟㅢ (복합 모음)
    JUNG_BUL_MAP = {
        'ㅏ': 0, 'ㅐ': 0, 'ㅑ': 0, 'ㅒ': 0, 'ㅓ': 0, 'ㅔ': 0, 'ㅕ': 0, 'ㅖ': 0, 'ㅣ': 0,
        'ㅗ': 1, 'ㅛ': 1, 'ㅜ': 1, 'ㅠ': 1, 'ㅡ': 1,
        'ㅘ': 2, 'ㅙ': 2, 'ㅚ': 2, 'ㅝ': 2, 'ㅞ': 2, 'ㅟ': 2, 'ㅢ': 2
    }
    
    def __init__(self, han_file):
        """초기화
        
        Args:
            han_file: .han 폰트 파일 경로
        """
        self.han_file = han_file
        self.font_data = None
        self.load_font()
    
    def load_font(self):
        """폰트 파일 로드"""
        with open(self.han_file, 'rb') as f:
            self.font_data = f.read()
        print(f"폰트 로드 완료: {len(self.font_data)} 바이트")
    
    def get_glyph_bitmap(self, glyph_index):
        """글리프 인덱스로 비트맵 추출
        
        Args:
            glyph_index: 글리프 인덱스
            
        Returns:
            16x16 numpy 배열 (0 또는 255)
        """
        offset = glyph_index * self.BYTES_PER_GLYPH
        glyph_data = self.font_data[offset:offset + self.BYTES_PER_GLYPH]
        
        # 16x16 비트맵 생성
        bitmap = np.zeros((self.GLYPH_HEIGHT, self.GLYPH_WIDTH), dtype=np.uint8)
        
        for row in range(self.GLYPH_HEIGHT):
            byte_offset = row * 2
            # MSB first, 2바이트 = 16비트
            high_byte = glyph_data[byte_offset]
            low_byte = glyph_data[byte_offset + 1]
            
            for col in range(8):
                if high_byte & (0x80 >> col):
                    bitmap[row, col] = 255
            for col in range(8):
                if low_byte & (0x80 >> col):
                    bitmap[row, col + 8] = 255
        
        return bitmap
    
    def decompose_hangul(self, char):
        """한글 음절을 초성, 중성, 종성으로 분해
        
        Args:
            char: 한글 음절 문자
            
        Returns:
            (cho_index, jung_index, jong_index) 튜플 (폰트 파일의 인덱스)
        """
        if not ('가' <= char <= '힣'):
            return None
        
        code = ord(char) - ord('가')
        
        # 유니코드 분해 (초성 0-18, 중성 0-20, 종성 0-27)
        cho_unicode = code // (21 * 28)
        jung_unicode = (code % (21 * 28)) // 28
        jong_unicode = code % 28
        
        # 폰트 파일 인덱스로 변환
        cho_index = cho_unicode + 1  # 폰트: 초성 인덱스 0은 비어있음
        jung_index = jung_unicode + 1  # 폰트: 중성 인덱스 0은 비어있음 (수정: +2 -> +1)
        jong_index = jong_unicode  # 폰트: 종성 인덱스 0은 종성 없음
        
        return cho_index, jung_index, jong_index
    
    def get_bul_indices(self, cho_index, jung_index, jong_index):
        """초성, 중성, 종성의 벌 인덱스 계산
        
        레퍼런스 문서의 벌 선택 규칙:
        - 초성: 1~8벌 (받침 없음 1~5, 받침 있음 6~8)
        - 중성: 1~4벌 (초성 종류와 받침 유무)
        - 종성: 1~4벌 (중성 종류)
        
        Args:
            cho_index: 초성 인덱스 (폰트 파일 인덱스, 1-19)
            jung_index: 중성 인덱스 (폰트 파일 인덱스, 1-21)
            jong_index: 종성 인덱스 (폰트 파일 인덱스, 0-27)
            
        Returns:
            (cho_bul, jung_bul, jong_bul) 튜플 (0부터 시작하는 인덱스)
        """
        # 중성 문자 확인
        jung_unicode_index = jung_index - 1
        jung_char = self.JUNG_TABLE[jung_unicode_index]
        
        # 종성 유무
        has_jong = jong_index > 0
        
        # 초성 벌 선택 (레퍼런스 기준)
        # 받침 없음:
        #   1벌: ㅏ, ㅐ, ㅑ, ㅒ, ㅓ, ㅔ, ㅕ, ㅖ, ㅣ
        #   2벌: ㅗ, ㅛ, ㅡ
        #   3벌: ㅜ, ㅠ
        #   4벌: ㅘ, ㅙ, ㅚ, ㅢ
        #   5벌: ㅝ, ㅞ, ㅟ
        # 받침 있음:
        #   6벌: ㅏ, ㅐ, ㅑ, ㅒ, ㅓ, ㅔ, ㅕ, ㅖ, ㅣ
        #   7벌: ㅗ, ㅛ, ㅜ, ㅠ, ㅡ
        #   8벌: ㅘ, ㅙ, ㅚ, ㅢ, ㅝ, ㅞ, ㅟ
        
        if not has_jong:
            if jung_char in ['ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅣ']:
                cho_bul = 0  # 1벌 -> 인덱스 0
            elif jung_char in ['ㅗ', 'ㅛ', 'ㅡ']:
                cho_bul = 1  # 2벌 -> 인덱스 1
            elif jung_char in ['ㅜ', 'ㅠ']:
                cho_bul = 2  # 3벌 -> 인덱스 2
            elif jung_char in ['ㅘ', 'ㅙ', 'ㅚ', 'ㅢ']:
                cho_bul = 3  # 4벌 -> 인덱스 3
            else:  # ㅝ, ㅞ, ㅟ
                cho_bul = 4  # 5벌 -> 인덱스 4
        else:
            if jung_char in ['ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅣ']:
                cho_bul = 5  # 6벌 -> 인덱스 5
            elif jung_char in ['ㅗ', 'ㅛ', 'ㅜ', 'ㅠ', 'ㅡ']:
                cho_bul = 6  # 7벌 -> 인덱스 6
            else:  # ㅘ, ㅙ, ㅚ, ㅢ, ㅝ, ㅞ, ㅟ
                cho_bul = 7  # 8벌 -> 인덱스 7
        
        # 중성 벌 선택 (레퍼런스 기준)
        # 1벌: 받침 없음 + 초성이 'ㄱ' 또는 'ㅋ'
        # 2벌: 받침 없음 + 초성이 'ㄱ, ㅋ' 이외
        # 3벌: 받침 있음 + 초성이 'ㄱ' 또는 'ㅋ'
        # 4벌: 받침 있음 + 초성이 'ㄱ, ㅋ' 이외
        
        # 초성 인덱스: ㄱ=1, ㅋ=16
        is_giyeok_or_kieuk = (cho_index == 1 or cho_index == 16)
        
        if not has_jong:
            jung_bul = 0 if is_giyeok_or_kieuk else 1  # 1벌 또는 2벌
        else:
            jung_bul = 2 if is_giyeok_or_kieuk else 3  # 3벌 또는 4벌
        
        # 종성 벌 선택 (레퍼런스 기준)
        # 1벌: ㅏ, ㅑ, ㅘ
        # 2벌: ㅓ, ㅕ, ㅚ, ㅝ, ㅟ, ㅢ, ㅣ
        # 3벌: ㅐ, ㅒ, ㅔ, ㅖ, ㅙ, ㅞ
        # 4벌: ㅗ, ㅛ, ㅜ, ㅠ, ㅡ
        
        if jong_index == 0:
            jong_bul = 0
        elif jung_char in ['ㅏ', 'ㅑ', 'ㅘ']:
            jong_bul = 0  # 1벌 -> 인덱스 0
        elif jung_char in ['ㅓ', 'ㅕ', 'ㅚ', 'ㅝ', 'ㅟ', 'ㅢ', 'ㅣ']:
            jong_bul = 1  # 2벌 -> 인덱스 1
        elif jung_char in ['ㅐ', 'ㅒ', 'ㅔ', 'ㅖ', 'ㅙ', 'ㅞ']:
            jong_bul = 2  # 3벌 -> 인덱스 2
        else:  # ㅗ, ㅛ, ㅜ, ㅠ, ㅡ
            jong_bul = 3  # 4벌 -> 인덱스 3
        
        return cho_bul, jung_bul, jong_bul
    
    def render_char(self, char):
        """한글 문자 하나를 렌더링
        
        Args:
            char: 한글 음절 문자
            
        Returns:
            16x16 numpy 배열
        """
        decomposed = self.decompose_hangul(char)
        if decomposed is None:
            # 한글이 아닌 경우 빈 비트맵 반환
            return np.zeros((self.GLYPH_HEIGHT, self.GLYPH_WIDTH), dtype=np.uint8)
        
        cho_index, jung_index, jong_index = decomposed
        cho_bul, jung_bul, jong_bul = self.get_bul_indices(cho_index, jung_index, jong_index)
        
        # 초성 글리프 인덱스
        cho_glyph_index = self.CHO_OFFSET + cho_bul * self.CHO_COUNT + cho_index
        
        # 중성 글리프 인덱스
        jung_glyph_index = self.JUNG_OFFSET + jung_bul * self.JUNG_COUNT + jung_index
        
        # 종성 글리프 인덱스
        jong_glyph_index = self.JONG_OFFSET + jong_bul * self.JONG_COUNT + jong_index
        
        # 각 글리프 비트맵 가져오기
        cho_bitmap = self.get_glyph_bitmap(cho_glyph_index)
        jung_bitmap = self.get_glyph_bitmap(jung_glyph_index)
        jong_bitmap = self.get_glyph_bitmap(jong_glyph_index) if jong_index > 0 else None
        
        # 비트맵 합성 (OR 연산)
        result = cho_bitmap | jung_bitmap
        if jong_bitmap is not None:
            result = result | jong_bitmap
        
        return result
    
    def render_text(self, text, spacing=2):
        """텍스트를 렌더링
        
        Args:
            text: 렌더링할 텍스트
            spacing: 문자 간 간격 (픽셀)
            
        Returns:
            PIL Image
        """
        # 한글만 필터링 (공백은 유지)
        chars = []
        for char in text:
            if '가' <= char <= '힣':
                chars.append(char)
            elif char == ' ':
                chars.append(' ')
        
        if not chars:
            return None
        
        # 전체 이미지 크기 계산
        total_width = len(chars) * self.GLYPH_WIDTH + (len(chars) - 1) * spacing
        total_height = self.GLYPH_HEIGHT
        
        # 이미지 생성 (흰색 배경)
        image = Image.new('L', (total_width, total_height), color=255)
        pixels = np.array(image)
        
        # 각 문자 렌더링
        x_offset = 0
        for char in chars:
            if char == ' ':
                # 공백은 건너뛰기
                x_offset += self.GLYPH_WIDTH + spacing
                continue
            
            char_bitmap = self.render_char(char)
            
            # 반전 (0=배경, 255=글자 -> 255=배경, 0=글자)
            char_bitmap = 255 - char_bitmap
            
            # 이미지에 복사
            pixels[:, x_offset:x_offset + self.GLYPH_WIDTH] = np.minimum(
                pixels[:, x_offset:x_offset + self.GLYPH_WIDTH],
                char_bitmap
            )
            
            x_offset += self.GLYPH_WIDTH + spacing
        
        return Image.fromarray(pixels)


def main():
    """메인 함수"""
    import sys
    
    if len(sys.argv) < 3:
        print("사용법: python render_hangul.py <font.han> <text> [output.png]")
        print("예제: python render_hangul.py H01_kr.han \"안녕하세요\" output.png")
        sys.exit(1)
    
    font_file = sys.argv[1]
    text = sys.argv[2]
    output_file = sys.argv[3] if len(sys.argv) > 3 else "output.png"
    
    print(f"=== 한글 텍스트 렌더링 ===")
    print(f"폰트: {font_file}")
    print(f"텍스트: {text}")
    
    # 렌더러 생성
    renderer = HangulRenderer(font_file)
    
    # 텍스트 렌더링
    image = renderer.render_text(text, spacing=2)
    
    if image:
        # 이미지 저장
        image.save(output_file)
        print(f"이미지 저장 완료: {output_file}")
        print(f"이미지 크기: {image.size}")
    else:
        print("렌더링할 문자가 없습니다.")


if __name__ == "__main__":
    main()
