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

GlyphManager::GlyphInfo FontServer::_get_simple_glyph_info(RID p_font_rid, char32_t p_char, Vector<RID> p_fallback_font_rids) const {
	GlyphManager::GlyphInfo glyph_info{};

	const FontCacheKey &cache_key = font_get_cache_key(p_font_rid);
	float font_oversampling = font_get_oversampling(p_font_rid);

	glyph_manager->update_glyph_cache(cache_key);

#ifdef MODULE_FREETYPE_ENABLED
	FT_Size default_ft_size = freetype_wrapper->lookup_size(cache_key.font_hash, cache_key.font_face_index, cache_key.font_size, font_oversampling);
	ERR_FAIL_COND_V(!default_ft_size, glyph_info);

	uint32_t glyph_index = FT_Get_Char_Index(default_ft_size->face, p_char);
	glyph_info = glyph_manager->get_glyph_info(default_ft_size->face, glyph_index);

	if (!glyph_index && !p_fallback_font_rids.empty()) {
		for (int i = 0; i < p_fallback_font_rids.size(); i++) {
			if (!p_fallback_font_rids[i].is_valid()) {
				continue;
			}

			const FontCacheKey &fallback_cache_key = font_get_cache_key(p_fallback_font_rids[i]);
			glyph_manager->update_glyph_cache(fallback_cache_key);

			FT_Size fallback_ft_size = freetype_wrapper->lookup_size(fallback_cache_key.font_hash, cache_key.font_face_index, cache_key.font_size, font_oversampling);
			uint32_t fallback_glyph_index = FT_Get_Char_Index(fallback_ft_size->face, p_char);
			if (fallback_ft_size && fallback_glyph_index) {
				glyph_info = glyph_manager->get_glyph_info(fallback_ft_size->face, fallback_glyph_index);
				break;
			}
		}
	}

	glyph_info.texture_offset /= font_oversampling;
	glyph_info.texture_size /= font_oversampling;
	glyph_info.advance /= font_oversampling;
#endif

	return glyph_info;
}

Vector2 FontServer::_draw_glyph(RID p_canvas_item, const GlyphManager::GlyphInfo &p_glyph_info, const Point2 &p_pos, const Color &p_modulate) const {
	if (p_glyph_info.found) {
		RID texture_rid = glyph_manager->get_texture_rid(p_glyph_info);
		if (texture_rid.is_valid()) {
			Color modulate = p_modulate;
			if (p_glyph_info.texture_format == Image::FORMAT_RGBA8) {
				modulate.r = modulate.g = modulate.b = 1.0;
			}

			Point2 texture_pos = p_pos + p_glyph_info.texture_offset;
			Rect2 texture_rect(texture_pos, p_glyph_info.texture_size);

			VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, texture_rect, texture_rid, p_glyph_info.texture_rect_uv, modulate, false, RID(), false);
		}

		return p_glyph_info.advance;
	}

	return Vector2();
}

#ifdef MODULE_RAQM_ENABLED
Vector2 FontServer::_draw_shaped_string(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w) const {
	_THREAD_SAFE_METHOD_

	Vector2 ofs;

	ERR_FAIL_COND_V(p_font.is_null(), ofs);

	RID font_rid = p_font->get_rid();
	if (!font_rid.is_valid()) {
		return ofs;
	}

	const FontCacheKey &cache_key = font_get_cache_key(font_rid);
	float font_oversampling = font_get_oversampling(font_rid);

	glyph_manager->update_glyph_cache(cache_key);
	raqm_wrapper->update_shaped_cache(cache_key);

	FT_Size ft_size = freetype_wrapper->lookup_size(cache_key.font_hash, cache_key.font_face_index, cache_key.font_size, font_oversampling);
	if (ft_size) {
		Vector<RaqmWrapper::ShapedInfo> shaped_infos = raqm_wrapper->shape_single_line(ft_size->face, p_text);
		for (int i = 0; i < shaped_infos.size(); i++) {
			GlyphManager::GlyphInfo glyph_info = glyph_manager->get_glyph_info(ft_size->face, shaped_infos[i].index);
			glyph_info.texture_offset /= font_oversampling;
			glyph_info.texture_size /= font_oversampling;
			glyph_info.advance /= font_oversampling;

			_draw_glyph(p_canvas_item, glyph_info, p_pos + ofs + shaped_infos[i].offset, p_modulate);
			ofs += shaped_infos[i].advance / font_oversampling;
		}
	}

	return ofs;
}
#endif

void FontServer::_bind_methods() {
}

FontServer *FontServer::singleton = NULL;

FontServer *FontServer::get_singleton() {
	return singleton;
}

RID FontServer::font_create() {
	FontInfo *font_info = memnew(FontInfo);
	return font_info_owner.make_rid(font_info);
}

void FontServer::font_free(RID p_font_rid) {
	if (font_info_owner.owns(p_font_rid)) {
		FontInfo *font_info = font_info_owner.get(p_font_rid);
		font_info_owner.free(p_font_rid);
		memdelete(font_info);
	}
}

void FontServer::font_set_size(RID p_font_rid, int p_size) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	font_info->cache_key.font_size = p_size;
}

void FontServer::font_set_use_mipmaps(RID p_font_rid, bool p_use_mipmaps) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	font_info->cache_key.font_use_mipmaps = p_use_mipmaps ? 1 : 0;
}

