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

ShelfPackTexture::Position ShelfPackTexture::Shelf::alloc_shelf(int p_index, int p_w, int p_h) {
	if (p_w > w || p_h > h) {
		return ShelfPackTexture::Position{};
	}
	int xx = x;
	x += p_w;
	w -= p_w;
	return ShelfPackTexture::Position{ p_index, xx, y };
}

ShelfPackTexture::Position ShelfPackTexture::pack_rect(int p_index, int p_w, int p_h) {
	int y = 0;
	int waste = 0;
	List<Shelf>::Element *best_shelf = nullptr;
	int best_waste = INT_MAX;

	for (List<Shelf>::Element *E = shelves.front(); E; E = E->next()) {
		y += E->get().h;
		if (p_w > E->get().w) {
			continue;
		}
		if (p_h == E->get().h) {
			return E->get().alloc_shelf(p_index, p_w, p_h);
		}
		if (p_h > E->get().h) {
			continue;
		}
		if (p_h < E->get().h) {
			waste = (E->get().h - p_h) * p_w;
			if (waste < best_waste) {
				best_waste = waste;
				best_shelf = E;
			}
		}
	}
	if (best_shelf) {
		return best_shelf->get().alloc_shelf(p_index, p_w, p_h);
	}
	if (p_h <= (texture_size - y) && p_w <= texture_size) {
		List<Shelf>::Element *E = shelves.push_back(Shelf{ 0, y, texture_size, p_h });
		return E->get().alloc_shelf(p_index, p_w, p_h);
	}
	return ShelfPackTexture::Position{};
}

GlyphManager::TextureInfomation::TextureInfomation(const FT_Bitmap &p_bitmap, int p_rect_range) {
	rect_range = p_rect_range;

	w = p_bitmap.width;
	h = p_bitmap.rows;

	mw = w + rect_range * 2;
	mh = h + rect_range * 2;

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
}

ShelfPackTexture::Position GlyphManager::_find_texture_pos(TextureInfomation &p_tex_info) {
	ShelfPackTexture::Position tex_pos{};

	ShelfPackTexture *glyph_texture = texture_map[current_cache_id].ptrw();
	for (int i = 0; i < texture_map[current_cache_id].size(); i++) {
		if (glyph_texture[i].image_format != p_tex_info.image_format)
			continue;

		if (p_tex_info.mw > glyph_texture[i].texture_size || p_tex_info.mh > glyph_texture[i].texture_size) // too big for this texture
			continue;

		tex_pos = glyph_texture[i].pack_rect(i, p_tex_info.mw, p_tex_info.mh);
		if (tex_pos.index != -1) {
			break;
		}
	}

	// could not find texture to fit, create one
	if (tex_pos.index == -1) {
		int texture_size = ShelfPackTexture::MIN_TEXTURE_SIZE;

		if (p_tex_info.mw > texture_size)
			texture_size = p_tex_info.mw; // special case, adapt to it?
		if (p_tex_info.mh > texture_size)
			texture_size = p_tex_info.mh; // special case, adapt to it?

		texture_size = next_power_of_2(texture_size);

		texture_size = MIN(texture_size, ShelfPackTexture::MAX_TEXTURE_SIZE);

		ShelfPackTexture tex{};
		tex.image_format = p_tex_info.image_format;
		tex.texture_size = texture_size;
		tex.image_data.resize(texture_size * texture_size * p_tex_info.color_size); // grayscale alpha

		{
			// zero texture
			PoolVector<uint8_t>::Write w = tex.image_data.write();
			ERR_FAIL_COND_V(texture_size * texture_size * p_tex_info.color_size > tex.image_data.size(), tex_pos);

			// Initialize the texture to all-white pixels to prevent artifacts when the
			// font_data is displayed at a non-default scale with filtering enabled.
			if (p_tex_info.color_size == 2) {
				for (int i = 0; i < texture_size * texture_size * p_tex_info.color_size; i += 2) {
					w[i + 0] = 255;
					w[i + 1] = 0;
				}
			} else {
				for (int i = 0; i < texture_size * texture_size * p_tex_info.color_size; i += 4) {
					w[i + 0] = 255;
					w[i + 1] = 255;
					w[i + 2] = 255;
					w[i + 3] = 0;
				}
			}
		}

		texture_map[current_cache_id].push_back(tex);
		int texture_index = texture_map[current_cache_id].size() - 1;
		tex_pos = texture_map[current_cache_id].write[texture_index].pack_rect(texture_index, p_tex_info.mw, p_tex_info.mh);
	}

	return tex_pos;
}

