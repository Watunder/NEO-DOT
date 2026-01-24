/*************************************************************************/
/*  text_shaper.h                                                        */
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

#ifndef TEXT_SHAPER_H
#define TEXT_SHAPER_H

#include "core/error_list.h"
#include "core/math/vector2.h"
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

/*************************************************************************/

class TextShaper {
	static TextShaper *singleton;

public:
	virtual Error init() = 0;

	static TextShaper *get_singleton();
	void set_singleton();

	virtual const char *get_name() const = 0;

	virtual Vector<ShapedData *> shape_text(const FontID &p_font_id, const Vector<FontID> &p_fallback_font_ids, const String &p_text, int p_font_size, int p_font_oversampling) = 0;

	TextShaper() {}
	virtual ~TextShaper() {}
};

class TextShaperManager {
	enum {
		MAX_SHAPERS = 10
	};

	static TextShaper *shapers[MAX_SHAPERS];
	static int shaper_count;

public:
	static void add_shaper(TextShaper *p_shaper);
	static void initialize(int p_shaper);
	static int get_shaper_count();
	static TextShaper *get_shaper(int p_shaper);
};

#endif
