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

	if (!index_to_face_map.has(current_cache_key)) {
		index_to_face_map[current_cache_key] = HashMap<uint32_t, FT_Face>();
	}

	if (!char_info_map.has(current_cache_key)) {
		char_info_map[current_cache_key] = HashMap<String, Vector<CharInfo>>();
	}
}

void RaqmWrapper::clear_cache(uint64_t p_cache_key) {
	if (index_to_face_map.has(current_cache_key)) {
		index_to_face_map.erase(current_cache_key);
	}

	if (char_info_map.has(current_cache_key)) {
		char_info_map.erase(current_cache_key);
	}
}

static _FORCE_INLINE_ bool have_glyph(uint32_t p_char_code) {
	return (p_char_code != 0x200Cu || p_char_code != 0x200Du);
};

Vector<CharInfo> RaqmWrapper::get_char_infos(const Vector<FT_Size> &p_ft_sizes, const String &p_text) {
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
			raqm_glyph_t glyph = glyphs[j + start];
			if (glyph.x_offset == 0 && glyph.y_offset == 0 && glyph.x_advance == 0 && glyph.y_advance == 0) {
				invisible_glyphs.push_back(j);
			}
			if (!index_to_face_map[current_cache_key].has(glyph.index)) {
				index_to_face_map[current_cache_key][glyph.index] = glyph.ftface;
			}
		}

		uint32_t current_cluster = glyphs[start].cluster;
		uint32_t next_cluster = (end < glyph_count) ? glyphs[end].cluster : text_len;

		for (int part_index = 0, c = current_cluster; c < next_cluster; c++) {
			CharInfo char_info;
			char_info.char_code = text[c];
			char_info.part_count = part_count;

			if (part_index < part_count && have_glyph(char_info.char_code) || invisible_glyphs.find(part_index + start) != -1) {
				char_info.type = CharInfo::SHAPED;
				char_info.part_index = part_index;

				raqm_glyph_t glyph = glyphs[part_index + start];
				char_info.glyph_index = glyph.index;
				char_info.glyph_offset = Vector2(glyph.x_offset / 64.0, -glyph.y_offset / 64.0);
				char_info.glyph_advance = Vector2(glyph.x_advance / 64.0, glyph.y_advance / 64.0);

				part_index++;
			}

			char_infos.write[c] = char_info;
		}
	}

	raqm_destroy(raqm_context);

	char_info_map[current_cache_key][p_text] = char_infos;

	return char_infos;
}

FT_Face RaqmWrapper::get_ft_face(uint32_t p_glyph_index) {
	ERR_FAIL_COND_V(!index_to_face_map.has(current_cache_key), NULL);
	ERR_FAIL_COND_V(!index_to_face_map[current_cache_key].has(p_glyph_index), NULL);

	return index_to_face_map[current_cache_key][p_glyph_index];
}

#endif
