/*************************************************************************/
/*  glyph_manager.cpp                                                    */
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

#include "glyph_manager.h"

#include "scene/resources/freetype_font.h"

_FORCE_INLINE_ ShelfPackTexture::Position GlyphManager::_find_texture_pos(int p_width, int p_height, int p_color_size, Image::Format p_image_format, int p_rect_range) {
	ShelfPackTexture::Position tex_pos{};

	ShelfPackTexture *glyph_texture = texture_map[current_cache_key.key].ptrw();
	for (int i = 0; i < texture_map[current_cache_key.key].size(); i++) {
		if (glyph_texture[i].image_format != p_image_format)
			continue;

		if (p_width > glyph_texture[i].texture_size || p_height > glyph_texture[i].texture_size) // too big for this texture
			continue;

		tex_pos = glyph_texture[i].pack_rect(i, p_width, p_height);
		if (tex_pos.index != -1) {
			break;
		}
	}

	// could not find texture to fit, create one
	if (tex_pos.index == -1) {
		int texture_size = MAX(current_cache_key.font_size * 8, ShelfPackTexture::MIN_TEXTURE_SIZE);

		if (p_width > texture_size)
			texture_size = p_width; // special case, adapt to it?
		if (p_height > texture_size)
			texture_size = p_height; // special case, adapt to it?

		texture_size = next_power_of_2(texture_size);

		texture_size = MIN(texture_size, ShelfPackTexture::MAX_TEXTURE_SIZE);

		ShelfPackTexture tex{};
		tex.image_format = p_image_format;
		tex.texture_size = texture_size;
		tex.image_data.resize(texture_size * texture_size * p_color_size); // grayscale alpha

		{
			// zero texture
			PoolVector<uint8_t>::Write w = tex.image_data.write();
			ERR_FAIL_COND_V(texture_size * texture_size * p_color_size > tex.image_data.size(), tex_pos);

			// Initialize the texture to all-white pixels to prevent artifacts when the
			// font_data is displayed at a non-default scale with filtering enabled.
			if (p_color_size == 2) {
				for (int i = 0; i < texture_size * texture_size * p_color_size; i += 2) {
					w[i + 0] = 255;
					w[i + 1] = 0;
				}
			} else {
				for (int i = 0; i < texture_size * texture_size * p_color_size; i += 4) {
					w[i + 0] = 255;
					w[i + 1] = 255;
					w[i + 2] = 255;
					w[i + 3] = 0;
				}
			}
		}

		texture_map[current_cache_key.key].push_back(tex);
		int texture_index = texture_map[current_cache_key.key].size() - 1;
		tex_pos = texture_map[current_cache_key.key].write[texture_index].pack_rect(texture_index, p_width, p_height);
	}

	return tex_pos;
}

#ifdef MODULE_FREETYPE_ENABLED
_FORCE_INLINE_ GlyphManager::GlyphInfo GlyphManager::_rasterize_bitmap(const FT_Bitmap &p_bitmap, int p_rect_range) {
	GlyphInfo glyph_info{};

	int rect_range = p_rect_range;

	int w = p_bitmap.width;
	int h = p_bitmap.rows;

	int mw = w + rect_range * 2;
	int mh = h + rect_range * 2;

	int color_size = 2;
	Image::Format image_format = Image::FORMAT_LA8;

	switch (p_bitmap.pixel_mode) {
		case FT_PIXEL_MODE_MONO:
		case FT_PIXEL_MODE_GRAY: {
			color_size = 2;
			image_format = Image::FORMAT_LA8;
		} break;
		case FT_PIXEL_MODE_BGRA: {
			color_size = 4;
			image_format = Image::FORMAT_RGBA8;
		} break;
	}

	ERR_FAIL_COND_V(mw > ShelfPackTexture::MAX_TEXTURE_SIZE, glyph_info);
	ERR_FAIL_COND_V(mh > ShelfPackTexture::MAX_TEXTURE_SIZE, glyph_info);

	ShelfPackTexture::Position tex_pos = _find_texture_pos(mw, mh, color_size, image_format, rect_range);
	ERR_FAIL_COND_V(tex_pos.index < 0, glyph_info);

	// fit glyph_info

	glyph_info.texture_index = tex_pos.index;
	ShelfPackTexture &tex = texture_map[current_cache_key.key].write[tex_pos.index];
	tex.dirty = true;

	{
		PoolVector<uint8_t>::Write wr = tex.image_data.write();

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int ofs = ((i + tex_pos.y + rect_range) * tex.texture_size + j + tex_pos.x + rect_range) * color_size;
				ERR_FAIL_COND_V(ofs >= tex.image_data.size(), glyph_info);
				switch (p_bitmap.pixel_mode) {
					case FT_PIXEL_MODE_MONO: {
						int byte = i * p_bitmap.pitch + (j >> 3);
						int bit = 1 << (7 - (j % 8));
						wr[ofs + 0] = 255; // grayscale as 1
						wr[ofs + 1] = (p_bitmap.buffer[byte] & bit) ? 255 : 0;
					} break;
					case FT_PIXEL_MODE_GRAY:
						wr[ofs + 0] = 255; // grayscale as 1
						wr[ofs + 1] = p_bitmap.buffer[i * p_bitmap.pitch + j];
						break;
					case FT_PIXEL_MODE_BGRA: {
						int ofs_color = i * p_bitmap.pitch + (j << 2);
						wr[ofs + 2] = p_bitmap.buffer[ofs_color + 0];
						wr[ofs + 1] = p_bitmap.buffer[ofs_color + 1];
						wr[ofs + 0] = p_bitmap.buffer[ofs_color + 2];
						wr[ofs + 3] = p_bitmap.buffer[ofs_color + 3];
					} break;
					default:
						ERR_FAIL_V_MSG(glyph_info, "Font uses unsupported pixel format: " + itos(p_bitmap.pixel_mode) + ".");
						break;
				}
			}
		}
	}

	glyph_info.found = true;
	glyph_info.texture_format = image_format;
	glyph_info.texture_rect_uv = Rect2(tex_pos.x + rect_range, tex_pos.y + rect_range, w, h);
	glyph_info.texture_size = glyph_info.texture_rect_uv.size;
	glyph_info.texture_flags |= current_cache_key.font_texture_flags;

	return glyph_info;
}
#endif