void FontServer::font_set_use_filter(RID p_font_rid, bool p_use_filter) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	font_info->cache_key.font_use_filter = p_use_filter ? 1 : 0;
}

void FontServer::font_set_force_autohinter(RID p_font_rid, bool p_force_autohinter) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	font_info->cache_key.font_force_autohinter = p_force_autohinter ? 1 : 0;
}

void FontServer::font_set_hinting(RID p_font_rid, int p_hinting) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	font_info->cache_key.font_hinting = p_hinting;
}

void FontServer::font_set_face_index(RID p_font_rid, int p_face_index) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	font_info->cache_key.font_face_index = p_face_index;
}

void FontServer::font_set_data(RID p_font_rid, const PoolVector<uint8_t> &p_font_buffer) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);
	ERR_FAIL_COND(p_font_buffer.empty());

#ifdef MODULE_FREETYPE_ENABLED
	font_info->cache_key.font_hash = freetype_wrapper->store_buffer(p_font_buffer);
	FT_Face ft_face = freetype_wrapper->lookup_face(font_info->cache_key.font_hash, font_info->cache_key.font_face_index);
	ERR_FAIL_COND(!ft_face);
#endif
}

void FontServer::font_clear_caches(RID p_font_rid) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	glyph_manager->clear_glyph_cache(font_info->cache_key);
#ifdef MODULE_RAQM_ENABLED
	raqm_wrapper->clear_shaped_cache(font_info->cache_key);
#endif
}

void FontServer::font_update_metrics(RID p_font_rid, float p_oversampling) {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND(!font_info);

	if (font_info->oversampling != p_oversampling) {
		font_info->oversampling = p_oversampling;
	}

#ifdef MODULE_FREETYPE_ENABLED
	FT_Size ft_size = freetype_wrapper->lookup_size(font_info->cache_key.font_hash, font_info->cache_key.font_face_index, font_info->cache_key.font_size, font_info->oversampling);
	ERR_FAIL_COND(!ft_size);

	font_info->ascent = (ft_size->metrics.ascender / 64.0) / font_info->oversampling;
	font_info->descent = (-ft_size->metrics.descender / 64.0) / font_info->oversampling;
#endif
}

float FontServer::font_get_ascent(RID p_font_rid) const {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND_V(!font_info, 0);

	return font_info->ascent;
}

float FontServer::font_get_descent(RID p_font_rid) const {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND_V(!font_info, 1);

	return font_info->descent;
}

float FontServer::font_get_oversampling(RID p_font_rid) const {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	ERR_FAIL_COND_V(!font_info, 1);

	return font_info->oversampling;
}

const FontCacheKey &FontServer::font_get_cache_key(RID p_font_rid) const {
	FontInfo *font_info = font_info_owner.getornull(p_font_rid);
	static const FontCacheKey empty_cache_key;
	ERR_FAIL_COND_V(!font_info, empty_cache_key);

	return font_info->cache_key;
}

Size2 FontServer::get_char_size(const Ref<Font> &p_font, char32_t p_char) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_font.is_null(), Size2(1, 1));

	RID font_rid = p_font->get_rid();
	if (!font_rid.is_valid()) {
		return Size2(1, 1);
	}

	Vector<RID> font_fallback_rids = p_font->get_fallback_rids();

	const GlyphManager::GlyphInfo glyph_info = _get_simple_glyph_info(font_rid, p_char, font_fallback_rids);
	return glyph_info.advance;
}

float FontServer::draw_char(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, char32_t p_char, const Color &p_modulate) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(p_font.is_null(), 0);

	RID font_rid = p_font->get_rid();
	if (!font_rid.is_valid()) {
		return 0;
	}

	Vector<RID> font_fallback_rids = p_font->get_fallback_rids();

	const GlyphManager::GlyphInfo glyph_info = _get_simple_glyph_info(font_rid, p_char, font_fallback_rids);
	return _draw_glyph(p_canvas_item, glyph_info, p_pos, p_modulate).width;
}

void FontServer::draw_string(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w) const {
#ifdef MODULE_RAQM_ENABLED
	_draw_shaped_string(p_canvas_item, p_font, p_pos, p_text, p_modulate, p_clip_w);
#else
	Vector2 ofs;

	for (int i = 0; i < p_text.length(); i++) {
		if (p_font.is_null())
			continue;

		int width = get_char_size(p_font, p_text[i]).width;

		if (p_clip_w >= 0 && (ofs.x + width) > p_clip_w)
			break; //clip

		ofs.x += draw_char(p_canvas_item, p_font, p_pos + ofs, p_text[i], p_modulate);
	}
#endif
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

FontServer::FontServer() {
	singleton = this;

#ifdef MODULE_FREETYPE_ENABLED
	freetype_wrapper = memnew(FreeTypeWrapper);
#endif
#ifdef MODULE_RAQM_ENABLED
	raqm_wrapper = memnew(RaqmWrapper);
#endif
	glyph_manager = memnew(GlyphManager);
}

FontServer::~FontServer() {
	singleton = NULL;

#ifdef MODULE_FREETYPE_ENABLED
	if (freetype_wrapper) {
		memdelete(freetype_wrapper);
	}
#endif
#ifdef MODULE_RAQM_ENABLED
	if (raqm_wrapper) {
		memdelete(raqm_wrapper);
	}
#endif
	if (glyph_manager) {
		memdelete(glyph_manager);
	}
}
