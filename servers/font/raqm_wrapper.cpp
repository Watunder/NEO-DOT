/*************************************************************************/
/*  raqm_wrapper.cpp                                                     */
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

#include "raqm_wrapper.h"

#ifdef MODULE_RAQM_ENABLED

#include "thirdparty/libraqm/raqm.h"

static _FORCE_INLINE_ void do_font_fallback(const Vector<FT_Size> &p_ft_sizes, raqm_t *p_raqm_context, raqm_glyph_t **r_glyphs, size_t *r_glyph_count, const uint32_t *p_text, int p_text_len) {
	struct Range {
		uint32_t start;
		uint32_t len;
	};

	Vector<Range> ranges;

	uint32_t last_cluster = -1;
	for (int i = 0; i < *r_glyph_count; i++) {
		uint32_t current_cluster = (*r_glyphs)[i].cluster;
		uint32_t current_index = (*r_glyphs)[i].index;

		if (last_cluster != -1 && current_cluster != last_cluster) {
			ranges.push_back(Range{ last_cluster, current_cluster - last_cluster });
			last_cluster = -1;
		}
		if (!current_index) {
			if (last_cluster == -1) {
				last_cluster = current_cluster;
			}
		}
	}
	if (last_cluster != -1) {
		ranges.push_back(Range{ last_cluster, p_text_len - last_cluster });
	}

	if (ranges.size() <= 0) {
		return;
	}

	raqm_clear_contents(p_raqm_context);
	if (!p_raqm_context ||
			!raqm_set_text(p_raqm_context, p_text, p_text_len) ||
			!raqm_set_freetype_face(p_raqm_context, p_ft_sizes[0]->face) ||
			!raqm_set_par_direction(p_raqm_context, RAQM_DIRECTION_DEFAULT)) {
		return;
	}

	for (int i = 0; i < ranges.size(); i++) {
		for (int j = 1; j < p_ft_sizes.size(); j++) {
			const FT_Face &ft_face = p_ft_sizes[j]->face;
			uint32_t glyph_index = FT_Get_Char_Index(ft_face, p_text[ranges[i].start]);
			if (glyph_index &&
					raqm_set_freetype_face_range(p_raqm_context, ft_face, ranges[i].start, ranges[i].len)) {
				break;
			}
		}
	}

	if (raqm_layout(p_raqm_context)) {
		*r_glyphs = raqm_get_glyphs(p_raqm_context, r_glyph_count);
	}
}

void RaqmWrapper::update_cache(uint64_t p_cache_key) {
	if (current_cache_key != p_cache_key) {
		current_cache_key = p_cache_key;
	}

	if (!char_info_map.has(current_cache_key)) {
		char_info_map[current_cache_key] = HashMap<String, Vector<CharInfo>>();
	}
}

void RaqmWrapper::clear_cache(uint64_t p_cache_key) {
	if (char_info_map.has(current_cache_key)) {
		char_info_map.erase(current_cache_key);
	}
}

static _FORCE_INLINE_ bool have_glyph(uint32_t p_codepoint) {
	return (p_codepoint != 0x200Cu || p_codepoint != 0x200Du);
};

Vector<RaqmWrapper::CharInfo> RaqmWrapper::get_char_infos(const Vector<FT_Size> &p_ft_sizes, const String &p_text) {
	if (char_info_map[current_cache_key].has(p_text)) {
		return char_info_map[current_cache_key][p_text];
	}

	Vector<CharInfo> char_infos;

	if (p_text.empty()) {
		return char_infos;
	}

	raqm_t *raqm_context = raqm_create();

	const uint32_t *text = (const uint32_t *)p_text.c_str();
	int text_len = p_text.length();

	if (!raqm_context ||
			!raqm_set_text(raqm_context, text, text_len) ||
			!raqm_set_freetype_face(raqm_context, p_ft_sizes[0]->face) ||
			!raqm_set_par_direction(raqm_context, RAQM_DIRECTION_DEFAULT)) {
		return char_infos;
	}

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = NULL;
	if (raqm_layout(raqm_context)) {
		glyphs = raqm_get_glyphs(raqm_context, &glyph_count);
	}

	if (p_ft_sizes.size() > 1) {
		do_font_fallback(p_ft_sizes, raqm_context, &glyphs, &glyph_count, text, text_len);
	}

	char_infos.resize(text_len);

	Vector<ShapedGlyph> shaped_glyphs;
	shaped_glyphs.resize(glyph_count);

	for (int i = 0; i < glyph_count;) {
		int start = i;
		int end = i;
		while (end < glyph_count && glyphs[end].cluster == glyphs[start].cluster) {
			end++;
		}
		i = end;
		int part_count = end - start;

		Vector<int> invisible_glyphs;

		for (int j = 0; j < part_count; j++) {
			ShapedGlyph shaped_glyph;
			shaped_glyph.index = glyphs[j + start].index;
			shaped_glyph.cluster = glyphs[j + start].cluster;
			shaped_glyph.offset = Vector2(glyphs[j + start].x_offset / 64.0, -glyphs[j + start].y_offset / 64.0);
			shaped_glyph.advance = Vector2(glyphs[j + start].x_advance / 64.0, glyphs[j + start].y_advance / 64.0);
			shaped_glyph.ft_face = glyphs[j + start].ftface;

			if (shaped_glyph.offset == Vector2() && shaped_glyph.advance == Vector2()) {
				invisible_glyphs.push_back(j);
			}

			shaped_glyphs.write[j + start] = shaped_glyph;
		}

		uint32_t current_cluster = glyphs[start].cluster;
		uint32_t next_cluster = (end < glyph_count) ? glyphs[end].cluster : text_len;

		for (int part_index = 0, c = current_cluster; c < next_cluster; c++) {
			CharInfo char_info;
			char_info.codepoint = text[c];
			char_info.part_count = part_count;

			if (part_index < part_count && have_glyph(char_info.codepoint) || invisible_glyphs.find(part_index + start) != -1) {
				char_info.glyph = shaped_glyphs[part_index + start];
				char_info.part_index = part_index;
				part_index++;
			}

			char_infos.write[c] = char_info;
		}
	}

	raqm_destroy(raqm_context);

	char_info_map[current_cache_key][p_text] = char_infos;

	return char_infos;
}

#endif
