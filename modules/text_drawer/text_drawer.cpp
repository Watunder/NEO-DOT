/*************************************************************************/
/*  text_drawer.cpp                                                      */
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

#include "text_drawer.h"

#include "core/method_bind_ext.gen.inc"

#include <raqm.h>

void TextDrawer::_draw_glyph(const DynamicFontData::CacheID &p_cache_id, const FT_Face &p_face, RID p_canvas_item, const Vector2 &p_pos, const uint32_t &p_index, const Vector2 &p_offset, const Color &p_modulate) const {
	glyph_manager->update_cache(p_cache_id);
	GlyphManager::Glyph glyph = glyph_manager->get_glyph(p_face, p_index);

	if (glyph.found) {
		glyph.textue_rect.position;
		glyph.textue_rect.size = glyph.textue_rect.size;

		Point2 cpos = p_pos + glyph.offset;
		cpos.x += p_offset.x / 64.0;
		cpos.y += p_offset.y / 64.0;

		Ref<ImageTexture> tex = glyph_manager->get_texture(glyph);
		if (tex.is_valid()) {
			Color modulate = p_modulate;
			if (tex->get_format() == Image::FORMAT_RGBA8) {
				modulate.r = modulate.g = modulate.b = 1.0;
			}
			RID texture = tex->get_rid();
			VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, Rect2(cpos, glyph.textue_rect.size), texture, glyph.textue_rect_uv, modulate, false, RID(), false);
		}
	}
}

void TextDrawer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("draw", "font", "canvas_item", "position", "string", "modulate", "clip_w"), &TextDrawer::draw, DEFVAL(Color(1, 1, 1)), DEFVAL(-1));
}

TextDrawer *TextDrawer::singleton = NULL;

TextDrawer *TextDrawer::get_singleton() {
	return singleton;
}

static _FORCE_INLINE_ void do_font_fallback(const FT_Face &p_default_face, const Vector<Ref<DynamicFontAtSize>> &p_fallbacks, HashMap<uint32_t, DynamicFontData::CacheID> &p_map, raqm_t *p_raqm_context, const uint32_t *p_text, size_t p_text_len) {
	for (int i = 0; i < p_text_len; i++) {
		if (FT_Get_Char_Index(p_default_face, p_text[i])) {
			continue;
		}
		for (int j = 0; j < p_fallbacks.size(); j++) {
			const FT_Face &fallback_face = p_fallbacks[j]->get_face();
			uint32_t glyph_index = FT_Get_Char_Index(fallback_face, p_text[i]);
			if (glyph_index) {
				p_map[glyph_index] = p_fallbacks[j]->get_cache_id();
				raqm_set_freetype_face_range(p_raqm_context, fallback_face, i, 1);
				break;
			}
		}
	}
}

Vector2 TextDrawer::draw(const Ref<DynamicFont> &p_font, RID p_canvas_item, const Vector2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w) const {
	Vector2 ofs;

	raqm_t *raqm_context = raqm_create();

	const DynamicFontData::CacheID &default_cache_id = p_font->get_data_at_size()->get_cache_id();
	const FT_Face &default_face = p_font->get_data_at_size()->get_face();
	const Vector<Ref<DynamicFontAtSize>> &fallback_data_at_size = p_font->get_fallback_data_at_size();

	const uint32_t *text = (const uint32_t *)p_text.c_str();
	int text_len = p_text.length();

	if (raqm_context && raqm_set_text(raqm_context, text, text_len) && raqm_set_freetype_face(raqm_context, default_face)) {
		HashMap<uint32_t, DynamicFontData::CacheID> fallback_cache_id_map;
		if (fallback_data_at_size.size() > 0) {
			do_font_fallback(default_face, fallback_data_at_size, fallback_cache_id_map, raqm_context, text, text_len);
		}

		size_t glyph_count;
		raqm_glyph_t *glyphs = NULL;
		if (raqm_layout(raqm_context)) {
			glyphs = raqm_get_glyphs(raqm_context, &glyph_count);
		}

		uint32_t glyph_cluster = 0;
		if (glyphs && glyph_count > 0) {
			for (int i = 0; i < glyph_count; i++) {
				uint32_t glyph_index = glyphs[i].index;
				Vector2 glyph_offset = Vector2(glyphs[i].x_offset, glyphs[i].y_offset);
				Vector2 glyph_advance = Vector2(glyphs[i].x_advance, glyphs[i].y_advance);

				if (p_clip_w >= 0 && ofs.x > p_clip_w && glyphs[i].cluster > glyph_cluster) {
					break; // clip
				}
				glyph_cluster = glyphs[i].cluster;

				DynamicFontData::CacheID glyph_cache_id = fallback_cache_id_map.has(glyph_index) ? fallback_cache_id_map[glyph_index] : default_cache_id;

				_draw_glyph(glyph_cache_id, glyphs[i].ftface, p_canvas_item, p_pos + ofs, glyph_index, glyph_offset, p_modulate);
				ofs += Vector2(glyph_advance.x / 64.0, glyph_advance.y / 64.0);
			}
		}
	}

	if (raqm_context) {
		raqm_destroy(raqm_context);
	}

	return ofs;
}

void TextDrawer::clear_cache() {
	glyph_manager->clear_cache();
}

TextDrawer::TextDrawer() {
	singleton = this;

	glyph_manager = memnew(GlyphManager);
}

TextDrawer::~TextDrawer() {
	singleton = NULL;

	if (glyph_manager) {
		memdelete(glyph_manager);
	}
}
