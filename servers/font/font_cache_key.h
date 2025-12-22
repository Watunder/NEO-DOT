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

#include "core/typedefs.h"

struct FontCacheKey {
	union {
		struct {
			uint64_t font_size : 10;
			uint64_t font_use_mipmaps : 1;
			uint64_t font_use_filter : 1;
			uint64_t font_force_autohinter : 1;
			uint64_t font_hinting : 2;
			uint64_t font_face_index : 1;
			uint64_t reserved : 16;
			uint64_t font_hash : 32;
		};
		uint64_t key;
	};

	FontCacheKey(const FontCacheKey &) = delete;
	FontCacheKey &operator=(const FontCacheKey &) = delete;

	FontCacheKey() { key = 0; }
	bool FontCacheKey::operator==(const FontCacheKey &p_key) const { return key == p_key.key; }
};

#endif
