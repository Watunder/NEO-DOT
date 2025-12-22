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
#ifdef MODULE_FREETYPE_ENABLED
#include <ft2build.h>
#include FT_FREETYPE_H
#include "font/freetype_wrapper.h"
#endif

#include "core/os/thread_safe.h"
#include "scene/resources/font.h"

#include "font/font_cache_key.h"
#include "font/glyph_manager.h"
#include "font/text_manager.h"

class FontServer : public Object {
	GDCLASS(FontServer, Object);

	_THREAD_SAFE_CLASS_

private:
	static FontServer *singleton;

	struct FontInfo : RID_Data {
		FontCacheKey cache_key;

		float ascent;
		float descent;
		float oversampling;

		FontInfo() {
			cache_key.font_size = 16;
			cache_key.font_use_mipmaps = 1;
			cache_key.font_use_filter = 1;
			cache_key.font_force_autohinter = 0;
			cache_key.font_hinting = 2;
			cache_key.font_hash = 0;

			ascent = 0;
			descent = 1;
			oversampling = 1;
		}
	};

	mutable RID_Owner<FontInfo> font_info_owner;

#ifdef MODULE_FREETYPE_ENABLED
	FreeTypeWrapper *freetype_wrapper = NULL;
#endif

	GlyphManager *glyph_manager = NULL;
	TextManger *text_manager = NULL;

protected:
	static void _bind_methods();

public:
	static FontServer *get_singleton();

	RID font_create();
	void font_free(RID p_font_rid);
	void font_set_size(RID p_font_rid, int p_size);
	void font_set_use_mipmaps(RID p_font_rid, bool p_use_mipmaps);
	void font_set_use_filter(RID p_font_rid, bool p_use_filter);
	void font_set_force_autohinter(RID p_font_rid, bool p_force_autohinter);
	void font_set_hinting(RID p_font_rid, int p_hinting);
	void font_set_face_index(RID p_font_rid, int p_face_index);
	void font_set_data(RID p_font_rid, const PoolVector<uint8_t> &p_font_buffer);
	void font_update_metrics(RID p_font_rid, float p_oversampling = 1);
	float font_get_ascent(RID p_font_rid) const;
	float font_get_descent(RID p_font_rid) const;
	float font_get_oversampling(RID p_font_rid) const;
	const FontCacheKey &font_get_cache_key(RID p_font_rid) const;

	Size2 get_char_size(const Ref<Font> &pFontCache, char32_t p_char) const;
	float draw_char(RID p_canvas_item, const Ref<Font> &pFontCache, const Point2 &p_pos, char32_t p_char, const Color &p_modulate = Color(1, 1, 1)) const;
	void draw_string(RID p_canvas_item, const Ref<Font> &pFontCache, const Point2 &p_pos, const String &p_text, const Color &p_modulate = Color(1, 1, 1), int p_clip_w = -1) const;
	void draw_string_aligned(RID p_canvas_item, const Ref<Font> &pFontCache, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate = Color(1, 1, 1)) const;

	FontServer();
	~FontServer();
};

#endif
