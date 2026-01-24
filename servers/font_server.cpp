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
#include "servers/text/text_shaper.h"

FontDriver *FontDriver::singleton = NULL;
FontDriver *FontDriver::get_singleton() {
	return singleton;
}

void FontDriver::set_singleton() {
	singleton = this;
}

FontDriver *FontDriverManager::drivers[MAX_DRIVERS];
int FontDriverManager::driver_count = 0;

void FontDriverManager::add_driver(FontDriver *p_driver) {
	ERR_FAIL_COND(driver_count >= MAX_DRIVERS);
	drivers[driver_count++] = p_driver;
}

void FontDriverManager::initialize(int p_driver) {
	int failed_driver = -1;

	if (p_driver >= 0 && p_driver < driver_count) {
		if (drivers[p_driver]->init() == OK) {
			drivers[p_driver]->set_singleton();
			return;
		} else {
			failed_driver = p_driver;
		}
	}

	for (int i = 0; i < driver_count; i++) {
		if (i == failed_driver) {
			continue;
		}

		if (drivers[i]->init() == OK) {
			drivers[i]->set_singleton();
			break;
		}
	}
}

int FontDriverManager::get_driver_count() {
	return driver_count;
}

FontDriver *FontDriverManager::get_driver(int p_driver) {
	ERR_FAIL_INDEX_V(p_driver, driver_count, NULL);
	return drivers[p_driver];
}

/*************************************************************************/

_FORCE_INLINE_ void FontServer::_font_clear_caches(Font *p_font) {
	FontDriver::get_singleton()->clear_glyph_cache(p_font->cache_key);

	for (int i = 0; i < p_font->temp_cache_keys.size(); i++) {
		FontDriver::get_singleton()->clear_glyph_cache(p_font->temp_cache_keys[i]);
	}
	p_font->temp_cache_keys.clear();
}

_FORCE_INLINE_ bool FontServer::_font_update_metrics(Font *p_font) {
	float new_ascent = 0.0f;
	float new_descent = 0.0f;

	ERR_FAIL_COND_V(!FontDriver::get_singleton()->get_font_metrics(p_font->cache_key.get_font_id(), p_font->cache_key.font_size, p_font->cache_key.font_oversampling, new_ascent, new_descent), false);

	p_font->ascent = new_ascent;
	p_font->descent = new_descent;

	return true;
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

RID FontServer::font_create(int p_size, int p_custom_flags, bool p_use_mipmaps, bool p_use_filter) {
	Font *font = memnew(Font);

	Vector<FontID> builtin_font_ids = FontDriver::get_singleton()->get_builtin_font_ids();
	ERR_FAIL_COND_V(builtin_font_ids.empty(), RID());

	font->cache_key.font_size = p_size;
	font->cache_key.font_oversampling = 1;
	font->cache_key.font_use_mipmaps = p_use_mipmaps;
	font->cache_key.font_use_filter = p_use_filter;
	font->cache_key.font_custom_flags = p_custom_flags;
	font->cache_key.font_id = builtin_font_ids[0];

	_font_update_metrics(font);

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

	if (font->cache_key.font_size != p_size) {
		_font_clear_caches(font);
		font->cache_key.font_size = p_size;
		_font_update_metrics(font);
	}
}

void FontServer::font_set_use_mipmaps(RID p_font, bool p_use_mipmaps) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (font->cache_key.font_use_mipmaps != p_use_mipmaps) {
		_font_clear_caches(font);
		font->cache_key.font_use_mipmaps = p_use_mipmaps;
	}
}

void FontServer::font_set_use_filter(RID p_font, bool p_use_filter) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (font->cache_key.font_use_filter != p_use_filter) {
		_font_clear_caches(font);
		font->cache_key.font_use_filter = p_use_filter;
	}
}

void FontServer::font_set_custom_flags(RID p_font, int p_custom_flags) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND(!font);

	if (font->cache_key.font_custom_flags != p_custom_flags) {
		_font_clear_caches(font);
		font->cache_key.font_custom_flags = p_custom_flags;
	}
}

bool FontServer::font_set_data(RID p_font, const PoolVector<uint8_t> &p_font_data) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);
	ERR_FAIL_COND_V(p_font_data.empty(), false);

	_font_clear_caches(font);

	FontID temp_font_id = FontDriver::get_singleton()->add_font_data(p_font_data);
	temp_font_id.font_index = font->cache_key.font_id.font_index;

	ERR_FAIL_COND_V(!FontDriver::get_singleton()->validate_font(temp_font_id), false);

	font->cache_key.font_id.font_hash = temp_font_id.font_hash;

	return _font_update_metrics(font);
}

