#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
초성/중성/종성 섹션 구조 정확히 확인
"""

# 초성 (19개)
CHO_TABLE = ['ㄱ', 'ㄲ', 'ㄴ', 'ㄷ', 'ㄸ', 'ㄹ', 'ㅁ', 'ㅂ', 'ㅃ', 'ㅅ',
             'ㅆ', 'ㅇ', 'ㅈ', 'ㅉ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ']

# 중성 (21개)
JUNG_TABLE = ['ㅏ', 'ㅐ', 'ㅑ', 'ㅒ', 'ㅓ', 'ㅔ', 'ㅕ', 'ㅖ', 'ㅗ', 'ㅘ',
              'ㅙ', 'ㅚ', 'ㅛ', 'ㅜ', 'ㅝ', 'ㅞ', 'ㅟ', 'ㅠ', 'ㅡ', 'ㅢ', 'ㅣ']

# 종성 (27개 + 없음)
JONG_TABLE = ['', 'ㄱ', 'ㄲ', 'ㄳ', 'ㄴ', 'ㄵ', 'ㄶ', 'ㄷ', 'ㄹ', 'ㄺ',
              'ㄻ', 'ㄼ', 'ㄽ', 'ㄾ', 'ㄿ', 'ㅀ', 'ㅁ', 'ㅂ', 'ㅄ', 'ㅅ',
              'ㅆ', 'ㅇ', 'ㅈ', 'ㅊ', 'ㅋ', 'ㅌ', 'ㅍ', 'ㅎ']

print("=" * 60)
print("초성 섹션 구조")
print("=" * 60)
print(f"유니코드: 0~18 (19개)")
print(f"폰트 인덱스: cho_index = cho_unicode + 1")
print(f"  인덱스 0: 비어있음")
print(f"  인덱스 1~19: {', '.join(CHO_TABLE)}")
print(f"섹션 크기: 20자 × 8벌 = 160 글리프")
print()

print("=" * 60)
print("중성 섹션 구조")
print("=" * 60)
print(f"유니코드: 0~20 (21개)")
print(f"폰트 인덱스: jung_index = jung_unicode + 1")
print(f"  인덱스 0: 비어있음")
print(f"  인덱스 1~21: {', '.join(JUNG_TABLE)}")
print(f"섹션 크기: 22자 × 4벌 = 88 글리프")
print(f"섹션 오프셋: 160 (초성 섹션 이후)")
print()

print("=" * 60)
print("종성 섹션 구조")
print("=" * 60)
print(f"유니코드: 0~27 (28개, 0=받침없음)")
print(f"폰트 인덱스: jong_index = jong_unicode (변환 없음)")
print(f"  인덱스 0: 받침 없음 (비어있음)")
print(f"  인덱스 1~27: {', '.join(JONG_TABLE[1:])}")
print(f"섹션 크기: 28자 × 4벌 = 112 글리프")
print(f"섹션 오프셋: 248 (초성 + 중성 섹션 이후)")
print()

print("=" * 60)
print("han_to_h.py 수정 필요 사항")
print("=" * 60)
print("JUNG_NAMES가 잘못 정의되어 있음:")
print("  현재: ['empty', 'empty2', 'a', 'ae', ...]  (22개로 인덱스 0, 1이 empty)")
print("  수정: ['empty', 'a', 'ae', ...]  (실제로는 인덱스 0만 empty)")
print()
print("⚠️  하지만 JUNG_NAMES는 단순 참고용이므로 실제 변환에는 영향 없음")
print("⚠️  실제 중요한 것은 JUNG_COUNT = 22 (맞음)")
print()

print("=" * 60)
print("문서 수정 필요 사항")
print("=" * 60)
print("트러블슈팅 섹션 (라인 584-585):")
print("  문제: 중성이 한 글자씩 밀림")
print("  원인: 중성 섹션은 인덱스 0만 비어있음")
print("  해결: jung_index = jung_unicode + 1")
print()
print("=> 이 부분은 올바르게 작성되어 있음!")
print()
print("하지만 문서의 다른 부분에서 혼란을 줄 수 있는 표현:")
print("  - 라인 43: '인덱스 0: 비어있음' 만 있고, 인덱스 1이 비어있지 않다는 것을 명확히 해야 함")
print("  - 중성 섹션 설명을 더 명확히 해야 함")
