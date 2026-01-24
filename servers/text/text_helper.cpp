/*************************************************************************/
/*  text_helper.cpp                                                      */
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

#include "text_helper.h"

#include "servers/font_server.h"
#include "servers/text/text_shaper.h"
#include "servers/visual_server.h"

#include <graphemebreak.h>
#include <linebreak.h>

LRUCache<uint64_t, Vector<CharInfo>> TextHelper::char_infos_cache(1024);
LRUCache<String, Vector<String>> TextHelper::graphemes_cache(1024);

_FORCE_INLINE_ void TextHelper::_process_shapeless_grapheme(CharInfo *p_char_info, char32_t p_char) {
	p_char_info->set_type(CharInfo::SHAPELESS);
	p_char_info->set_char_code(p_char);
}

_FORCE_INLINE_ int TextHelper::_process_shaped_grapheme(CharInfo *p_char_infos, const char32_t *p_text_ptr, int p_start_idx, const String &p_grapheme, const FontID &p_font_id, const Vector<FontID> &p_fallback_font_ids, int p_font_size, int p_font_oversampling) {
	int grapheme_len = p_grapheme.length();

	if (!TextShaper::get_singleton()) {
		for (int i = 0; i < grapheme_len; i++) {
			p_char_infos[i].set_type(CharInfo::SHAPELESS);
			p_char_infos[i].set_char_code(p_text_ptr[p_start_idx + i]);
		}
		return grapheme_len;
	}

	Vector<ShapedData *> shaped_datas = TextShaper::get_singleton()->shape_text(
			p_font_id, p_fallback_font_ids, p_grapheme, p_font_size, p_font_oversampling);

	int shaped_count = shaped_datas.size();

	for (int i = 0; i < grapheme_len; i++) {
		if (i < shaped_count && shaped_datas[i]) {
			p_char_infos[i].set_shaped_data(shaped_datas[i]);
			p_char_infos[i].set_char_code(p_text_ptr[p_start_idx + i]);
		} else {
			p_char_infos[i].set_type(CharInfo::INVISIBLE);
			p_char_infos[i].set_char_code(p_text_ptr[p_start_idx + i]);
		}
	}

	for (int i = grapheme_len; i < shaped_count; i++) {
		if (shaped_datas[i]) {
			memdelete(shaped_datas[i]);
		}
	}

	return grapheme_len;
}

_FORCE_INLINE_ void TextHelper::_draw_glyph(RID p_canvas_item, RID p_font, const GlyphInfo &p_glyph_info, const Vector2 &p_pos, const Color &p_modulate) {
	if (!p_glyph_info.found) {
		return;
	}

	RID texture_rid = FontServer::get_singleton()->font_get_glyph_texture_rid(p_font, p_glyph_info);
	if (!texture_rid.is_valid()) {
		return;
	}

	Color modulate = p_modulate;
	if (p_glyph_info.texture_format == Image::FORMAT_RGBA8) {
		modulate.r = modulate.g = modulate.b = 1.0;
	}

	Vector2 texture_pos = p_pos + p_glyph_info.texture_offset;
	Rect2 texture_rect(texture_pos, p_glyph_info.texture_size);

	VisualServer::get_singleton()->canvas_item_add_texture_rect_region(p_canvas_item, texture_rect, texture_rid, p_glyph_info.texture_rect_uv, modulate, false, RID(), false);
}

Vector<String> TextHelper::get_graphemes(const String &p_text) {
	const Vector<String> *cached = graphemes_cache.getptr(p_text);
	if (cached) {
		return *cached;
	}

	Vector<String> graphemes;

	const char32_t *text_ptr = p_text.ptr();
	int text_len = p_text.length();

	if (text_len == 0) {
		graphemes_cache.insert(p_text, graphemes);
		return graphemes;
	}

	if (text_len == 1) {
		graphemes.push_back(p_text);
		graphemes_cache.insert(p_text, graphemes);
		return graphemes;
	}

	Vector<char> breaks;
	breaks.resize(text_len + 1);

	set_graphemebreaks_utf32((const utf32_t *)text_ptr, text_len, "", breaks.ptrw());

	const char *breaks_ptr = breaks.ptr();

	for (int i = 0; i < text_len;) {
		int start = i;
		int end = i;
		while (end < text_len && breaks_ptr[end] == GRAPHEMEBREAK_NOBREAK) {
			end++;
		}
		end++;
		i = end;
		int len = end - start;

		if (len > 0) {
			String grapheme(text_ptr + start, len);
			graphemes.push_back(grapheme);
		}
	}

	graphemes_cache.insert(p_text, graphemes);

	return graphemes;
}

