#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
모든 .eng 파일을 .h 파일로 변환하는 스크립트
"""

import os
import sys
from eng_to_h import EnglishFontConverter

def main():
    # 경로 설정
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir = os.path.join(script_dir, '..', 'EasyView-font', 'en')
    output_dir = os.path.join(script_dir, '..', 'HangulDisp', 'fonts', 'en')
    
    # 입력 폴더 확인
    if not os.path.exists(input_dir):
        print(f"오류: 입력 폴더를 찾을 수 없습니다: {input_dir}")
        sys.exit(1)
    
    # 출력 폴더 생성
    os.makedirs(output_dir, exist_ok=True)
    
    # .eng 파일 목록 가져오기
    eng_files = [f for f in os.listdir(input_dir) if f.endswith('.eng')]
    
    if not eng_files:
        print(f"오류: .eng 파일을 찾을 수 없습니다: {input_dir}")
        sys.exit(1)
    
    print(f"=== 영문 폰트 일괄 변환기 ===")
    print(f"입력 폴더: {input_dir}")
    print(f"출력 폴더: {output_dir}")
    print(f"변환할 파일 수: {len(eng_files)}")
    print()
    
    # 변환 통계
    success_count = 0
    fail_count = 0
    failed_files = []
    
    # 각 파일 변환
    for i, eng_file in enumerate(sorted(eng_files), 1):
        input_path = os.path.join(input_dir, eng_file)
        output_file = eng_file.replace('.eng', '.h')
        output_path = os.path.join(output_dir, output_file)
        
        print(f"[{i}/{len(eng_files)}] {eng_file} -> {output_file}... ", end='', flush=True)
        
        try:
            converter = EnglishFontConverter(input_path)
            
            if not converter.read_font_file():
                print("실패 (파일 읽기 오류)")
                fail_count += 1
                failed_files.append(eng_file)
                continue
            
            if not converter.generate_header_file(output_path):
                print("실패 (헤더 생성 오류)")
                fail_count += 1
                failed_files.append(eng_file)
                continue
            
            print("완료")
            success_count += 1
            
        except Exception as e:
            print(f"실패 ({e})")
            fail_count += 1
            failed_files.append(eng_file)
    
    # 결과 출력
    print()
    print("=== 변환 완료 ===")
    print(f"성공: {success_count}개")
    print(f"실패: {fail_count}개")
    
    if failed_files:
        print("\n실패한 파일:")
        for f in failed_files:
            print(f"  - {f}")
    
    print(f"\n출력 폴더: {output_dir}")


if __name__ == "__main__":
    main()
