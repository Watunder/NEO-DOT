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

	ShelfPackTexture *glyph_texture = texture_map[current_cache_key].ptrw();
	for (int i = 0; i < texture_map[current_cache_key].size(); i++) {
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

		texture_map[current_cache_key].push_back(tex);
		int texture_index = texture_map[current_cache_key].size() - 1;
		tex_pos = texture_map[current_cache_key].write[texture_index].pack_rect(texture_index, p_width, p_height);
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
	ShelfPackTexture &tex = texture_map[current_cache_key].write[tex_pos.index];
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
	glyph_info.texture_rect_uv = Rect2(tex_pos.x + rect_range, tex_pos.y + rect_range, w, h);
	glyph_info.texture_size = glyph_info.texture_rect_uv.size;
	if (current_cache_key.font_use_mipmaps) {
		glyph_info.texture_flags |= Texture::FLAG_MIPMAPS;
	}
	if (current_cache_key.font_use_filter) {
		glyph_info.texture_flags |= Texture::FLAG_FILTER;
	}

	return glyph_info;
}
#endif

void GlyphManager::update_glyph_cache(const FontCacheKey &p_cache_key) {
	if (current_cache_key == p_cache_key) {
		return;
	}
	current_cache_key = p_cache_key;

	if (!glyph_map.has(current_cache_key)) {
		glyph_map[current_cache_key] = HashMap<uint32_t, GlyphInfo>();
	}

	if (!texture_map.has(current_cache_key)) {
		texture_map[current_cache_key] = Vector<ShelfPackTexture>();
	}
}

void GlyphManager::clear_glyph_cache(const FontCacheKey &p_cache_key) {
	if (glyph_map.has(p_cache_key)) {
		glyph_map[p_cache_key].clear();
	}

	if (texture_map.has(p_cache_key)) {
		texture_map[p_cache_key].clear();
	}
}

GlyphManager::GlyphInfo GlyphManager::get_glyph_info(const Ref<FontHandle> &p_handle, uint32_t p_index) {
	GlyphInfo glyph_info{};

	ERR_FAIL_COND_V(p_handle.is_null(), glyph_info);
	ERR_FAIL_COND_V(p_index == 0, glyph_info);

	FontCacheKey cache_key = p_handle->get_cache_key();
	update_glyph_cache(cache_key);

	if (glyph_map[current_cache_key].has(p_index)) {
		return glyph_map[current_cache_key][p_index];
	}

#ifdef MODULE_FREETYPE_ENABLED
	Ref<FreeTypeFontHandle> font_handle = p_handle;
	if (font_handle.is_valid()) {
		int load_flags = FT_LOAD_TARGET_NORMAL;
		switch (cache_key.font_hinting) {
			case FreeTypeFont::HINTING_NONE:
				load_flags = FT_LOAD_NO_HINTING;
				break;
			case FreeTypeFont::HINTING_LIGHT:
				load_flags = FT_LOAD_TARGET_LIGHT;
				break;
			case FreeTypeFont::HINTING_NORMAL:
				load_flags = FT_LOAD_TARGET_NORMAL;
			default:
				load_flags = FT_LOAD_TARGET_NORMAL;
				break;
		}

		font_handle->update_cache(current_cache_key.font_size, font_handle->get_oversampling());
		FT_Size size = font_handle->get_ft_size();
		if (size && size->face) {
			uint32_t glyph_index = FT_Get_Char_Index(size->face, p_index);
			int error = FT_Load_Glyph(size->face, glyph_index, FT_HAS_COLOR(size->face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT | (cache_key.font_force_autohinter ? FT_LOAD_FORCE_AUTOHINT : 0) | load_flags);
			if (!error) {
				FT_GlyphSlot slot = size->face->glyph;

				error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
				if (!error) {
					glyph_info = _rasterize_bitmap(slot->bitmap);
					glyph_info.offset = Vector2(slot->bitmap_left, -slot->bitmap_top);
					glyph_info.advance = Vector2(slot->advance.x / 64.0, slot->advance.y / 64.0);
				}
			}
		}

	} else
#endif
	{
		// TODO: BitmapFont
	}

	if (glyph_info.found) {
		glyph_map[current_cache_key][p_index] = glyph_info;
	}

	return glyph_info;
}

Ref<ImageTexture> GlyphManager::get_texture(const GlyphInfo &p_glyph_info) {
	ERR_FAIL_COND_V(!texture_map.has(current_cache_key), Ref<ImageTexture>());
	ERR_FAIL_INDEX_V(p_glyph_info.texture_index, texture_map[current_cache_key].size(), Ref<ImageTexture>());
	if (texture_map[current_cache_key][p_glyph_info.texture_index].dirty) {
		ShelfPackTexture &tex = texture_map[current_cache_key].write[p_glyph_info.texture_index];
		tex.dirty = false;
		Ref<Image> img = memnew(Image(tex.texture_size, tex.texture_size, 0, tex.image_format, tex.image_data));
		if (tex.texture.is_null()) {
			tex.texture.instance();
			tex.texture->create_from_image(img, Texture::FLAG_VIDEO_SURFACE | p_glyph_info.texture_flags);
		} else {
			tex.texture->set_data(img); // update
		}
	}
	return texture_map[current_cache_key][p_glyph_info.texture_index].texture;
}
