/*************************************************************************/
/*  glyph_manager.h                                                      */
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

#ifndef GLYPH_MANAGER_H
#define GLYPH_MANAGER_H

#include "configs/modules_enabled.gen.h"
#ifdef MODULE_FREETYPE_ENABLED
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include "font_cache_key.h"
#include "shelf_pack_texture.h"

class GlyphManager {
public:
	struct GlyphInfo {
		bool found = false;

		Vector2 offset;
		Vector2 advance;

		int texture_index = -1;
		Size2 texture_size;
		Rect2 texture_rect_uv;
		Image::Format texture_format;

		int texture_flags = 0;
	};

private:
	FontCacheKey current_cache_key;

	HashMap<uint64_t, Vector<ShelfPackTexture>> texture_map;
	HashMap<uint64_t, HashMap<uint32_t, GlyphInfo>> glyph_map;

	_FORCE_INLINE_ ShelfPackTexture::Position _find_texture_pos(int p_width, int p_height, int p_color_size, Image::Format p_image_format, int p_rect_range);
#ifdef MODULE_FREETYPE_ENABLED
	_FORCE_INLINE_ GlyphInfo _rasterize_bitmap(const FT_Bitmap &p_bitmap, int p_rect_range = 1);
#endif

public:
	void update_glyph_cache(const FontCacheKey &p_cache_key);
	void clear_glyph_cache(const FontCacheKey &p_cache_key);

#ifdef MODULE_FREETYPE_ENABLED
	GlyphInfo get_glyph_info(const FT_Face &p_ft_face, uint32_t p_index);
#endif

	RID get_texture_rid(const GlyphInfo &p_glyph_info);
};

#endif
