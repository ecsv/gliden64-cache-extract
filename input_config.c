/**
 * gliden64_cache_extract, GLideN64 TexCache Extraction tool for debugging
 *
 * Copyright (C) 2013-2015  Sven Eckelmann <sven@narfation.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gliden64_cache_extract.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int parse_config(uint32_t config)
{
	uint32_t remaining_bits;
	const uint32_t highres_bits = HIRESTEXTURES_MASK|TILE_HIRESTEX|FORCE16BPP_HIRESTEX|GZ_HIRESTEXCACHE|LET_TEXARTISTS_FLY;
	const uint32_t tex_bits = FILTER_MASK|ENHANCEMENT_MASK|FORCE16BPP_TEX|GZ_TEXCACHE;
	uint32_t testbits;

	switch (globals.type) {
	default:
	case INPUT_UNKNOWN:
		testbits = highres_bits | tex_bits;
		break;
	case INPUT_HIRES:
		testbits = highres_bits;
		break;
	case INPUT_TEX:
		testbits = tex_bits;
		break;
	};

	if (globals.verbose >= VERBOSITY_GLOBAL_HEADER) {
		const char *conf_str;

		fprintf(stderr, "Config Header:\n");

		if ((testbits & HIRESTEXTURES_MASK) == HIRESTEXTURES_MASK) {
			if ((config & HIRESTEXTURES_MASK) == NO_HIRESTEXTURES)
				conf_str = "0";
			else if ((config & HIRESTEXTURES_MASK) == RICE_HIRESTEXTURES)
				conf_str = "1";
			else
				conf_str = "set to an unsupported format";
			fprintf(stderr, "\ttxHiresEnable: %s\n", conf_str);
		}

		if ((testbits & TILE_HIRESTEX) == TILE_HIRESTEX)
			fprintf(stderr, "\tghq_hirs_tile: %s\n", (config & TILE_HIRESTEX) ? "True" : "False");

		if ((testbits & FORCE16BPP_HIRESTEX) == FORCE16BPP_HIRESTEX)
			fprintf(stderr, "\ttxForce16bpp: %s\n", (config & FORCE16BPP_HIRESTEX) ? "True" : "False");

		if ((testbits & GZ_HIRESTEXCACHE) == GZ_HIRESTEXCACHE)
			fprintf(stderr, "\ttxCacheCompression: %s\n", (config & GZ_HIRESTEXCACHE) ? "True" : "False");

		if ((testbits & LET_TEXARTISTS_FLY) == LET_TEXARTISTS_FLY)
			fprintf(stderr, "\ttxHiresFullAlphaChannel: %s\n", (config & LET_TEXARTISTS_FLY) ? "True" : "False");

		if ((testbits & FILTER_MASK) == FILTER_MASK) {
			if ((config & FILTER_MASK) == NO_FILTER)
				conf_str = "0";
			else if ((config & FILTER_MASK) == SMOOTH_FILTER_1)
				conf_str = "1";
			else if ((config & FILTER_MASK) == SMOOTH_FILTER_2)
				conf_str = "2";
			else if ((config & FILTER_MASK) == SMOOTH_FILTER_3)
				conf_str = "3";
			else if ((config & FILTER_MASK) == SMOOTH_FILTER_4)
				conf_str = "4";
			else if ((config & FILTER_MASK) == SHARP_FILTER_1)
				conf_str = "5";
			else if ((config & FILTER_MASK) == SHARP_FILTER_2)
				conf_str = "6";
			else
				conf_str = "set to an unsupported format";
			fprintf(stderr, "\ttxFilterMode: %s\n", conf_str);
		}

		if ((testbits & ENHANCEMENT_MASK) == ENHANCEMENT_MASK) {
			if ((config & ENHANCEMENT_MASK) == NO_ENHANCEMENT)
				conf_str = "0";
			else if ((config & ENHANCEMENT_MASK) == X2_ENHANCEMENT)
				conf_str = "2";
			else if ((config & ENHANCEMENT_MASK) == X2SAI_ENHANCEMENT)
				conf_str = "3";
			else if ((config & ENHANCEMENT_MASK) == HQ2X_ENHANCEMENT)
				conf_str = "4";
			else if ((config & ENHANCEMENT_MASK) == HQ2XS_ENHANCEMENT)
				conf_str = "5";
			else if ((config & ENHANCEMENT_MASK) == LQ2X_ENHANCEMENT)
				conf_str = "6";
			else if ((config & ENHANCEMENT_MASK) == LQ2XS_ENHANCEMENT)
				conf_str = "7";
			else if ((config & ENHANCEMENT_MASK) == HQ4X_ENHANCEMENT)
				conf_str = "8";
			else if ((config & ENHANCEMENT_MASK) == BRZ2X_ENHANCEMENT)
				conf_str = "9";
			else if ((config & ENHANCEMENT_MASK) == BRZ3X_ENHANCEMENT)
				conf_str = "10";
			else if ((config & ENHANCEMENT_MASK) == BRZ4X_ENHANCEMENT)
				conf_str = "11";
			else if ((config & ENHANCEMENT_MASK) == BRZ5X_ENHANCEMENT)
				conf_str = "12";
			else
				conf_str = "set to an unsupported format";
			fprintf(stderr, "\ttxEnhancementMode: %s\n", conf_str);
		}

		if ((testbits & FORCE16BPP_TEX) == FORCE16BPP_TEX)
			fprintf(stderr, "\ttxForce16bpp: %s\n", (config & FORCE16BPP_TEX) ? "True" : "False");

		if ((testbits & GZ_TEXCACHE) == GZ_TEXCACHE)
			fprintf(stderr, "\ttxCacheCompression: %s\n", (config & GZ_TEXCACHE) ? "True" : "False");

		fprintf(stderr, "\n");
	}

	remaining_bits = config & ~(testbits);
	if (remaining_bits)
		fprintf(stderr, "Warning: Unknown bits %#"PRIx32" set in config field\n", remaining_bits);

	return 0;
}
