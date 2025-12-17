/*************************************************************************/
/*  freetype_wrapper.h                                                   */
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

#ifndef freetype_wrapper_H
#define freetype_wrapper_H

#include "configs/modules_enabled.gen.h"
#ifdef MODULE_FREETYPE_ENABLED

#include "core/hash_map.h"
#include "core/pool_vector.h"
#include "scene/resources/freetype_font.h"

#include "font_cache_key.h"

#include <ft2build.h>
#include FT_CACHE_H
#include FT_FREETYPE_H

class FreeTypeFontData;

struct FontDataID {
	uint32_t font_hash = 0;
	uint32_t font_face_index = 0;
};

class FreeTypeWrapper {
	FT_Library ft_library;

	FTC_Manager ftc_manager;

	mutable HashMap<uint32_t, FontDataID *> font_id_map;
	FontDataID *_get_font_data_id(uint32_t p_font_hash, uint32_t p_font_face_index = 0) const;

public:
	static HashMap<uint32_t, Ref<FreeTypeFontData>> font_data_map;
	static void store_font_data(uint32_t p_font_hash, Ref<FreeTypeFontData> p_font_data);

	FT_Face lookup_face(uint32_t p_font_hash) const;
	FT_Size lookup_size(uint32_t p_font_hash, int p_size, float p_oversampling) const;

	FreeTypeWrapper();
	~FreeTypeWrapper();
};

#endif

#endif
