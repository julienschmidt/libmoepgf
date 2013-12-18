/*
 * This is a library providing arithmetic functions on GF(2^1) and GF(2^8).
 * Copyright (C) 2013  Alexander Kurtz <alexander@kurtz.be>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <immintrin.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gf256.h"
#include "gf.h"

#if GF256_POLYNOMIAL == 285
#include "gf256tables285.h"
#else
#error "Invalid prime polynomial or tables not available."
#endif

static const uint8_t inverses[GF256_SIZE] = GF256_INV_TABLE;
static const uint8_t pt[GF256_SIZE][GF256_EXPONENT] = GF256_POLYNOMIAL_DIV_TABLE;
static const uint8_t tl[GF256_SIZE][16] = GF256_SHUFFLE_LOW_TABLE;
static const uint8_t th[GF256_SIZE][16] = GF256_SHUFFLE_HIGH_TABLE;

inline void
ffadd256_region_avx2(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region_avx2(region1, region2, length);
}

void
ffmadd256_region_c_avx2(uint8_t *region1, const uint8_t *region2,
					uint8_t constant, int length)
{
	register __m256i t1, t2, m1, m2, in1, in2, out, l, h;
	register __m128i bc;

	if (constant == 0)
		return;

	if (constant == 1) {
		ffxor_region_avx2(region1, region2, length);
		return;
	}

	bc = _mm_load_si128((void *)tl[constant]);
	t1 = __builtin_ia32_vbroadcastsi256(bc);
	bc = _mm_load_si128((void *)th[constant]);
	t2 = __builtin_ia32_vbroadcastsi256(bc);
	m1 = _mm256_set1_epi8(0x0f);
	m2 = _mm256_set1_epi8(0xf0);

	for (; length & 0xffffffe0; region1+=32, region2+=32, length-=32) {
		in2 = _mm256_load_si256((void *)region2);
		in1 = _mm256_load_si256((void *)region1);
		l = _mm256_and_si256(in2, m1);
		l = _mm256_shuffle_epi8(t1, l);
		h = _mm256_and_si256(in2, m2);
		h = _mm256_srli_epi64(h, 4);
		h = _mm256_shuffle_epi8(t2, h);
		out = _mm256_xor_si256(h, l);
		out = _mm256_xor_si256(out, in1);
		_mm256_store_si256((void *)region1, out);
	}
	
	ffmadd256_region_c_gpr(region1, region2, constant, length);
}

void
ffmul256_region_c_avx2(uint8_t *region, uint8_t constant, int length)
{
	register __m256i t1, t2, m1, m2, in, out, l, h;
	register __m128i bc;

	if (constant == 0) {
		memset(region, 0, length);
		return;
	}

	if (constant == 1)
		return;

	bc = _mm_load_si128((void *)tl[constant]);
	t1 = __builtin_ia32_vbroadcastsi256(bc);
	bc = _mm_load_si128((void *)th[constant]);
	t2 = __builtin_ia32_vbroadcastsi256(bc);
	m1 = _mm256_set1_epi8(0x0f);
	m2 = _mm256_set1_epi8(0xf0);

	for (; length & 0xffffffe0; region+=32, length-=32) {
		in = _mm256_load_si256((void *)region);
		l = _mm256_and_si256(in, m1);
		l = _mm256_shuffle_epi8(t1, l);
		h = _mm256_and_si256(in, m2);
		h = _mm256_srli_epi64(h, 4);
		h = _mm256_shuffle_epi8(t2, h);
		out = _mm256_xor_si256(h, l);
		_mm256_store_si256((void *)region, out);
	}
	
	ffmul256_region_c_gpr(region, constant, length);
}
