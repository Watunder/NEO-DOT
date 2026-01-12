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

#ifdef MODULE_FREETYPE_ENABLED
#include "font/builtin_fonts.gen.h"
#endif

#include "thirdparty/zstd/common/xxhash.h"

#ifdef MODULE_FREETYPE_ENABLED
void FontServer::_setup_builtin_fonts() {
	{
		PoolVector<uint8_t> NotoSansUI_Regular;
		NotoSansUI_Regular.resize(_font_DroidSansFallback_size);
		copymem(NotoSansUI_Regular.write().ptr(), _font_NotoSansUI_Regular, _font_NotoSansUI_Regular_size);

		default_font_id.font_hash = XXH32(NotoSansUI_Regular.read().ptr(), NotoSansUI_Regular.size(), 0);
		freetype_wrapper->update_font_data(default_font_id, NotoSansUI_Regular);
		builtin_font_ids.push_back(default_font_id);
	}
	{
		PoolVector<uint8_t> NotoEmoji_Regular;
		NotoEmoji_Regular.resize(_font_NotoEmoji_Regular_size);
		copymem(NotoEmoji_Regular.write().ptr(), _font_NotoEmoji_Regular, _font_NotoEmoji_Regular_size);

		FontID font_id;
		font_id.font_hash = XXH32(NotoEmoji_Regular.read().ptr(), NotoEmoji_Regular.size(), 0);
		freetype_wrapper->update_font_data(font_id, NotoEmoji_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> DroidSansFallback;
		DroidSansFallback.resize(_font_DroidSansFallback_size);
		copymem(DroidSansFallback.write().ptr(), _font_DroidSansFallback, _font_DroidSansFallback_size);

		FontID font_id;
		font_id.font_hash = XXH32(DroidSansFallback.read().ptr(), DroidSansFallback.size(), 0);
		freetype_wrapper->update_font_data(font_id, DroidSansFallback);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> DroidSansJapanese;
		DroidSansJapanese.resize(_font_DroidSansJapanese_size);
		copymem(DroidSansJapanese.write().ptr(), _font_DroidSansJapanese, _font_DroidSansJapanese_size);

		FontID font_id;
		font_id.font_hash = XXH32(DroidSansJapanese.read().ptr(), DroidSansJapanese.size(), 0);
		freetype_wrapper->update_font_data(font_id, DroidSansJapanese);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoNaskhArabicUI_Regular;
		NotoNaskhArabicUI_Regular.resize(_font_NotoNaskhArabicUI_Regular_size);
		copymem(NotoNaskhArabicUI_Regular.write().ptr(), _font_NotoNaskhArabicUI_Regular, _font_NotoNaskhArabicUI_Regular_size);

		FontID font_id;
		font_id.font_hash = XXH32(NotoNaskhArabicUI_Regular.read().ptr(), NotoNaskhArabicUI_Regular.size(), 0);
		freetype_wrapper->update_font_data(font_id, NotoNaskhArabicUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoSansHebrew_Regular;
		NotoSansHebrew_Regular.resize(_font_NotoSansHebrew_Regular_size);
		copymem(NotoSansHebrew_Regular.write().ptr(), _font_NotoSansHebrew_Regular, _font_NotoSansHebrew_Regular_size);

		FontID font_id;
		font_id.font_hash = XXH32(NotoSansHebrew_Regular.read().ptr(), NotoSansHebrew_Regular.size(), 0);
		freetype_wrapper->update_font_data(font_id, NotoSansHebrew_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoSansThaiUI_Regular;
		NotoSansThaiUI_Regular.resize(_font_NotoSansThaiUI_Regular_size);
		copymem(NotoSansThaiUI_Regular.write().ptr(), _font_NotoSansThaiUI_Regular, _font_NotoSansThaiUI_Regular_size);

		FontID font_id;
		font_id.font_hash = XXH32(NotoSansThaiUI_Regular.read().ptr(), NotoSansThaiUI_Regular.size(), 0);
		freetype_wrapper->update_font_data(font_id, NotoSansThaiUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoSansDevanagariUI_Regular;
		NotoSansDevanagariUI_Regular.resize(_font_NotoSansDevanagariUI_Regular_size);
		copymem(NotoSansDevanagariUI_Regular.write().ptr(), _font_NotoSansDevanagariUI_Regular, _font_NotoSansDevanagariUI_Regular_size);

		FontID font_id;
		font_id.font_hash = XXH32(NotoSansDevanagariUI_Regular.read().ptr(), NotoSansDevanagariUI_Regular.size(), 0);
		freetype_wrapper->update_font_data(font_id, NotoSansDevanagariUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
}
#endif

#ifdef MODULE_RAQM_ENABLED
Vector<RaqmWrapper::CharInfo> FontServer::_shape_string(Font *p_font, const String &p_text) const {
	Vector<RaqmWrapper::CharInfo> char_infos;

	ERR_FAIL_COND_V(!p_font, char_infos);

	FontID font_id = p_font->cache_key.get_font_id();

	FT_Size ft_size = freetype_wrapper->get_ft_size(font_id, p_font->cache_key.font_size, p_font->oversampling);
	ERR_FAIL_COND_V(!ft_size, char_infos);

	Vector<FT_Size> ft_sizes;
	ft_sizes.push_back(ft_size);

	uint64_t hash = p_font->cache_key.font_size;
	hash = hash * 31 + font_id.hash();

	if (!builtin_font_ids.empty()) {
		for (int i = 0; i < builtin_font_ids.size(); i++) {
			ERR_CONTINUE(!builtin_font_ids[i].font_hash);

			if (builtin_font_ids[i] == font_id) {
				continue;
			}

			FontID fallback_font_id = builtin_font_ids[i];
			FT_Size fallback_ft_size = freetype_wrapper->get_ft_size(fallback_font_id, p_font->cache_key.font_size, p_font->oversampling);
			ERR_CONTINUE(!fallback_ft_size);

			if (fallback_ft_size) {
				ft_sizes.push_back(fallback_ft_size);

				hash = hash * 31 + fallback_font_id.hash();
			}
		}
	}

	raqm_wrapper->update_cache(hash);
	char_infos = raqm_wrapper->get_char_infos(ft_sizes, p_text);

	return char_infos;
}

GlyphManager::GlyphInfo FontServer::_get_shaped_glyph_info(Font *p_font, const RaqmWrapper::ShapedGlyph &p_shaped_glyph) const {
	_THREAD_SAFE_METHOD_

	GlyphManager::GlyphInfo glyph_info{};
	ERR_FAIL_COND_V(!p_font, glyph_info);

	Ref<FreeTypeWrapper::FontInfo> font_info = freetype_wrapper->get_font_info(p_shaped_glyph.ft_face);
	ERR_FAIL_COND_V(!font_info.is_valid(), glyph_info);

	const FontCacheKey &temp_cache_key = p_font->cache_key.create_temp_key(font_info->id);
	if (p_font->temp_cache_keys.find(temp_cache_key) == -1) {
		p_font->temp_cache_keys.push_back(temp_cache_key);
	}

	glyph_manager->update_cache(temp_cache_key);
	glyph_info = glyph_manager->get_glyph_info(p_shaped_glyph.ft_face, p_shaped_glyph.index);

	glyph_info.texture_offset /= p_font->oversampling;
	glyph_info.texture_size /= p_font->oversampling;
	glyph_info.glyph_advance /= p_font->oversampling;

	return glyph_info;
}
#endif

GlyphManager::GlyphInfo FontServer::_get_simple_glyph_info(Font *p_font, char32_t p_char) const {
	_THREAD_SAFE_METHOD_

	GlyphManager::GlyphInfo glyph_info{};
	ERR_FAIL_COND_V(!p_font, glyph_info);

#ifdef MODULE_FREETYPE_ENABLED
	FontID font_id = p_font->cache_key.get_font_id();

	FT_Size ft_size = freetype_wrapper->get_ft_size(font_id, p_font->cache_key.font_size, p_font->oversampling);
	ERR_FAIL_COND_V(!ft_size, glyph_info);

	uint32_t glyph_index = FT_Get_Char_Index(ft_size->face, p_char);

	glyph_manager->update_cache(p_font->cache_key);
	glyph_info = glyph_manager->get_glyph_info(ft_size->face, glyph_index);

	if (!glyph_index && !builtin_font_ids.empty()) {
		for (int i = 0; i < builtin_font_ids.size(); i++) {
			ERR_CONTINUE(!builtin_font_ids[i].font_hash);

			if (builtin_font_ids[i] == font_id) {
				continue;
			}

			FontID fallback_font_id = builtin_font_ids[i];
			FT_Size fallback_ft_size = freetype_wrapper->get_ft_size(fallback_font_id, p_font->cache_key.font_size, p_font->oversampling);
			ERR_CONTINUE(!fallback_ft_size);

			uint32_t fallback_glyph_index = FT_Get_Char_Index(fallback_ft_size->face, p_char);

			if (fallback_glyph_index) {
				const FontCacheKey &temp_cache_key = p_font->cache_key.create_temp_key(fallback_font_id);
				if (p_font->temp_cache_keys.find(temp_cache_key) == -1) {
					p_font->temp_cache_keys.push_back(temp_cache_key);
				}

				glyph_manager->update_cache(temp_cache_key);
				glyph_info = glyph_manager->get_glyph_info(fallback_ft_size->face, fallback_glyph_index);
				break;
			}
		}
	}

	glyph_info.texture_offset /= p_font->oversampling;
	glyph_info.texture_size /= p_font->oversampling;
	glyph_info.glyph_advance /= p_font->oversampling;

	if (glyph_info.found) {
		glyph_info.glyph_advance.width += p_font->spacing_glyph;
	}
	if (p_char == U' ') {
		glyph_info.glyph_advance.width += p_font->spacing_space_char;
	}
#endif

	return glyph_info;
}

void FontServer::_draw_glyph(RID p_canvas_item, const GlyphManager::GlyphInfo &p_glyph_info, const Point2 &p_pos, const Color &p_modulate) const {
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
	}
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

RID FontServer::font_create(int p_size, int p_custom_flags, int p_texture_flags) {
	Font *font = memnew(Font);

	font->cache_key.font_size = p_size;
	font->cache_key.font_texture_flags = p_texture_flags;
	font->cache_key.font_custom_flags = p_custom_flags;
	font->cache_key.font_hash = default_font_id.font_hash;
	font->cache_key.font_index = default_font_id.font_index;

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

void FontServer::font_set_texture_flags(RID p_font, int p_texture_flags) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_texture_flags = p_texture_flags;
}

void FontServer::font_set_use_mipmaps(RID p_font, bool p_use_mipmaps) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (p_use_mipmaps) {
		font->cache_key.font_texture_flags |= Texture::FLAG_MIPMAPS;
	} else {
		font->cache_key.font_texture_flags &= ~Texture::FLAG_MIPMAPS;
	}
}

void FontServer::font_set_use_filter(RID p_font, bool p_use_filter) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (p_use_filter) {
		font->cache_key.font_texture_flags |= Texture::FLAG_FILTER;
	} else {
		font->cache_key.font_texture_flags &= ~Texture::FLAG_FILTER;
	}
}

void FontServer::font_set_custom_flags(RID p_font, int p_custom_flags) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	font->cache_key.font_custom_flags = p_custom_flags;
}

bool FontServer::font_update_data(RID p_font, const PoolVector<uint8_t> &p_font_data) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);

#ifdef MODULE_FREETYPE_ENABLED
	FontID temp_font_id;
	if (!p_font_data.empty()) {
		temp_font_id.font_hash = XXH32(p_font_data.read().ptr(), p_font_data.size(), 0);
		{
			temp_font_id.font_index = 0;

			freetype_wrapper->update_font_data(temp_font_id, p_font_data);

			FT_Face ft_face = freetype_wrapper->get_ft_face(temp_font_id);
			ERR_FAIL_COND_V(!ft_face, false);
		}
	} else {
		temp_font_id = default_font_id;
	}

	Ref<FreeTypeWrapper::FontInfo> font_info = freetype_wrapper->get_font_info(temp_font_id);
	ERR_FAIL_COND_V(!font_info.is_valid(), false);
	ERR_FAIL_INDEX_V(font->cache_key.font_index, font_info->face_count, false);

	temp_font_id.font_index = font->cache_key.font_index;

	freetype_wrapper->update_font_data(temp_font_id, p_font_data);

	FT_Face ft_face = freetype_wrapper->get_ft_face(temp_font_id);
	ERR_FAIL_COND_V(!ft_face, false);
#endif

	font->cache_key.font_hash = temp_font_id.font_hash;

	return true;
}

bool FontServer::font_update_index(RID p_font, int p_font_index) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);

