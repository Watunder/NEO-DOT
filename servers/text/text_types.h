/*************************************************************************/
/*  text_types.h                                                         */
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

#ifndef TEXT_TYPES_H
#define TEXT_TYPES_H

#include "core/error_macros.h"
#include "core/math/vector2.h"
#include "core/os/memory.h"
#include "core/reference.h"
#include "core/rid.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "servers/font_server.h"

struct ShapedData {
	FontID glyph_font_id;

	uint32_t glyph_index;
	Vector2 glyph_offset;
	Vector2 glyph_advance;

	int cluster_glyph_count;
	int cluster_glyph_index;

	ShapedData() :
			glyph_index(0),
			glyph_offset(0, 0),
			glyph_advance(0, 0),
			cluster_glyph_count(0),
			cluster_glyph_index(-1) {}
};

struct CharInfo {
	enum Type {
		INVISIBLE,
		SHAPELESS,
		SHAPED,
	};

private:
	Type type;
	char32_t char_code;
	ShapedData *shaped;

public:
	_FORCE_INLINE_ Type get_type() const {
		return type;
	}

	_FORCE_INLINE_ void set_type(Type p_type) {
		type = p_type;
	}

	_FORCE_INLINE_ char32_t get_char_code() const {
		return char_code;
	}

	_FORCE_INLINE_ void set_char_code(char32_t p_char_code) {
		char_code = p_char_code;
	}

	_FORCE_INLINE_ bool is_cluster_end() const {
		return (shaped->cluster_glyph_index + 1 == shaped->cluster_glyph_count);
	}

	_FORCE_INLINE_ FontID get_glyph_font_id() const {
		ERR_FAIL_COND_V(!shaped, FontID());
		return shaped->glyph_font_id;
	}

	_FORCE_INLINE_ uint32_t get_glyph_index() const {
		ERR_FAIL_COND_V(!shaped, 0);
		return shaped->glyph_index;
	}

	_FORCE_INLINE_ Vector2 get_glyph_offset() const {
		ERR_FAIL_COND_V(!shaped, Vector2());
		return shaped->glyph_offset;
	}

	_FORCE_INLINE_ Vector2 get_glyph_advance() const {
		ERR_FAIL_COND_V(!shaped, Vector2());
		return shaped->glyph_advance;
	}

	_FORCE_INLINE_ void set_shaped_data(ShapedData *p_shaped_data) {
		if (shaped) {
			memdelete(shaped);
		}
		type = SHAPED;
		shaped = p_shaped_data;
	}

	CharInfo() :
			type(INVISIBLE),
			char_code(0),
			shaped(NULL) {}

	CharInfo &operator=(const CharInfo &p_other) {
		if (this == &p_other) {
			return *this;
		}

		type = p_other.type;
		char_code = p_other.char_code;

		if (shaped) {
			memdelete(shaped);
		}
		if (p_other.shaped) {
			shaped = memnew(ShapedData(*p_other.shaped));
		}

		return *this;
	}

	~CharInfo() {
		if (shaped) {
			memdelete(shaped);
		}
	}
};

struct TextLine : Reference {
	RID font;
	String original_line;

	float height = 0.0f;
	int spacing_glyph = 0;
	int spacing_space_char = 0;

	FontID font_id;
	Vector<FontID> fallback_font_ids;
	uint16_t font_size = 1;
	uint8_t font_oversampling = 1;

	uint64_t cache_header = 0;

	Vector<CharInfo> char_infos;
};

#endif
