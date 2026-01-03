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

static _FORCE_INLINE_ void do_font_fallback(const FT_Face &p_ft_face, const Vector<FT_Face> &p_fallback_ft_faces, raqm_t *p_raqm_context, raqm_glyph_t **r_glyphs, size_t *r_glyph_count, const uint32_t *p_text, int p_text_len) {
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
			!raqm_set_freetype_face(p_raqm_context, p_ft_face) ||
			!raqm_set_par_direction(p_raqm_context, RAQM_DIRECTION_DEFAULT)) {
		return;
	}

	for (int i = 0; i < ranges.size(); i++) {
		for (int j = 0; j < p_fallback_ft_faces.size(); j++) {
			const FT_Face &ft_face = p_fallback_ft_faces[j];
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

Vector<RaqmWrapper::ShapedInfo> RaqmWrapper::shape_single_line(const FT_Face &p_ft_face, const String &p_text, const Vector<FT_Face> &p_fallback_ft_faces) {
	Vector<ShapedInfo> shaped_infos;

	ERR_FAIL_COND_V(!p_ft_face, shaped_infos);

	if (p_text.empty()) {
		return shaped_infos;
	}

	raqm_t *raqm_context = raqm_create();

	const uint32_t *text = (const uint32_t *)p_text.c_str();
	int text_len = p_text.length();

	if (!raqm_context ||
			!raqm_set_text(raqm_context, text, text_len) ||
			!raqm_set_freetype_face(raqm_context, p_ft_face) ||
			!raqm_set_par_direction(raqm_context, RAQM_DIRECTION_DEFAULT)) {
		return shaped_infos;
	}

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = NULL;
	if (raqm_layout(raqm_context)) {
		glyphs = raqm_get_glyphs(raqm_context, &glyph_count);
	}

	if (!p_fallback_ft_faces.empty()) {
		do_font_fallback(p_ft_face, p_fallback_ft_faces, raqm_context, &glyphs, &glyph_count, text, text_len);
	}

	for (int i = 0; i < glyph_count; i++) {
		ShapedInfo shaped_info;
		shaped_info.index = glyphs[i].index;
		shaped_info.cluster = glyphs[i].cluster;
		shaped_info.offset = Vector2(glyphs[i].x_offset / 64.0, glyphs[i].y_offset / 64.0);
		shaped_info.advance = Vector2(glyphs[i].x_advance / 64.0, glyphs[i].y_advance / 64.0);
		shaped_info.ft_face = glyphs[i].ftface;

		shaped_infos.push_back(shaped_info);
	}

	raqm_clear_contents(raqm_context);

	return shaped_infos;
}

#endif