#ifdef MODULE_FREETYPE_ENABLED
	Ref<FreeTypeWrapper::FontInfo> font_info = freetype_wrapper->get_font_info(font->cache_key.get_font_id());
	ERR_FAIL_COND_V(!font_info.is_valid(), false);
	ERR_FAIL_INDEX_V(p_font_index, font_info->face_count, false);

	FontID temp_font_id;
	if (font->cache_key.font_index != p_font_index) {
		temp_font_id.font_hash = font->cache_key.font_hash;
		temp_font_id.font_index = p_font_index;

		freetype_wrapper->update_font_data(temp_font_id, font_info->data);

		FT_Face ft_face = freetype_wrapper->get_ft_face(temp_font_id);
		ERR_FAIL_COND_V(!ft_face, false);
	}
#endif

	font->cache_key.font_index = temp_font_id.font_index;

	return true;
}

bool FontServer::font_update_metrics(RID p_font, float p_oversampling) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);

	if (p_oversampling > 0 && font->oversampling != p_oversampling) {
		font->oversampling = p_oversampling;
	}

#ifdef MODULE_FREETYPE_ENABLED
	FT_Size ft_size = freetype_wrapper->get_ft_size(font->cache_key.get_font_id(), font->cache_key.font_size, font->oversampling);
	ERR_FAIL_COND_V(!ft_size, false);

	font->ascent = (ft_size->metrics.ascender / 64.0) / font->oversampling;
	font->descent = (-ft_size->metrics.descender / 64.0) / font->oversampling;
