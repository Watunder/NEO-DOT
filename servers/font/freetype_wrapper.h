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

#ifndef FREETYPE_WRAPPER_H
#define FREETYPE_WRAPPER_H

#include "configs/modules_enabled.gen.h"
#ifdef MODULE_FREETYPE_ENABLED

#include "core/hash_map.h"
#include "core/map.h"
#include "core/pool_vector.h"

#include "font_cache_key.h"

#include <ft2build.h>
#include FT_CACHE_H
#include FT_FREETYPE_H

class FreeTypeWrapper {
public:
	struct FontInfo {
		FontID id;
		uint8_t face_count = 0;
	};

private:
	FT_Library ft_library = NULL;

	HashMap<uint32_t, PoolVector<uint8_t>> font_buffer_map;
	HashMap<FontID, FT_Face, FontIDHasher> ft_face_map;
	Map<FT_Face, FontInfo> font_infos;

public:
	uint32_t store_font_buffer(const PoolVector<uint8_t> &p_font_buffer);

	FT_Face get_ft_face(const FontID &p_font_id);
	FT_Size get_ft_size(const FontID &p_font_id, int p_size, float p_oversampling);

	FontInfo get_font_info(const FT_Face &p_ft_face) const;

	FreeTypeWrapper();
	~FreeTypeWrapper();
};

#endif

#endif
