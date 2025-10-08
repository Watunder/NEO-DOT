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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "scene/resources/dynamic_font.h"
#include "scene/resources/texture.h"

struct ShelfPackTexture {
	enum {
		MIN_TEXTURE_SIZE = 256,
		MAX_TEXTURE_SIZE = 4096,
	};

	struct Position {
		int index = -1;
		int x = 0;
		int y = 0;
	};

	struct Shelf {
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;

		Position alloc_shelf(int p_index, int p_w, int p_h);
	};

	List<Shelf> shelves;
	bool dirty = true;

	int texture_size = MIN_TEXTURE_SIZE;
	PoolVector<uint8_t> image_data;
	Ref<ImageTexture> texture;
	Image::Format image_format;

	Position pack_rect(int p_index, int p_w, int p_h);
};

class GlyphManager {
public:
	struct Glyph {
		bool found = false;

		Vector2 offset;

		int texture_index = -1;
		Rect2 textue_rect;
		Rect2 textue_rect_uv;
	};

private:
	DynamicFontData::CacheID current_cache_id;

	Map<DynamicFontData::CacheID, Vector<ShelfPackTexture>> texture_map;
	Map<DynamicFontData::CacheID, HashMap<uint32_t, Glyph>> glyph_map;

	struct TextureInfomation {
		int rect_range;

		int w;
		int h;

		int mw;
		int mh;

		int color_size;
		Image::Format image_format;

		TextureInfomation(const FT_Bitmap &p_bitmap, int p_rect_range);
	};

	ShelfPackTexture::Position _find_texture_pos(TextureInfomation &p_tex_info);

	Glyph _rasterize_bitmap(const FT_GlyphSlot &p_slot, int p_rect_range = 1);

public:
	void update_cache(const DynamicFontData::CacheID &p_cache_id);
	void clear_cache();

	Glyph get_glyph(const FT_Face &p_face, const uint32_t &p_index);

	Ref<ImageTexture> get_texture(const Glyph &p_glyph);
};