Vector<CharInfo> TextHelper::get_char_infos(const Ref<TextLine> &p_text_line) {
	ERR_FAIL_COND_V(!p_text_line.is_valid(), Vector<CharInfo>());

	Vector<CharInfo> char_infos;
	int line_len = p_text_line->original_line.length();

	if (line_len == 0) {
		return char_infos;
	}

	char_infos.resize(line_len);
	CharInfo *char_infos_ptr = char_infos.ptrw();
	const char32_t *line_ptr = p_text_line->original_line.ptr();

	Vector<String> graphemes = get_graphemes(p_text_line->original_line);
	int char_idx = 0;

	for (int i = 0; i < graphemes.size(); i++) {
		const String &grapheme = graphemes[i];
		int grapheme_len = grapheme.length();

		uint64_t h = p_text_line->cache_header;
		h = h * 31 + grapheme.hash();

		const Vector<CharInfo> *cached = char_infos_cache.getptr(h);
		if (cached) {
			for (int j = 0; j < grapheme_len; j++) {
				char_infos_ptr[char_idx + j] = (*cached)[j];
			}
			char_idx += grapheme_len;
		} else {
			int processed = 0;
			if (grapheme_len == 1) {
				_process_shapeless_grapheme(&char_infos_ptr[char_idx], line_ptr[char_idx]);
				processed = 1;
			} else {
				processed = _process_shaped_grapheme(
						&char_infos_ptr[char_idx], line_ptr, char_idx, grapheme,
						p_text_line->font_id, p_text_line->fallback_font_ids,
						p_text_line->font_size, p_text_line->font_oversampling);
			}
			char_idx += processed;

			Vector<CharInfo> sub_char_infos;
			sub_char_infos.resize(grapheme_len);

			for (int j = 0; j < grapheme_len; j++) {
				sub_char_infos.write[j] = char_infos_ptr[char_idx - grapheme_len + j];
			}

			char_infos_cache.insert(h, sub_char_infos);
		}
	}

	return char_infos;
}

Ref<TextLine> TextHelper::create_text_line(RID p_font, const String &p_line) {
	ERR_FAIL_COND_V(!p_font.is_valid(), Ref<TextLine>());

	Ref<TextLine> text_line;
	text_line.instance();

	text_line->font = p_font;
	text_line->original_line = p_line;

	text_line->spacing_glyph = FontServer::get_singleton()->font_get_spacing(p_font, FontServer::SPACING_GLYPH);
	text_line->spacing_space_char = FontServer::get_singleton()->font_get_spacing(p_font, FontServer::SPACING_SPACE_CHAR);

	text_line->height = FontServer::get_singleton()->font_get_ascent(p_font) + FontServer::get_singleton()->font_get_descent(p_font);

	GlyphCacheKey cache_key = FontServer::get_singleton()->font_get_cache_key(p_font);
	text_line->font_id = cache_key.get_font_id();
	text_line->font_size = cache_key.font_size;
	text_line->font_oversampling = cache_key.font_oversampling;

	Vector<FontID> builtin_font_ids = FontDriver::get_singleton()->get_builtin_font_ids();
	for (int i = 0; i < builtin_font_ids.size(); i++) {
		if (builtin_font_ids[i] != text_line->font_id) {
			text_line->fallback_font_ids.push_back(builtin_font_ids[i]);
		}
	}

	uint64_t h = text_line->font_id.hash();
	for (int i = 0; i < text_line->fallback_font_ids.size(); i++) {
		h = h * 31 + text_line->fallback_font_ids[i].hash();
	}
	h = h * 31 + text_line->font_size;
	h = h * 31 + text_line->font_oversampling;

	text_line->cache_header = h;

	text_line->char_infos = get_char_infos(text_line);

	return text_line;
}

