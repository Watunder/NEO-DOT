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
#include "font/builtin_fonts.gen.h"

#ifdef MODULE_FREETYPE_ENABLED
void FontServer::_setup_fallback_fonts() {
	{
		RID font = font_create();
		PoolVector<uint8_t> DroidSansFallback;
		DroidSansFallback.resize(_font_DroidSansFallback_size);
		copymem(DroidSansFallback.write().ptr(), _font_DroidSansFallback, _font_DroidSansFallback_size);
		font_set_data(font, DroidSansFallback);
		add_fallback_font(font);
	}
	{
		RID font = font_create();
		PoolVector<uint8_t> DroidSansJapanese;
		DroidSansJapanese.resize(_font_DroidSansJapanese_size);
		copymem(DroidSansJapanese.write().ptr(), _font_DroidSansJapanese, _font_DroidSansJapanese_size);
		font_set_data(font, DroidSansJapanese);
		add_fallback_font(font);
	}
	{
		RID font = font_create();
		PoolVector<uint8_t> NotoNaskhArabicUI_Regular;
		NotoNaskhArabicUI_Regular.resize(_font_NotoNaskhArabicUI_Regular_size);
		copymem(NotoNaskhArabicUI_Regular.write().ptr(), _font_NotoNaskhArabicUI_Regular, _font_NotoNaskhArabicUI_Regular_size);
		font_set_data(font, NotoNaskhArabicUI_Regular);
		add_fallback_font(font);
	}
	{
		RID font = font_create();
		PoolVector<uint8_t> NotoSansHebrew_Regular;
		NotoSansHebrew_Regular.resize(_font_NotoSansHebrew_Regular_size);
		copymem(NotoSansHebrew_Regular.write().ptr(), _font_NotoSansHebrew_Regular, _font_NotoSansHebrew_Regular_size);
		font_set_data(font, NotoSansHebrew_Regular);
		add_fallback_font(font);
	}
	{
		RID font = font_create();
		PoolVector<uint8_t> NotoSansThaiUI_Regular;
		NotoSansThaiUI_Regular.resize(_font_NotoSansThaiUI_Regular_size);
		copymem(NotoSansThaiUI_Regular.write().ptr(), _font_NotoSansThaiUI_Regular, _font_NotoSansThaiUI_Regular_size);
		font_set_data(font, NotoSansThaiUI_Regular);
		add_fallback_font(font);
	}
	{
		RID font = font_create();
		PoolVector<uint8_t> NotoSansDevanagariUI_Regular;
		NotoSansDevanagariUI_Regular.resize(_font_NotoSansDevanagariUI_Regular_size);
		copymem(NotoSansDevanagariUI_Regular.write().ptr(), _font_NotoSansDevanagariUI_Regular, _font_NotoSansDevanagariUI_Regular_size);
		font_set_data(font, NotoSansDevanagariUI_Regular);
		add_fallback_font(font);
	}
}
#endif

#ifdef MODULE_RAQM_ENABLED
Vector2 FontServer::_draw_shaped_string(RID p_canvas_item, RID p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w, Vector<RID> p_fallback_fonts) const {
	Vector2 ofs;

	const FontCacheKey &cache_key = font_get_cache_key(p_font);
	float font_oversampling = font_get_oversampling(p_font);

	int font_spacing_glyph = font_get_spacing(p_font, SpacingType::SPACING_GLYPH);
	int font_spacing_space_char = font_get_spacing(p_font, SpacingType::SPACING_SPACE_CHAR);

	glyph_manager->update_glyph_cache(cache_key);
	raqm_wrapper->update_shaped_cache(cache_key);

	FT_Size ft_size = freetype_wrapper->lookup_size(cache_key.font_hash, cache_key.font_face_index, cache_key.font_size, font_oversampling);
	if (ft_size) {
		Vector<FT_Face> fallback_ft_faces;
		if (!p_fallback_fonts.empty()) {
			for (int i = 0; i < p_fallback_fonts.size(); i++) {
				if (!p_fallback_fonts[i].is_valid()) {
					continue;
				}

				const FontCacheKey &fallback_cache_key = font_get_cache_key(p_fallback_fonts[i]);

				FT_Size fallback_ft_size = freetype_wrapper->lookup_size(fallback_cache_key.font_hash, fallback_cache_key.font_face_index, cache_key.font_size, font_oversampling);
				if (fallback_ft_size) {
					fallback_ft_faces.push_back(fallback_ft_size->face);
				}
			}
		}

		Vector<RaqmWrapper::ShapedInfo> shaped_infos = raqm_wrapper->shape_single_line(ft_size->face, p_text, fallback_ft_faces);
		for (int i = 0; i < shaped_infos.size(); i++) {
			if (p_clip_w >= 0 && ofs.x > p_clip_w) {
				if (i == 0 || shaped_infos[i].cluster != shaped_infos[i - 1].cluster) {
					break;
				}
			}

			GlyphManager::GlyphInfo glyph_info = glyph_manager->get_glyph_info(shaped_infos[i].ft_face, shaped_infos[i].index);

			glyph_info.texture_offset /= font_oversampling;
			glyph_info.texture_size /= font_oversampling;
			glyph_info.advance /= font_oversampling;

			if (glyph_info.found) {
				glyph_info.advance.width += font_spacing_glyph;
			}
			if (shaped_infos[i].index == 32) {
				glyph_info.advance.width += font_spacing_space_char;
			}

			_draw_glyph(p_canvas_item, glyph_info, p_pos + ofs + shaped_infos[i].offset, p_modulate);
			ofs += shaped_infos[i].advance / font_oversampling;
		}
	}

	return ofs;
}
#endif