GlyphManager::Glyph GlyphManager::_rasterize_bitmap(const FT_GlyphSlot &p_slot, int p_rect_range) {
	Glyph glyph{};

	FT_Bitmap bitmap = p_slot->bitmap;

	TextureInfomation tex_info{ bitmap, p_rect_range };
	ERR_FAIL_COND_V(tex_info.mw > ShelfPackTexture::MAX_TEXTURE_SIZE, glyph);
	ERR_FAIL_COND_V(tex_info.mh > ShelfPackTexture::MAX_TEXTURE_SIZE, glyph);

	ShelfPackTexture::Position tex_pos = _find_texture_pos(tex_info);
	ERR_FAIL_COND_V(tex_pos.index < 0, glyph);

	// fit glyph

	glyph.texture_index = tex_pos.index;
	ShelfPackTexture &tex = texture_map[current_cache_id].write[tex_pos.index];
	tex.dirty = true;

	{
		PoolVector<uint8_t>::Write wr = tex.image_data.write();

		for (int i = 0; i < tex_info.h; i++) {
			for (int j = 0; j < tex_info.w; j++) {
				int ofs = ((i + tex_pos.y + tex_info.rect_range) * tex.texture_size + j + tex_pos.x + tex_info.rect_range) * tex_info.color_size;
				ERR_FAIL_COND_V(ofs >= tex.image_data.size(), glyph);
				switch (bitmap.pixel_mode) {
					case FT_PIXEL_MODE_MONO: {
						int byte = i * bitmap.pitch + (j >> 3);
						int bit = 1 << (7 - (j % 8));
						wr[ofs + 0] = 255; // grayscale as 1
						wr[ofs + 1] = (bitmap.buffer[byte] & bit) ? 255 : 0;
					} break;
					case FT_PIXEL_MODE_GRAY:
						wr[ofs + 0] = 255; // grayscale as 1
						wr[ofs + 1] = bitmap.buffer[i * bitmap.pitch + j];
						break;
					case FT_PIXEL_MODE_BGRA: {
						int ofs_color = i * bitmap.pitch + (j << 2);
						wr[ofs + 2] = bitmap.buffer[ofs_color + 0];
						wr[ofs + 1] = bitmap.buffer[ofs_color + 1];
						wr[ofs + 0] = bitmap.buffer[ofs_color + 2];
						wr[ofs + 3] = bitmap.buffer[ofs_color + 3];
					} break;
					default:
						ERR_FAIL_V_MSG(glyph, "Font uses unsupported pixel format: " + itos(bitmap.pixel_mode) + ".");
						break;
				}
			}
		}
	}

	glyph.found = true;
	glyph.offset.x = p_slot->bitmap_left;
	glyph.offset.y = -p_slot->bitmap_top;
	glyph.textue_rect_uv = Rect2(tex_pos.x + tex_info.rect_range, tex_pos.y + tex_info.rect_range, tex_info.w, tex_info.h);
	glyph.textue_rect = glyph.textue_rect_uv;

	return glyph;
}

void GlyphManager::update_cache(const DynamicFontData::CacheID &p_cache_id) {
	if (current_cache_id == p_cache_id)
		return;

	current_cache_id = p_cache_id;

	if (!glyph_map.has(current_cache_id)) {
		glyph_map[current_cache_id] = HashMap<uint32_t, GlyphManager::Glyph>();
	}

	if (!texture_map.has(current_cache_id)) {
		texture_map[current_cache_id] = Vector<ShelfPackTexture>();
	}
}

void GlyphManager::clear_cache() {
	current_cache_id.key = 0;
	glyph_map.clear();
	texture_map.clear();
}

GlyphManager::Glyph GlyphManager::get_glyph(const FT_Face &p_face, const uint32_t &p_index) {
	if (glyph_map[current_cache_id].has(p_index)) {
		return glyph_map[current_cache_id][p_index];
	}

	Glyph glyph{};

	int error = FT_Load_Glyph(p_face, p_index, FT_HAS_COLOR(p_face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT);
	if (!error) {
		FT_GlyphSlot slot = p_face->glyph;
		error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);
		if (!error) {
			glyph = _rasterize_bitmap(slot);
		}
	}

	if (glyph.found) {
		glyph_map[current_cache_id][p_index] = glyph;
	}

	return glyph;
}

Ref<ImageTexture> GlyphManager::get_texture(const Glyph &p_glyph) {
	ERR_FAIL_COND_V(!texture_map.has(current_cache_id), Ref<ImageTexture>());
	ERR_FAIL_INDEX_V(p_glyph.texture_index, texture_map[current_cache_id].size(), Ref<ImageTexture>());
	if (texture_map[current_cache_id][p_glyph.texture_index].dirty) {
		ShelfPackTexture &tex = texture_map[current_cache_id].write[p_glyph.texture_index];
		tex.dirty = false;
		Ref<Image> img = memnew(Image(tex.texture_size, tex.texture_size, 0, tex.image_format, tex.image_data));
		if (tex.texture.is_null()) {
			tex.texture.instance();
			tex.texture->create_from_image(img, Texture::FLAG_VIDEO_SURFACE);
		} else {
			tex.texture->set_data(img); // update
		}
	}
	return texture_map[current_cache_id][p_glyph.texture_index].texture;
}
