/*************************************************************************/
/*  text_data.h                                                          */
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

#ifndef TEXT_DATA_H
#define TEXT_DATA_H

#include "core/reference.h"
#include "core/rid.h"
#include "core/ustring.h"
#include "core/vector.h"

struct CharInfo {
	enum Type {
		INVISIBLE,
		SHAPELESS,
		SHAPED,
	};

	Type type;
	uint32_t char_code;

	int part_count = 0;
	int part_index = -1;
	uint32_t glyph_index = 0;
	Vector2 glyph_offset;
	Vector2 glyph_advance;

	CharInfo() :
			type(INVISIBLE),
			char_code(0) {}

	_FORCE_INLINE_ bool is_last_part() const {
		return (part_index + 1 == part_count);
	};
};

struct TextData : Reference {
	RID font;
	String original_text;

	Vector<CharInfo> char_infos;
};

#endif
