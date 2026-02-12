/*************************************************************************/
/*  text_helper.h                                                        */
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

#ifndef TEXT_HELPER_H
#define TEXT_HELPER_H

#include "core/hash_map.h"
#include "core/hashfuncs.h"
#include "core/math/vector2.h"
#include "core/object.h"
#include "core/rid.h"
#include "servers/font_server.h"

#include "lru_cache.h"
#include "text_shaper.h"
#include "text_types.h"

struct CharInfoCacheKey {
	uint64_t header = 0;
	String grapheme;

	_FORCE_INLINE_ bool operator==(const CharInfoCacheKey &p_key) const {
		return (header == p_key.header &&
				grapheme == p_key.grapheme);
	}

	_FORCE_INLINE_ bool operator!=(const CharInfoCacheKey &p_key) const {
		return (header != p_key.header ||
				grapheme != p_key.grapheme);
	}

	_FORCE_INLINE_ uint64_t hash() const {
		uint64_t h = header;
		h = h * 31 + grapheme.hash();
		return h;
	}
};

struct CharInfoCacheKeyHasher {
	static _FORCE_INLINE_ uint32_t hash(const CharInfoCacheKey &p_key) {
		return HashMapHasherDefault::hash(p_key.hash());
	}
};

/*************************************************************************/

class TextHelper {
	static LRUCache<CharInfoCacheKey, Vector<CharInfo>, CharInfoCacheKeyHasher> char_infos_cache;
	static LRUCache<String, Vector<String>> graphemes_cache;

	static _FORCE_INLINE_ void _process_shapeless_grapheme(CharInfo *p_char_info, char32_t p_char);
	static _FORCE_INLINE_ void _process_shaped_grapheme(CharInfo *p_char_infos, const char32_t *p_text_ptr, int p_start_idx, const String &p_grapheme, const FontID &p_font_id, const Vector<FontID> &p_fallback_font_ids, int p_font_size, int p_font_oversampling);

	static _FORCE_INLINE_ void _draw_glyph(RID p_canvas_item, RID p_font, const GlyphInfo &p_glyph_info, const Vector2 &p_pos, const Color &p_modulate, bool p_preserve_color = true);

public:
	static Vector<String> get_graphemes(const String &p_text);
	static Vector<CharInfo> get_char_infos(const Ref<TextLine> &p_text_line);

	static Vector<Ref<TextLine>> create_text_lines(RID p_font, const String &p_text);
	static Ref<TextLine> create_text_line(RID p_font, const String &p_line);

	static Vector2 get_char_size_in_text_line(const Ref<TextLine> &p_text_line, int p_char_index);
	static Vector2 draw_char_in_text_line(const Ref<TextLine> &p_text_line, int p_char_index, RID p_canvas_item, const Vector2 &p_pos, const Color &p_modulate = Color(1, 1, 1), bool p_preserve_color = true);

	static Vector2 get_text_line_size(const Ref<TextLine> &p_text_line);
	static Vector2 draw_text_line(const Ref<TextLine> &p_text_line, RID p_canvas_item, const Vector2 &p_pos, const Color &p_modulate = Color(1, 1, 1), float p_clip_w = 0.0f, bool p_preserve_color = true);
	static Vector2 draw_text_line_aligned(const Ref<TextLine> &p_text_line, RID p_canvas_item, const Vector2 &p_pos, HAlign p_align, float p_width, const Color &p_modulate = Color(1, 1, 1), bool p_preserve_color = true);

	static Vector2 get_string_size(RID p_font, const String &p_text);
	static Vector2 get_char_size(RID p_font, char32_t p_char);
	static Vector2 draw_char(RID p_canvas_item, RID p_font, const Vector2 &p_pos, char32_t p_char, const Color &p_modulate = Color(1, 1, 1), bool p_preserve_color = true);
};

#endif
