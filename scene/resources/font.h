/*************************************************************************/
/*  font.h                                                               */
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

#ifndef FONT_H
#define FONT_H

#include "core/reference.h"
#include "core/resource.h"

class FontData : public Resource {
	GDCLASS(FontData, Resource);

protected:
	static void _bind_methods();

public:
	virtual Error load_from_file(String p_path) = 0;
	virtual Error load_from_memory(const uint8_t *p_buffer, int p_size) = 0;
};

/*************************************************************************/

class Font : public Resource {
	GDCLASS(Font, Resource);

public:
	enum SpacingType {
		SPACING_TOP,
		SPACING_BOTTOM,
		SPACING_GLYPH,
		SPACING_SPACE_CHAR,
	};

protected:
	int spacing_top;
	int spacing_bottom;
	int spacing_glyph;
	int spacing_space_char;

	bool use_mipmaps;
	bool use_filter;

	static void _bind_methods();

public:
	virtual Ref<FontData> get_data() const { return Ref<FontData>(); };
	virtual void set_data(const Ref<FontData> &p_data) = 0;

	virtual RID get_rid() const { return RID(); };

	virtual Size2 get_char_size(char32_t p_char) const = 0;
	virtual Size2 get_string_size(const String &p_string) const = 0;

	virtual bool get_use_mipmaps() const = 0;
	virtual void set_use_mipmaps(bool p_enable) = 0;

	virtual bool get_use_filter() const = 0;
	virtual void set_use_filter(bool p_enable) = 0;

	virtual int get_spacing(int p_type) const;
	virtual void set_spacing(int p_type, int p_value);

	virtual float get_height() const = 0;

	virtual float get_ascent() const = 0;
	virtual float get_descent() const = 0;

	virtual bool is_distance_field_hint() const { return false; };

	Font();
	~Font();
};

VARIANT_ENUM_CAST(Font::SpacingType);

#endif
