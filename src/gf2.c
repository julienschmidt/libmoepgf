/*
 * This file is part of moep80211gf.
 * 
 * Copyright (C) 2014 	Stephan M. Guenther <moepi@moepi.net>
 * Copyright (C) 2014 	Maximilian Riemensberger <riemensberger@tum.de>
 * Copyright (C) 2013 	Alexander Kurtz <alexander@kurtz.be>
 * 
 * moep80211gf is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * moep80211gf is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License * along
 * with moep80211gf.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>

#include "gf2.h"
#include "gf.h"

inline uint8_t
ffinv2(uint8_t element)
{
	return element;
}

inline uint8_t
ffadd2(uint8_t summand1, uint8_t summand2)
{
	return summand1 ^ summand2;
}

inline uint8_t
ffmul2(uint8_t factor1, uint8_t factor2)
{
	return factor1 & factor2;
}

inline void
ffadd2_region_gpr(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region_gpr(region1, region2, length);
}

inline void
ffmadd2_region_c_gpr(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		ffxor_region_gpr(region1, region2, length);
}

inline void
ffmul2_region_c(uint8_t *region, uint8_t constant, int length)
{
	if (constant == 0)
		memset(region, 0, length);
}

void
gf2_init()
{
	return;
}

#ifdef __x86_64__
inline void
ffadd2_region_sse2(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region_sse2(region1, region2, length);
}

inline void
ffadd2_region_avx2(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region_avx2(region1, region2, length);
}

inline void
ffmadd2_region_c_sse2(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		ffxor_region_sse2(region1, region2, length);
}

inline void
ffmadd2_region_c_avx2(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		ffxor_region_avx2(region1, region2, length);
}
#endif

#ifdef __arm__
inline void
ffadd2_region_neon(uint8_t *region1, const uint8_t *region2, int length)
{
	ffxor_region_neon(region1, region2, length);
}

inline void
ffmadd2_region_c_neon(uint8_t *region1, const uint8_t *region2,
				uint8_t constant, int length)
{
	if (constant != 0)
		ffxor_region_neon(region1, region2, length);
}
#endif

