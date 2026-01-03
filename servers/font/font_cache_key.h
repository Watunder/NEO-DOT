/*************************************************************************/
/*  font_cache_key.h                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-present Godot Engine contributors (cf. AUTHORS.md).*/
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef FONT_CACHE_KEY_H
#define FONT_CACHE_KEY_H

#include "core/hashfuncs.h"
#include "core/typedefs.h"

struct FontID {
	uint32_t font_hash = 0;
	uint8_t font_index = 0;

	bool operator==(const FontID &p_id) const {
		return (p_id.font_hash == font_hash &&
				p_id.font_index == font_index);
	}

	uint32_t hash() const {
		uint32_t h = font_hash;
		h = h * 31 + font_index;
		return h;
	}
};

struct FontIDHasher {
	static _FORCE_INLINE_ uint32_t hash(const FontID &p_id) { return p_id.hash(); }
};

struct FontCacheKey {
	union {
		struct {
			uint64_t font_hash : 32;
			uint64_t font_index : 8;
			uint64_t font_size : 16;
			uint64_t font_texture_flags : 3;
			uint64_t font_custom_flags : 3;
			uint64_t reserved : 2;
		};

		uint64_t key;
	};

	FontCacheKey() :
			key(0) {}

	bool operator==(const FontCacheKey &p_key) const { return key == p_key.key; }

	_FORCE_INLINE_ FontID get_font_id() const {
		FontID font_id;
		font_id.font_hash = font_hash;
		font_id.font_index = font_index;
		return font_id;
	}

	_FORCE_INLINE_ FontCacheKey create_temp_key(const FontID &p_font_id) const {
		FontCacheKey temp_cache_key;
		temp_cache_key.key = key;
		temp_cache_key.font_hash = p_font_id.font_hash;
		temp_cache_key.font_index = p_font_id.font_index;
		return temp_cache_key;
	}
};

#endif