Vector<Ref<TextLine>> TextHelper::create_text_lines(RID p_font, const String &p_text) {
	ERR_FAIL_COND_V(!p_font.is_valid(), Vector<Ref<TextLine>>());

	Vector<Ref<TextLine>> text_lines;

	const char32_t *text_ptr = p_text.ptr();
	int text_len = p_text.length();

	if (text_len == 0) {
		Ref<TextLine> empty_line = create_text_line(p_font, "");
		text_lines.push_back(empty_line);
		return text_lines;
	}

	Vector<char> breaks;
	breaks.resize(text_len + 1);
	set_linebreaks_utf32((const utf32_t *)text_ptr, text_len, "", breaks.ptrw());

	Vector<int> line_starts;
	line_starts.push_back(0);

	for (int i = 0; i < text_len; i++) {
		if (breaks[i] == LINEBREAK_MUSTBREAK) {
			line_starts.push_back(i + 1);
		}
	}
	line_starts.push_back(text_len);

	for (int line_idx = 0; line_idx < line_starts.size() - 1; line_idx++) {
		int line_start = line_starts[line_idx];
		int line_end = line_starts[line_idx + 1];
		int line_len = line_end - line_start;

		String line(text_ptr + line_start, line_len);

		Ref<TextLine> text_line = create_text_line(p_font, line);
		text_lines.push_back(text_line);
	}

	return text_lines;
}

Vector2 TextHelper::draw_char_in_text_line(const Ref<TextLine> &p_text_line, int p_char_index, RID p_canvas_item, const Vector2 &p_pos, const Color &p_modulate) {
	Vector2 ofs;

	ERR_FAIL_COND_V(!p_text_line.is_valid(), ofs);
	ERR_FAIL_COND_V(!p_text_line->font.is_valid(), ofs);
	ERR_FAIL_INDEX_V(p_char_index, p_text_line->char_infos.size(), ofs);

	const CharInfo &char_info = p_text_line->char_infos[p_char_index];

	if (char_info.get_type() == CharInfo::SHAPELESS) {
		const GlyphInfo &glyph_info = FontServer::get_singleton()->font_get_glyph_info(p_text_line->font, char_info.get_char_code());
		_draw_glyph(p_canvas_item, p_text_line->font, glyph_info, p_pos + ofs, p_modulate);

		ofs += glyph_info.advance;
	} else if (char_info.get_type() == CharInfo::SHAPED) {
		GlyphCacheKey cache_key = FontServer::get_singleton()->font_get_cache_key(p_text_line->font);
		GlyphCacheKey temp_cache_key = cache_key.create_temp_key(char_info.get_glyph_font_id());

		const GlyphInfo &glyph_info = FontDriver::get_singleton()->get_glyph_info(temp_cache_key, char_info.get_glyph_index());
		_draw_glyph(p_canvas_item, p_text_line->font, glyph_info, p_pos + ofs + char_info.get_glyph_offset(), p_modulate);

		ofs += char_info.get_glyph_advance();
		if (char_info.get_char_code() == 0x0020u) {
			ofs.width += p_text_line->spacing_space_char;
		}
		if (char_info.is_cluster_end()) {
			ofs.width += p_text_line->spacing_glyph;
		}
	}

	return ofs;
}

Vector2 TextHelper::get_char_size_in_text_line(const Ref<TextLine> &p_text_line, int p_char_index) {
	Vector2 size(0, p_text_line->height);

	ERR_FAIL_COND_V(!p_text_line.is_valid(), size);
	ERR_FAIL_COND_V(!p_text_line->font.is_valid(), size);
	ERR_FAIL_INDEX_V(p_char_index, p_text_line->char_infos.size(), size);

	const CharInfo &char_info = p_text_line->char_infos[p_char_index];

	if (char_info.get_type() == CharInfo::SHAPELESS) {
		const GlyphInfo &glyph_info = FontServer::get_singleton()->font_get_glyph_info(p_text_line->font, char_info.get_char_code());
		size.x += glyph_info.advance.x;
	} else if (char_info.get_type() == CharInfo::SHAPED) {
		size.x += char_info.get_glyph_advance().x;

		if (char_info.get_char_code() == 0x0020u) {
			size.x += p_text_line->spacing_space_char;
		}
		if (char_info.is_cluster_end()) {
			size.x += p_text_line->spacing_glyph;
		}
	}

	return size;
}

