/*************************************************************************/
/*  font_server.cpp                                                      */
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

#include "font_server.h"

#include "core/method_bind_ext.gen.inc"

void FontServer::_bind_methods() {
}

FontServer *FontServer::singleton = NULL;

FontServer *FontServer::get_singleton() {
	return singleton;
}

float FontServer::draw_char(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, char32_t p_char, const Color &p_modulate) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_font.is_null(), 0);

	Ref<FontHandle> font_handle = p_font->get_handle();

	GlyphManager::GlyphInfo glyph_info = glyph_manager->get_glyph_info(font_handle, p_char);
	if (glyph_info.found) {
		Ref<ImageTexture> texture = glyph_manager->get_texture(glyph_info);
		if (texture.is_valid()) {
			Color modulate = p_modulate;
			if (texture->get_format() == Image::FORMAT_RGBA8) {
				modulate.r = modulate.g = modulate.b = 1.0;
			}

			RID texture_rid = texture->get_rid();

			Point2 texture_pos = p_pos + glyph_info.offset / font_handle->get_oversampling();
			Rect2 texture_rect(texture_pos, glyph_info.texture_size / font_handle->get_oversampling());

			VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, texture_rect, texture_rid, glyph_info.texture_rect_uv, modulate, false, RID(), false);
		}

		return p_font->get_char_size(p_char).width;
	}

	return 0;
}

void FontServer::draw_string(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w) const {
	Vector2 ofs;

	for (int i = 0; i < p_text.length(); i++) {
		if (p_font.is_null())
			continue;

		int width = p_font->get_char_size(p_text[i]).width;

		if (p_clip_w >= 0 && (ofs.x + width) > p_clip_w)
			break; //clip

		ofs.x += draw_char(p_canvas_item, p_font, p_pos + ofs, p_text[i], p_modulate);
	}
}

void FontServer::draw_string_aligned(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate) const {
	float length = p_font->get_string_size(p_text).width;
	if (length >= p_width) {
		draw_string(p_canvas_item, p_font, p_pos, p_text, p_modulate, p_width);
		return;
	}

	float ofs = 0.f;
	switch (p_align) {
		case HALIGN_LEFT: {
			ofs = 0;
		} break;
		case HALIGN_CENTER: {
			ofs = Math::floor((p_width - length) / 2.0);
		} break;
		case HALIGN_RIGHT: {
			ofs = p_width - length;
		} break;
		default: {
			ERR_PRINT("Unknown halignment type");
		} break;
	}
	draw_string(p_canvas_item, p_font, p_pos + Point2(ofs, 0), p_text, p_modulate, p_width);
}

void FontServer::clear_glyph_cache(const FontHandle::CacheKey &p_cache_key) {
	glyph_manager->clear_glyph_cache(p_cache_key);
}

FontServer::FontServer() {
	singleton = this;

	glyph_manager = memnew(GlyphManager);
	text_manager = memnew(TextManger);
}

FontServer::~FontServer() {
	singleton = NULL;

	if (glyph_manager) {
		memdelete(glyph_manager);
	}
	if (text_manager) {
		memdelete(text_manager);
	}
}