GlyphManager::GlyphInfo FontServer::_get_simple_glyph_info(RID p_font, char32_t p_char, Vector<RID> p_fallback_fonts) const {
	GlyphManager::GlyphInfo glyph_info{};

	const FontCacheKey &cache_key = font_get_cache_key(p_font);
	float font_oversampling = font_get_oversampling(p_font);

	int font_spacing_glyph = font_get_spacing(p_font, SpacingType::SPACING_GLYPH);
	int font_spacing_space_char = font_get_spacing(p_font, SpacingType::SPACING_SPACE_CHAR);

	glyph_manager->update_glyph_cache(cache_key);

#ifdef MODULE_FREETYPE_ENABLED
	FT_Size ft_size = freetype_wrapper->lookup_size(cache_key.font_hash, cache_key.font_face_index, cache_key.font_size, font_oversampling);
	ERR_FAIL_COND_V(!ft_size, glyph_info);

	uint32_t glyph_index = FT_Get_Char_Index(ft_size->face, p_char);
	glyph_info = glyph_manager->get_glyph_info(ft_size->face, glyph_index);

	if (!glyph_index && !p_fallback_fonts.empty()) {
		for (int i = 0; i < p_fallback_fonts.size(); i++) {
			if (!p_fallback_fonts[i].is_valid()) {
				continue;
			}

			const FontCacheKey &fallback_cache_key = font_get_cache_key(p_fallback_fonts[i]);

			FT_Size fallback_ft_size = freetype_wrapper->lookup_size(fallback_cache_key.font_hash, fallback_cache_key.font_face_index, cache_key.font_size, font_oversampling);
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

	if (glyph_info.found) {
		glyph_info.advance.width += font_spacing_glyph;
	}
	if (p_char == ' ') {
		glyph_info.advance.width += font_spacing_space_char;
	}
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

void FontServer::_bind_methods() {
	BIND_ENUM_CONSTANT(SPACING_TOP);
	BIND_ENUM_CONSTANT(SPACING_BOTTOM);
	BIND_ENUM_CONSTANT(SPACING_GLYPH);
	BIND_ENUM_CONSTANT(SPACING_SPACE_CHAR);
}

FontServer *FontServer::singleton = NULL;

FontServer *FontServer::get_singleton() {
	return singleton;
}

RID FontServer::font_create() {
	Font *font = memnew(Font);
	return font_owner.make_rid(font);
}

void FontServer::font_free(RID p_font) {
	if (font_owner.owns(p_font)) {
		Font *font = font_owner.get(p_font);
		font_owner.free(p_font);
		memdelete(font);
	}
}

void FontServer::font_set_size(RID p_font, int p_size) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_size = p_size;
}

void FontServer::font_set_use_mipmaps(RID p_font, bool p_use_mipmaps) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_use_mipmaps = p_use_mipmaps ? 1 : 0;
}

void FontServer::font_set_use_filter(RID p_font, bool p_use_filter) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_use_filter = p_use_filter ? 1 : 0;
}

void FontServer::font_set_force_autohinter(RID p_font, bool p_force_autohinter) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_force_autohinter = p_force_autohinter ? 1 : 0;
}

void FontServer::font_set_hinting(RID p_font, int p_hinting) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_hinting = p_hinting;
}

void FontServer::font_set_face_index(RID p_font, int p_face_index) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_face_index = p_face_index;
}

void FontServer::font_set_data(RID p_font, const PoolVector<uint8_t> &p_font_buffer) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);
	ERR_FAIL_COND(p_font_buffer.empty());

#ifdef MODULE_FREETYPE_ENABLED
	font->cache_key.font_hash = freetype_wrapper->store_buffer(p_font_buffer);
	FT_Face ft_face = freetype_wrapper->lookup_face(font->cache_key.font_hash, font->cache_key.font_face_index);
	ERR_FAIL_COND(!ft_face);
#endif
}

void FontServer::font_clear_caches(RID p_font) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	glyph_manager->clear_glyph_cache(font->cache_key);
#ifdef MODULE_RAQM_ENABLED
	raqm_wrapper->clear_shaped_cache(font->cache_key);
#endif
}

