#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
자동 실행 스크립트 - 16x16과 32x32 이미지 생성
"""

import sys
import os

# 현재 디렉토리를 경로에 추가
sys.path.insert(0, os.path.dirname(__file__))

from render_all_fonts import render_all_fonts

def main():
    font_dir = os.path.join(os.path.dirname(__file__), '..', 'EasyView-font', 'ko')
    font_dir = os.path.abspath(font_dir)
    
    text = "안녕하세요 한글출력입니다"
    
    print("="*60)
    print("모든 한글 폰트 렌더링")
    print("="*60)
    print(f"텍스트: {text}")
    print(f"폰트 디렉토리: {font_dir}\n")
    
    # 16x16 렌더링
    print("\n" + "="*60)
    print("[1/2] 16x16 크기 렌더링")
    print("="*60 + "\n")
    
    output_16 = os.path.join(os.path.dirname(__file__), '..', 'all_fonts_16x16.png')
    output_16 = os.path.abspath(output_16)
    
    render_all_fonts(font_dir, text, output_16, scale=1, spacing=2)
    
    # 32x32 렌더링
    print("\n\n" + "="*60)
    print("[2/2] 32x32 크기 렌더링")
    print("="*60 + "\n")
    
    output_32 = os.path.join(os.path.dirname(__file__), '..', 'all_fonts_32x32.png')
    output_32 = os.path.abspath(output_32)
    
    render_all_fonts(font_dir, text, output_32, scale=2, spacing=2)
    
    print("\n\n" + "="*60)
    print("모든 작업 완료!")
    print("="*60)
    print(f"\n생성된 파일:")
    print(f"  - {output_16}")
    print(f"  - {output_32}")

if __name__ == "__main__":
    main()