bool FontServer::font_set_builtin_data(RID p_font, int p_builtin_index) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);

	_font_clear_caches(font);

	Vector<FontID> builtin_font_ids = FontDriver::get_singleton()->get_builtin_font_ids();
	ERR_FAIL_COND_V(builtin_font_ids.empty(), false);
	ERR_FAIL_INDEX_V(p_builtin_index, builtin_font_ids.size(), false);

	FontID temp_font_id = builtin_font_ids[p_builtin_index];
	temp_font_id.font_index = font->cache_key.font_id.font_index;

	ERR_FAIL_COND_V(!FontDriver::get_singleton()->validate_font(temp_font_id), false);

	font->cache_key.font_id.font_hash = temp_font_id.font_hash;

	return _font_update_metrics(font);
}

bool FontServer::font_set_path(RID p_font, const String &p_font_path) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);
	ERR_FAIL_COND_V(p_font_path.empty(), false);

	_font_clear_caches(font);

	FontID temp_font_id = FontDriver::get_singleton()->add_font_path(p_font_path);
	temp_font_id.font_index = font->cache_key.font_id.font_index;

	ERR_FAIL_COND_V(!FontDriver::get_singleton()->validate_font(temp_font_id), false);

	font->cache_key.font_id.font_hash = temp_font_id.font_hash;

	return _font_update_metrics(font);
}

bool FontServer::font_set_index(RID p_font, int p_font_index) {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, false);

	Ref<FontInfo> font_info = FontDriver::get_singleton()->get_font_info(font->cache_key.get_font_id());
	ERR_FAIL_COND_V(!font_info.is_valid(), false);
	ERR_FAIL_INDEX_V(p_font_index, font_info->face_count, false);

	if (font->cache_key.font_id.font_index != p_font_index) {
		_font_clear_caches(font);

		FontID temp_font_id = font->cache_key.font_id;
		temp_font_id.font_index = p_font_index;

		ERR_FAIL_COND_V(!FontDriver::get_singleton()->validate_font(temp_font_id), false);

		font->cache_key.font_id.font_index = p_font_index;

		return _font_update_metrics(font);
	}

	return true;
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

int FontServer::font_get_oversampling(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, 1);

	return font->cache_key.font_oversampling;
}

GlyphCacheKey FontServer::font_get_cache_key(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, GlyphCacheKey{});

	return font->cache_key;
}

PoolVector<uint8_t> FontServer::font_get_data(RID p_font) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, PoolVector<uint8_t>());

	Ref<FontInfo> font_info = FontDriver::get_singleton()->get_font_info(font->cache_key.get_font_id());
	ERR_FAIL_COND_V(!font_info.is_valid(), PoolVector<uint8_t>());

	return font_info->data;
}

GlyphInfo FontServer::font_get_glyph_info(RID p_font, char32_t p_char) const {
	Font *font = font_owner.getornull(p_font);
	ERR_FAIL_COND_V(!font, GlyphInfo{});
	ERR_FAIL_COND_V(!FontDriver::get_singleton(), GlyphInfo{});

	FontID font_id = font->cache_key.get_font_id();

	uint32_t glyph_index = FontDriver::get_singleton()->get_glyph_index(font_id, p_char);

	GlyphInfo glyph_info;

	if (glyph_index) {
		glyph_info = FontDriver::get_singleton()->get_glyph_info(font->cache_key, glyph_index);
	} else {
		Vector<FontID> builtin_font_ids = FontDriver::get_singleton()->get_builtin_font_ids();
		for (int i = 0; i < builtin_font_ids.size(); i++) {
			if (builtin_font_ids[i] == font_id) {
				continue;
			}

			FontID fallback_font_id = builtin_font_ids[i];

			glyph_index = FontDriver::get_singleton()->get_glyph_index(fallback_font_id, p_char);

			if (glyph_index) {
				GlyphCacheKey temp_cache_key = font->cache_key.create_temp_key(fallback_font_id);
				if (font->temp_cache_keys.find(temp_cache_key) == -1) {
					font->temp_cache_keys.push_back(temp_cache_key);
				}

				glyph_info = FontDriver::get_singleton()->get_glyph_info(temp_cache_key, glyph_index);
				break;
			}
		}
	}

	if (p_char == 0x0020u) {
		glyph_info.advance.width += font->spacing_space_char;
	}
	if (glyph_info.found) {
		glyph_info.advance.width += font->spacing_glyph;
	}

	return glyph_info;
}

RID FontServer::font_get_glyph_texture_rid(RID p_font, const GlyphInfo &p_glyph_info) const {
	ERR_FAIL_COND_V(!FontDriver::get_singleton(), RID());
	return FontDriver::get_singleton()->get_glyph_texture_rid(p_glyph_info);
}

FontServer::FontServer() {
	singleton = this;
}

FontServer::~FontServer() {
	singleton = NULL;

	for (int i = 0; i < FontDriverManager::get_driver_count(); i++) {
		memdelete(FontDriverManager::get_driver(i));
	}

	for (int i = 0; i < TextShaperManager::get_shaper_count(); i++) {
		memdelete(TextShaperManager::get_shaper(i));
	}
}