void FontServer::font_update_metrics(RID p_font, float p_oversampling) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (font->oversampling != p_oversampling) {
		font->oversampling = p_oversampling;
	}

#ifdef MODULE_FREETYPE_ENABLED
	FT_Size ft_size = freetype_wrapper->lookup_size(font->cache_key.font_hash, font->cache_key.font_face_index, font->cache_key.font_size, font->oversampling);
	ERR_FAIL_COND(!ft_size);

	font->ascent = (ft_size->metrics.ascender / 64.0) / font->oversampling;
	font->descent = (-ft_size->metrics.descender / 64.0) / font->oversampling;
#endif
}

void FontServer::font_set_spacing(RID p_font, SpacingType p_spcing_type, int p_spacing) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (p_spcing_type == SPACING_TOP) {
		font->spacing_top = p_spacing;
	} else if (p_spcing_type == SPACING_BOTTOM) {
		font->spacing_bottom = p_spacing;
	} else if (p_spcing_type == SPACING_GLYPH) {
		font->spacing_glyph = p_spacing;
	} else if (p_spcing_type == SPACING_SPACE_CHAR) {
		font->spacing_space_char = p_spacing;
	}
}

int FontServer::font_get_spacing(RID p_font, SpacingType p_spcing_type) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, 0);

	if (p_spcing_type == SPACING_TOP) {
		return font->spacing_top;
	} else if (p_spcing_type == SPACING_BOTTOM) {
		return font->spacing_bottom;
	} else if (p_spcing_type == SPACING_GLYPH) {
		return font->spacing_glyph;
	} else if (p_spcing_type == SPACING_SPACE_CHAR) {
		return font->spacing_space_char;
	}

	return 0;
}

float FontServer::font_get_ascent(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, 0);

	return font->ascent + font->spacing_top;
}

float FontServer::font_get_descent(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, 1);

	return font->descent + font->spacing_bottom;
}

float FontServer::font_get_oversampling(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, 1);

	return font->oversampling;
}

const FontCacheKey &FontServer::font_get_cache_key(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	static const FontCacheKey empty_cache_key;
	ERR_FAIL_COND_V(!font, empty_cache_key);

	return font->cache_key;
}

Size2 FontServer::font_get_char_size(RID p_font, char32_t p_char) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!p_font.is_valid(), Size2(1, 1));

	const GlyphManager::GlyphInfo glyph_info = _get_simple_glyph_info(p_font, p_char, fallback_fonts);
	return glyph_info.advance;
}

Size2 FontServer::font_get_string_size(RID p_font, const String &p_string) const {
	float height = FontServer::get_singleton()->font_get_ascent(p_font) + FontServer::get_singleton()->font_get_descent(p_font);

	if (p_string.length() == 0) {
		return Size2(0, height);
	}

	float width = 0;

	const char32_t *sptr = &p_string[0];
	for (int i = 0; i < p_string.length(); i++) {
		width += font_get_char_size(p_font, sptr[i]).width;
	}

	return Size2(width, height);
}

void FontServer::add_fallback_font(RID p_font) {
	ERR_FAIL_COND(!font_owner.owns(p_font));
	fallback_fonts.push_back(p_font);
}

void FontServer::remove_fallback_font(RID p_font) {
	ERR_FAIL_COND(!font_owner.owns(p_font));
	fallback_fonts.erase(p_font);
}

int FontServer::get_fallback_font_count() const {
	return fallback_fonts.size();
}

RID FontServer::get_fallback_font(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, fallback_fonts.size(), RID());
	return fallback_fonts[p_idx];
}

float FontServer::draw_char(RID p_canvas_item, RID p_font, const Point2 &p_pos, char32_t p_char, const Color &p_modulate) const {
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND_V(!p_font.is_valid(), 0);

	const GlyphManager::GlyphInfo glyph_info = _get_simple_glyph_info(p_font, p_char, fallback_fonts);
	return _draw_glyph(p_canvas_item, glyph_info, p_pos, p_modulate).width;
}

void FontServer::draw_string(RID p_canvas_item, RID p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate, int p_clip_w) const {
#ifdef MODULE_RAQM_ENABLED
	_THREAD_SAFE_METHOD_

	ERR_FAIL_COND(!p_font.is_valid());

	_draw_shaped_string(p_canvas_item, p_font, p_pos, p_text, p_modulate, p_clip_w, fallback_fonts);
#else
	Vector2 ofs;

	for (int i = 0; i < p_text.length(); i++) {
		int width = font_get_char_size(p_font, p_text[i]).width;

		if (p_clip_w >= 0 && (ofs.x + width) > p_clip_w)
			break;

		ofs.x += draw_char(p_canvas_item, p_font, p_pos + ofs, p_text[i], p_modulate);
	}
#endif
}

void FontServer::draw_string_aligned(RID p_canvas_item, RID p_font, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate) const {
	float length = font_get_string_size(p_font, p_text).width;
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

	_setup_fallback_fonts();
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