Vector2 TextHelper::draw_text_line(const Ref<TextLine> &p_text_line, RID p_canvas_item, const Vector2 &p_pos, const Color &p_modulate, float p_clip_w) {
	ERR_FAIL_COND_V(!p_text_line.is_valid(), Vector2());
	ERR_FAIL_COND_V(!p_text_line->font.is_valid(), Vector2());

	Vector2 ofs;
	for (int i = 0; i < p_text_line->char_infos.size(); i++) {
		if (p_clip_w > 0.0 && ofs.x > p_clip_w) {
			const CharInfo &char_info = p_text_line->char_infos[i];
			if (i == 0 || char_info.get_type() != CharInfo::SHAPED || char_info.is_cluster_end()) {
				break;
			}
		}
		ofs += draw_char_in_text_line(p_text_line, i, p_canvas_item, p_pos + ofs, p_modulate);
	}

	return ofs;
}

Vector2 TextHelper::get_text_line_size(const Ref<TextLine> &p_text_line) {
	Vector2 size(0, p_text_line->height);

	ERR_FAIL_COND_V(!p_text_line.is_valid(), size);
	ERR_FAIL_COND_V(!p_text_line->font.is_valid(), size);

	for (int i = 0; i < p_text_line->char_infos.size(); i++) {
		Vector2 char_size = get_char_size_in_text_line(p_text_line, i);
		size.x += char_size.x;
	}

	return size;
}

Vector2 TextHelper::draw_text_line_aligned(const Ref<TextLine> &p_text_line, RID p_canvas_item, const Vector2 &p_pos, HAlign p_align, float p_width, const Color &p_modulate) {
	ERR_FAIL_COND_V(!p_text_line.is_valid(), Vector2());
	ERR_FAIL_COND_V(!p_text_line->font.is_valid(), Vector2());

	float length = get_text_line_size(p_text_line).width;
	if (length >= p_width) {
		return draw_text_line(p_text_line, p_canvas_item, p_pos, p_modulate, p_width);
	}

	float ofs = 0.0f;
	switch (p_align) {
		case HALIGN_LEFT:
			ofs = 0.0;
			break;
		case HALIGN_CENTER:
			ofs = (p_width - length) / 2.0;
			break;
		case HALIGN_RIGHT:
			ofs = p_width - length;
			break;
	}

	return draw_text_line(p_text_line, p_canvas_item, p_pos + Vector2(ofs, 0), p_modulate, p_width);
}

Vector2 TextHelper::get_char_size(RID p_font, char32_t p_char) {
	ERR_FAIL_COND_V(!p_font.is_valid(), Vector2());

	GlyphInfo glyph_info = FontServer::get_singleton()->font_get_glyph_info(p_font, p_char);
	float height = FontServer::get_singleton()->font_get_ascent(p_font) + FontServer::get_singleton()->font_get_descent(p_font);
	return Vector2(glyph_info.advance.x, height);
}

Vector2 TextHelper::get_string_size(RID p_font, const String &p_text) {
	ERR_FAIL_COND_V(!p_font.is_valid(), Vector2());

	float width = 0.0f;
	for (int i = 0; i < p_text.length(); i++) {
		width += get_char_size(p_font, p_text[i]).x;
	}
	float height = FontServer::get_singleton()->font_get_ascent(p_font) + FontServer::get_singleton()->font_get_descent(p_font);
	return Vector2(width, height);
}

Vector2 TextHelper::draw_char(RID p_canvas_item, RID p_font, const Vector2 &p_pos, char32_t p_char, const Color &p_modulate) {
	ERR_FAIL_COND_V(!p_font.is_valid(), Vector2());

	const GlyphInfo &glyph_info = FontServer::get_singleton()->font_get_glyph_info(p_font, p_char);
	_draw_glyph(p_canvas_item, p_font, glyph_info, p_pos, p_modulate);

	return glyph_info.advance;
}
