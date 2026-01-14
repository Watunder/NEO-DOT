/*************************************************************************/
/*  font_server.h                                                        */
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

#ifndef FONT_SERVER_H
#define FONT_SERVER_H

#include "configs/modules_enabled.gen.h"
#include "core/os/thread_safe.h"

#ifdef MODULE_FREETYPE_ENABLED
#include "font/freetype_wrapper.h"
#endif
#ifdef MODULE_RAQM_ENABLED
#include "font/raqm_wrapper.h"
#endif
#include "font/font_cache_key.h"
#include "font/glyph_manager.h"

class FontServer : public Object {
	GDCLASS(FontServer, Object);

	_THREAD_SAFE_CLASS_

public:
	enum SpacingType {
		SPACING_TOP,
		SPACING_BOTTOM,
		SPACING_GLYPH,
		SPACING_SPACE_CHAR,
	};

	struct Font : RID_Data {
		FontCacheKey cache_key;
		Vector<FontCacheKey> temp_cache_keys;

		float ascent;
		float descent;
		int oversampling;

		int spacing_top;
		int spacing_bottom;
		int spacing_glyph;
		int spacing_space_char;

		Font() {
			ascent = 0;
			descent = 1;
			oversampling = 1;

			spacing_top = 0;
			spacing_bottom = 0;
			spacing_glyph = 0;
			spacing_space_char = 0;
		}
	};

	mutable RID_Owner<Font> font_owner;

	struct TextData : Reference {
		RID font;
		String original_text;

#ifdef MODULE_RAQM_ENABLED
		Vector<RaqmWrapper::CharInfo> char_infos;
#endif
	};

private:
	static FontServer *singleton;

	Vector<FontID> builtin_font_ids;
	FontID default_font_id;

#ifdef MODULE_FREETYPE_ENABLED
	FreeTypeWrapper *freetype_wrapper = NULL;

	_FORCE_INLINE_ void _setup_builtin_fonts();
#endif

#ifdef MODULE_RAQM_ENABLED
	RaqmWrapper *raqm_wrapper = NULL;

	_FORCE_INLINE_ Vector<RaqmWrapper::CharInfo> _shape_string(Font *p_font, const String &p_text) const;
	_FORCE_INLINE_ GlyphManager::GlyphInfo _get_shaped_glyph_info(Font *p_font, const RaqmWrapper::ShapedGlyph &p_shaped_glyph) const;
#endif

	GlyphManager *glyph_manager = NULL;

	_FORCE_INLINE_ GlyphManager::GlyphInfo _get_simple_glyph_info(Font *p_font, char32_t p_char) const;
	_FORCE_INLINE_ void _draw_glyph(RID p_canvas_item, const GlyphManager::GlyphInfo &p_glyph_info, const Point2 &p_pos, const Color &p_modulate) const;

protected:
	static void _bind_methods();

public:
	static FontServer *get_singleton();

	RID font_create(int p_size = 1, int p_custom_flags = 0, int p_texture_flags = Texture::FLAG_MIPMAPS | Texture::FLAG_FILTER);
	void font_free(RID p_font);
	void font_set_size(RID p_font, int p_size);
	void font_set_texture_flags(RID p_font, int p_texture_flags);
	void font_set_use_mipmaps(RID p_font, bool p_use_mipmaps);
	void font_set_use_filter(RID p_font, bool p_use_filter);
	void font_set_custom_flags(RID p_font, int p_custom_flags);
	bool font_update_data(RID p_font, const PoolVector<uint8_t> &p_font_data);
	bool font_update_index(RID p_font, int p_font_index);
	bool font_update_metrics(RID p_font, int p_oversampling = -1);
	void font_clear_caches(RID p_font);
	float font_get_ascent(RID p_font) const;
	float font_get_descent(RID p_font) const;
	float font_get_oversampling(RID p_font) const;
	FontCacheKey font_get_cache_key(RID p_font) const;
	PoolVector<uint8_t> font_get_data(RID p_font) const;

	void font_set_spacing(RID p_font, SpacingType p_spcing_type, int p_spacing);
	int font_get_spacing(RID p_font, SpacingType p_spcing_type) const;

	Size2 font_get_char_size(RID p_font, char32_t p_char) const;
	Size2 font_get_string_size(RID p_font, const String &p_text) const;

	Ref<TextData> create_text_data(RID p_font, const String &p_text) const;
	float draw_text_data(const Ref<TextData> &p_text_data, RID p_canvas_item, const Point2 &p_pos, int p_char_index, const Color &p_modulate = Color(1, 1, 1)) const;

	float draw_char(RID p_canvas_item, RID p_font, const Point2 &p_pos, char32_t p_char, const Color &p_modulate = Color(1, 1, 1)) const;
	void draw_string(RID p_canvas_item, RID p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate = Color(1, 1, 1), float p_clip_w = 0.0) const;
	void draw_string_aligned(RID p_canvas_item, RID p_font, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate = Color(1, 1, 1)) const;

	FontServer();
	~FontServer();
};

VARIANT_ENUM_CAST(FontServer::SpacingType);

#endif