#endif

	return true;
}

void FontServer::font_clear_caches(RID p_font) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	glyph_manager->clear_cache(font->cache_key);

	for (int i = 0; i < font->temp_cache_keys.size(); i++) {
		glyph_manager->clear_cache(font->temp_cache_keys[i]);
	}
	font->temp_cache_keys.clear();
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

FontCacheKey FontServer::font_get_cache_key(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, FontCacheKey{});

	return font->cache_key;
}

PoolVector<uint8_t> FontServer::font_get_data(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, PoolVector<uint8_t>());

#ifdef MODULE_FREETYPE_ENABLED
	Ref<FreeTypeWrapper::FontInfo> font_info = freetype_wrapper->get_font_info(font->cache_key.get_font_id());
	ERR_FAIL_COND_V(!font_info.is_valid(), PoolVector<uint8_t>());

	return font_info->data;
#endif
}

Size2 FontServer::font_get_char_size(RID p_font, char32_t p_char) const {
	ERR_FAIL_COND_V(!p_font.is_valid(), Size2(1, 1));

	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, Size2(1, 1));

	const GlyphManager::GlyphInfo &glyph_info = _get_simple_glyph_info(font, p_char);
	return glyph_info.glyph_advance;
}

Size2 FontServer::font_get_string_size(RID p_font, const String &p_text) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, Size2(0, 1));

	float height = font->ascent + font->descent;

	if (p_text.length() == 0) {
		return Size2(0, height);
	}

	float width = 0;

