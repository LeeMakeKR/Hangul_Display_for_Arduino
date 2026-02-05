#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
중성 섹션 구조 확인 스크립트
"""

# 한글 '가'를 분석
char = '가'
code = ord(char) - ord('가')  # 0

cho_unicode = code // (21 * 28)  # 0 (ㄱ)
jung_unicode = (code % (21 * 28)) // 28  # 0 (ㅏ)
jong_unicode = code % 28  # 0 (받침 없음)

print(f"문자: {char}")
print(f"유니코드 인덱스: cho={cho_unicode}, jung={jung_unicode}, jong={jong_unicode}")
print()

# 변환 테이블
JUNG_TABLE = [
    'ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅗ', 'ㅘ',
    'ㅙ', 'ㅚ', 'ㅛ', 'ㅜ', 'ㅝ', 'ㅞ', 'ㅟ', 'ㅠ', 'ㅡ', 'ㅢ', 'ㅣ'
]

print("중성 변환 테이블:")
for i, jung in enumerate(JUNG_TABLE):
    print(f"  유니코드 {i}: {jung}")
print()

# 한글 'ㅏ'는 유니코드 인덱스 0
# 폰트 파일에서는?
print("옵션 1: jung_index = jung_unicode + 1")
jung_index_1 = jung_unicode + 1
print(f"  폰트 인덱스: {jung_index_1}")
print(f"  => 폰트 파일 인덱스 0은 비어있음, 인덱스 1부터 ㅏ ㅐ ㅑ...")
print()

print("옵션 2: jung_index = jung_unicode + 2")
jung_index_2 = jung_unicode + 2
print(f"  폰트 인덱스: {jung_index_2}")
print(f"  => 폰트 파일 인덱스 0, 1은 비어있음, 인덱스 2부터 ㅏ ㅐ ㅑ...")
print()

# han_to_h.py의 JUNG_NAMES 확인
JUNG_NAMES = [
    "empty", "empty2", "a", "ae", "ya", "yae", "eo", "e", "yeo", "ye",
    "o", "wa", "wae", "oe", "yo", "u", "wo", "we", "wi", "yu",
    "eu", "ui", "i"
]

print("han_to_h.py의 JUNG_NAMES:")
for i, name in enumerate(JUNG_NAMES):
    print(f"  인덱스 {i}: {name}")
print()

print("=> JUNG_NAMES를 보면 인덱스 0, 1이 모두 empty로 비어있음")
print("=> 따라서 jung_index = jung_unicode + 2가 맞는 것으로 보임")
print()

# 그런데 render_hangul.py에서는 +1을 사용하고 렌더링이 성공함
print("⚠️  하지만 render_hangul.py에서는 +1을 사용하고 렌더링이 성공함")
print("⚠️  이것은 폰트 파일에서 실제로는 인덱스 0만 비어있다는 의미일 수 있음")