void GlyphManager::update_cache(const FontCacheKey &p_cache_key) {
	if (current_cache_key == p_cache_key) {
		return;
	}

	current_cache_key.key = p_cache_key.key;

	if (!glyph_info_map.has(current_cache_key.key)) {
		glyph_info_map[current_cache_key.key] = HashMap<uint32_t, GlyphInfo>();
	}

	if (!texture_map.has(current_cache_key.key)) {
		texture_map[current_cache_key.key] = Vector<ShelfPackTexture>();
	}
}

void GlyphManager::clear_cache(const FontCacheKey &p_cache_key) {
	if (glyph_info_map.has(p_cache_key.key)) {
		glyph_info_map.erase(p_cache_key.key);
	}

	if (texture_map.has(p_cache_key.key)) {
		texture_map.erase(p_cache_key.key);
	}
}

#ifdef MODULE_FREETYPE_ENABLED
GlyphManager::GlyphInfo GlyphManager::get_glyph_info(const FT_Face &p_ft_face, uint32_t p_glyph_index) {
	GlyphInfo glyph_info{};

	ERR_FAIL_COND_V(!p_ft_face, glyph_info);

	if (glyph_info_map[current_cache_key.key].has(p_glyph_index)) {
		return glyph_info_map[current_cache_key.key][p_glyph_index];
	}

	int load_flags = FT_HAS_COLOR(p_ft_face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT;
	switch (current_cache_key.font_custom_flags) {
		case FreeTypeFont::HINTING_NONE:
			load_flags |= FT_LOAD_NO_HINTING;
			break;
		case FreeTypeFont::HINTING_AUTO:
			load_flags |= FT_LOAD_FORCE_AUTOHINT;
			break;
		case FreeTypeFont::HINTING_LIGHT:
			load_flags |= FT_LOAD_TARGET_LIGHT;
			break;
		default:
			load_flags |= FT_LOAD_TARGET_NORMAL;
			break;
	}

	int error = FT_Load_Glyph(p_ft_face, p_glyph_index, load_flags);

	FT_GlyphSlot ft_glyph_slot = p_ft_face->glyph;
	if (!error) {
		error = FT_Render_Glyph(ft_glyph_slot, FT_RENDER_MODE_NORMAL);
	}
	if (!error) {
		glyph_info = _rasterize_bitmap(ft_glyph_slot->bitmap);
		glyph_info.texture_offset = Vector2(ft_glyph_slot->bitmap_left, -ft_glyph_slot->bitmap_top);
		glyph_info.glyph_advance = Vector2(ft_glyph_slot->advance.x / 64.0, ft_glyph_slot->advance.y / 64.0);
	}

	if (glyph_info.found) {
		glyph_info_map[current_cache_key.key][p_glyph_index] = glyph_info;
	}

	return glyph_info;
}
#endif

RID GlyphManager::get_texture_rid(const GlyphInfo &p_glyph_info) {
	ERR_FAIL_COND_V(!texture_map.has(current_cache_key.key), RID());
	ERR_FAIL_INDEX_V(p_glyph_info.texture_index, texture_map[current_cache_key.key].size(), RID());
	if (texture_map[current_cache_key.key][p_glyph_info.texture_index].dirty) {
		ShelfPackTexture &tex = texture_map[current_cache_key.key].write[p_glyph_info.texture_index];
		tex.dirty = false;
		Ref<Image> img = memnew(Image(tex.texture_size, tex.texture_size, 0, tex.image_format, tex.image_data));
		if (!tex.texture_rid.is_valid()) {
			tex.texture_rid = VisualServer::get_singleton()->texture_create_from_image(img, Texture::FLAG_VIDEO_SURFACE | p_glyph_info.texture_flags);
		} else {
			VisualServer::get_singleton()->texture_set_data(tex.texture_rid, img);
		}
	}
	return texture_map[current_cache_key.key][p_glyph_info.texture_index].texture_rid;
}