#ifdef MODULE_RAQM_ENABLED
	const Vector<RaqmWrapper::CharInfo> &char_infos = _shape_string(font, p_text);
	for (int i = 0; i < p_text.length(); i++) {
		const Vector<RaqmWrapper::ShapedGlyph> &glyphs = char_infos[i].glyphs;
		for (int j = 0; j < glyphs.size(); j++) {
			if (glyphs[j].cluster != i) {
				continue;
			}

			bool is_cluster_end = (j == 0 || glyphs[j].cluster != glyphs[j - 1].cluster);

			width += glyphs[j].advance.width / font->oversampling;

			if (glyphs[j].index == 32) {
				width += font->spacing_space_char;
			}
			if (is_cluster_end) {
				width += font->spacing_glyph;
			}
		}
	}
#else
	const char32_t *sptr = &p_text[0];
	for (int i = 0; i < p_text.length(); i++) {
		width += font_get_char_size(p_font, sptr[i]).width;
	}
#endif

	return Size2(width, height);
}

Ref<FontServer::TextData> FontServer::create_text_data(RID p_font, const String &p_text) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, Ref<TextData>());

	Ref<TextData> text_data;
	text_data.instance();

	text_data->font = p_font;
	text_data->original_text = p_text;

#ifdef MODULE_RAQM_ENABLED
	text_data->char_infos = _shape_string(font, text_data->original_text);
