#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
모든 한글 폰트를 하나의 이미지로 렌더링하는 도구
"""

import os
import sys
import glob
from PIL import Image, ImageDraw, ImageFont
import numpy as np

# render_hangul 모듈 import
sys.path.insert(0, os.path.dirname(__file__))
from render_hangul import HangulRenderer


def render_all_fonts(font_dir, text, output_file, scale=1, spacing=2):
    """모든 폰트로 텍스트를 렌더링하여 하나의 이미지로 생성
    
    Args:
        font_dir: 폰트 디렉토리 경로
        text: 렌더링할 텍스트
        output_file: 출력 파일 경로
        scale: 배율 (1=16px, 2=32px)
        spacing: 문자 간 간격
    """
    # .han 파일 목록 가져오기
    han_files = sorted(glob.glob(os.path.join(font_dir, '*.han')))
    
    if not han_files:
        print(f"오류: {font_dir}에서 .han 파일을 찾을 수 없습니다.")
        return
    
    print(f"총 {len(han_files)}개의 폰트 발견")
    
    # 각 폰트별로 렌더링
    font_images = []
    font_names = []
    failed_fonts = []
    
    for i, font_file in enumerate(han_files):
        font_name = os.path.splitext(os.path.basename(font_file))[0]
        
        # 10개마다 진행상황 출력
        if i % 10 == 0:
            print(f"\n진행: {i}/{len(han_files)}")
        
        try:
            print(f"  {font_name}", end='', flush=True)
            
            renderer = HangulRenderer(font_file)
            image = renderer.render_text(text, spacing=spacing)
            
            if image:
                font_images.append(image)
                font_names.append(font_name)
                print(" ✓")
            else:
                print(" ✗")
                failed_fonts.append(font_name)
                
        except Exception as e:
            print(f" ✗ ({str(e)[:30]})")
            failed_fonts.append(font_name)
            continue
    
    if not font_images:
        print("오류: 렌더링된 이미지가 없습니다.")
        return
    
    print(f"\n\n=== 결과 ===")
    print(f"성공: {len(font_images)}개")
    print(f"실패: {len(failed_fonts)}개")
    if failed_fonts:
        print(f"실패 목록: {', '.join(failed_fonts[:10])}{'...' if len(failed_fonts) > 10 else ''}")
    
    # 이미지 합성
    # 폰트 이름 표시를 위한 여백 추가
    name_width = 200  # 폰트 이름 영역
    glyph_height = 16
    max_width = max(img.width for img in font_images)
    total_width = name_width + max_width + 20  # 여백 포함
    line_height = glyph_height + 2  # 줄 간격
    total_height = len(font_images) * line_height + 40  # 상하 여백
    
    print(f"\n이미지 생성 중...")
    print(f"크기: {total_width} x {total_height}")
    
    # 최종 이미지 생성 (흰색 배경)
    final_image = Image.new('L', (total_width, total_height), color=255)
    draw = ImageDraw.Draw(final_image)
    
    # 각 폰트 이미지 배치
    y_offset = 20
    for i, (img, name) in enumerate(zip(font_images, font_names)):
        # 폰트 이름 텍스트 (간단하게 표시)
        try:
            draw.text((10, y_offset + 2), f"{i+1:3d}. {name[:25]}", fill=0)
        except:
            pass
        
        # 폰트 이미지 붙이기
        final_image.paste(img, (name_width, y_offset))
        
        y_offset += line_height
        
        # 100개마다 진행 출력
        if (i + 1) % 50 == 0:
            print(f"  배치 완료: {i+1}/{len(font_images)}")
    
    # 스케일 적용
    if scale > 1:
        print(f"스케일 적용 중... ({scale}배)")
        new_size = (final_image.width * scale, final_image.height * scale)
        final_image = final_image.resize(new_size, Image.NEAREST)
    
    # 이미지 저장
    print(f"저장 중...")
    final_image.save(output_file)
    print(f"\n✓ 완료: {output_file}")
    print(f"  크기: {final_image.width} x {final_image.height} 픽셀")
    print(f"  폰트: {len(font_images)}개")


def main():
    """메인 함수"""
    if len(sys.argv) < 2:
        print("사용법: python render_all_fonts.py <text> [scale]")
        print("예제: python render_all_fonts.py \"안녕하세요 한글출력입니다\" 1")
        print("      scale: 1=16px, 2=32px (기본값: 1)")
        sys.exit(1)
    
    text = sys.argv[1]
    scale = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    
    font_dir = os.path.join(os.path.dirname(__file__), '..', 'EasyView-font', 'ko')
    font_dir = os.path.abspath(font_dir)
    
    if not os.path.exists(font_dir):
        print(f"오류: 폰트 디렉토리를 찾을 수 없습니다: {font_dir}")
        sys.exit(1)
    
    size_name = f"{16*scale}x{16*scale}"
    output_file = f"all_fonts_{size_name}.png"
    
    print(f"=== 모든 폰트 렌더링 ===")
    print(f"텍스트: {text}")
    print(f"크기: {size_name}")
    print(f"폰트 디렉토리: {font_dir}")
    print(f"출력 파일: {output_file}\n")
    
    render_all_fonts(font_dir, text, output_file, scale=scale, spacing=2)


if __name__ == "__main__":
    main()
