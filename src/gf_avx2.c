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

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "gf.h"
#include "gf2.h"
#include "gf4.h"
#include "gf16.h"
#include "gf256.h"

void
ffxor_region_avx2(uint8_t *region1, const uint8_t *region2, int length)
{
	register __m256i in, out;

	for (; length & 0xffffffe0; region1+=32, region2+=32, length-=32) {
		in  = _mm256_load_si256((void *)region2);
		out = _mm256_load_si256((void *)region1);
		out = _mm256_xor_si256(in, out);
		_mm256_store_si256((void *)region1, out);
	}
	
	ffxor_region_gpr(region1, region2, length);
}