#endif

	return text_data;
}

float FontServer::draw_text_data(const Ref<TextData> &p_text_data, RID p_canvas_item, const Point2 &p_pos, int p_char_index, const Color &p_modulate, float p_clip_w) const {
	ERR_FAIL_COND_V(!p_text_data.is_valid(), 0);
	ERR_FAIL_COND_V(!p_text_data->font.is_valid(), 0);

	Vector2 ofs;

#ifdef MODULE_RAQM_ENABLED
	Font *font = font_owner.getornull(p_text_data->font);
	ERR_FAIL_COND_V(!font, 0);
	ERR_FAIL_INDEX_V(p_char_index, p_text_data->char_infos.size(), 0);

	const Vector<RaqmWrapper::ShapedGlyph> &glyphs = p_text_data->char_infos[p_char_index].glyphs;

	for (int j = 0; j < glyphs.size(); j++) {
		if (glyphs[j].cluster != p_char_index) {
			continue;
		}

		bool is_cluster_end = (j == 0 || glyphs[j].cluster != glyphs[j - 1].cluster);
		Vector2 advance = glyphs[j].advance / font->oversampling;

		if (is_cluster_end && p_clip_w > 0.0 && (ofs.x + advance.width) > p_clip_w) {
			break;
		}

		const GlyphManager::GlyphInfo &glyph_info = _get_shaped_glyph_info(font, glyphs[j]);
		_draw_glyph(p_canvas_item, glyph_info, p_pos + ofs + glyphs[j].offset / font->oversampling, p_modulate);

		ofs += advance;
		if (glyphs[j].index == 32) {
			ofs.width += font->spacing_space_char;
		}
		if (is_cluster_end) {
			ofs.width += font->spacing_glyph;
		}
	}
#else
	int width = font_get_char_size(p_text_data->font, p_text_data->original_text[p_char_index]).width;
	if (p_clip_w > 0.0 && (ofs.x + width) > p_clip_w) {
		return ofs.width;
	}

	ofs.x += draw_char(p_canvas_item, p_text_data->font, p_pos + ofs, p_text_data->original_text[p_char_index], p_modulate);
#endif

	return ofs.width;
}

float FontServer::draw_char(RID p_canvas_item, RID p_font, const Point2 &p_pos, char32_t p_char, const Color &p_modulate) const {
	ERR_FAIL_COND_V(!p_font.is_valid(), 0);

	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, 0);

	const GlyphManager::GlyphInfo &glyph_info = _get_simple_glyph_info(font, p_char);
	_draw_glyph(p_canvas_item, glyph_info, p_pos, p_modulate);
	return glyph_info.glyph_advance.width;
}

void FontServer::draw_string(RID p_canvas_item, RID p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate, float p_clip_w) const {
	Ref<TextData> text_data = create_text_data(p_font, p_text);

	Vector2 ofs;
	for (int i = 0; i < p_text.length(); i++) {
		ofs.x += draw_text_data(text_data, p_canvas_item, p_pos + ofs, i, p_modulate, p_clip_w);
	}
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

	_setup_builtin_fonts();
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
