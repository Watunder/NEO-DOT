/*************************************************************************/
/*  text_shaper_raqm.cpp                                                 */
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

#include "text_shaper_raqm.h"

#include "core/os/memory.h"
#include "servers/font_server.h"

#include "modules/freetype/font_driver_freetype.h"

#include <raqm.h>

#include <ft2build.h>
#include FT_FREETYPE_H

static _FORCE_INLINE_ bool _have_glyph(char32_t p_char_code) {
	return (p_char_code != 0x200Cu && p_char_code != 0x200Du);
}

static _FORCE_INLINE_ bool _font_covers_all(const FontID &p_font, const char32_t *p_text, int p_len) {
	if (!FontDriver::get_singleton()) {
		return false;
	}

	for (int i = 0; i < p_len; i++) {
		if (FontDriver::get_singleton()->get_glyph_index(p_font, p_text[i]) == 0) {
			return false;
		}
	}

	return true;
}

static _FORCE_INLINE_ FontID _select_font_for_text(const FontID &p_primary, const Vector<FontID> &p_fallbacks, const char32_t *p_text, int p_len) {
	if (_font_covers_all(p_primary, p_text, p_len)) {
		return p_primary;
	}

	for (int i = 0; i < p_fallbacks.size(); i++) {
		if (_font_covers_all(p_fallbacks[i], p_text, p_len)) {
			return p_fallbacks[i];
		}
	}

	return p_primary;
}

static _FORCE_INLINE_ void _shape_run(FontDriverFreeType *p_driver, const char32_t *p_text_ptr, int p_start, int p_end, const FontID &p_run_font, int p_font_size, int p_font_oversampling, Vector<ShapedData *> &r_out) {
	const int run_len = p_end - p_start;
	if (run_len <= 0) {
		return;
	}

	FT_Face face = p_driver->get_ft_face(p_run_font);
	ERR_FAIL_COND(!face);

	FT_Size size = p_driver->get_ft_size(p_run_font, p_font_size, p_font_oversampling);
	ERR_FAIL_COND(!size);

	raqm_t *rq = raqm_create();
	ERR_FAIL_COND(!rq);

	if (!raqm_set_text(rq, (const uint32_t *)(p_text_ptr + p_start), (size_t)run_len)) {
		raqm_destroy(rq);
		ERR_FAIL();
	}

	if (!raqm_set_freetype_face(rq, face)) {
		raqm_destroy(rq);
		ERR_FAIL();
	}

	if (!raqm_set_par_direction(rq, RAQM_DIRECTION_DEFAULT)) {
		raqm_destroy(rq);
		ERR_FAIL();
	}

	if (!raqm_layout(rq)) {
		raqm_destroy(rq);
		ERR_FAIL();
	}

	size_t glyph_count = 0;
	raqm_glyph_t *glyphs = raqm_get_glyphs(rq, &glyph_count);
	if (!glyphs || glyph_count == 0) {
		raqm_destroy(rq);
		return;
	}

	int i = 0;
	while (i < (int)glyph_count) {
		const uint32_t cluster = glyphs[i].cluster;
		int j = i + 1;
		while (j < (int)glyph_count && glyphs[j].cluster == cluster) {
			j++;
		}

		uint32_t next_cluster = (uint32_t)run_len;
		if (j < (int)glyph_count) {
			next_cluster = glyphs[j].cluster;
		}
		if (next_cluster < cluster) {
			next_cluster = (uint32_t)run_len;
		}

		const int cluster_glyph_count = (j - i);

		Vector<uint8_t> invisible_mask;
		invisible_mask.resize(cluster_glyph_count);

		for (int g = 0; g < cluster_glyph_count; g++) {
			const raqm_glyph_t &glyph = glyphs[i + g];
			if (glyph.x_offset == 0 && glyph.y_offset == 0 && glyph.x_advance == 0 && glyph.y_advance == 0) {
				invisible_mask.write[g] = 1;
			} else {
				invisible_mask.write[g] = 0;
			}
		}

		int cluster_glyph_index = 0;
		for (int c = (int)cluster; c < (int)next_cluster; c++) {
			if (cluster_glyph_index < cluster_glyph_count &&
					(_have_glyph(p_text_ptr[p_start + c]) || (invisible_mask[cluster_glyph_index] != 0))) {
				const raqm_glyph_t &glyph = glyphs[i + cluster_glyph_index];

				ShapedData *sd = memnew(ShapedData);
				sd->glyph_font_id = p_run_font;
				sd->glyph_index = glyph.index;
				sd->glyph_offset = Vector2((float)glyph.x_offset / 64.0, -(float)glyph.y_offset / 64.0);
				sd->glyph_advance = Vector2((float)glyph.x_advance / 64.0, (float)glyph.y_advance / 64.0);
				sd->cluster_glyph_count = cluster_glyph_count;
				sd->cluster_glyph_index = cluster_glyph_index;

				r_out.write[p_start + c] = sd;
				cluster_glyph_index++;
			} else {
				r_out.write[p_start + c] = NULL;
			}
		}

		i = j;
	}

	raqm_destroy(rq);
}

Vector<ShapedData *> TextShaperRaqm::shape_text(const FontID &p_font_id, const Vector<FontID> &p_fallback_font_ids, const String &p_text, int p_font_size, int p_font_oversampling) {
	Vector<ShapedData *> out;
	const int len = p_text.length();
	out.resize(len);

	if (len == 0) {
		return out;
	}

	FontDriverFreeType *driver = dynamic_cast<FontDriverFreeType *>(FontDriver::get_singleton());
	ERR_FAIL_COND_V(!driver, out);

	const char32_t *text_ptr = p_text.ptr();

	FontID run_font = _select_font_for_text(p_font_id, p_fallback_font_ids, text_ptr, len);

	_shape_run(driver, text_ptr, 0, len, run_font, p_font_size, p_font_oversampling, out);

	return out;
}
