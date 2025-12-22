/*************************************************************************/
/*  font.cpp                                                             */
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

#include "font.h"

#include "core/method_bind_ext.gen.inc"

void FontData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load"), &FontData::load_from_file);
}

/*************************************************************************/

int Font::get_spacing(int p_type) const {
	if (p_type == SPACING_TOP) {
		return spacing_top;
	} else if (p_type == SPACING_BOTTOM) {
		return spacing_bottom;
	} else if (p_type == SPACING_GLYPH) {
		return spacing_glyph;
	} else if (p_type == SPACING_SPACE_CHAR) {
		return spacing_space_char;
	}

	return 0;
}

void Font::set_spacing(int p_type, int p_value) {
	if (p_type == SPACING_TOP) {
		spacing_top = p_value;
	} else if (p_type == SPACING_BOTTOM) {
		spacing_bottom = p_value;
	} else if (p_type == SPACING_GLYPH) {
		spacing_glyph = p_value;
	} else if (p_type == SPACING_SPACE_CHAR) {
		spacing_space_char = p_value;
	}

	emit_changed();
	_change_notify();
}

void Font::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_data"), &Font::get_data);
	ClassDB::bind_method(D_METHOD("set_data"), &Font::set_data);

	ClassDB::bind_method(D_METHOD("get_height"), &Font::get_height);

	ClassDB::bind_method(D_METHOD("get_ascent"), &Font::get_ascent);
	ClassDB::bind_method(D_METHOD("get_descent"), &Font::get_descent);

	ClassDB::bind_method(D_METHOD("is_distance_field_hint"), &Font::is_distance_field_hint);

	ClassDB::bind_method(D_METHOD("get_char_size", "char"), &Font::get_char_size);
	ClassDB::bind_method(D_METHOD("get_string_size", "string"), &Font::get_string_size);

	ClassDB::bind_method(D_METHOD("set_use_mipmaps", "enable"), &Font::set_use_mipmaps);
	ClassDB::bind_method(D_METHOD("get_use_mipmaps"), &Font::get_use_mipmaps);

	ClassDB::bind_method(D_METHOD("set_use_filter", "enable"), &Font::set_use_filter);
	ClassDB::bind_method(D_METHOD("get_use_filter"), &Font::get_use_filter);

	ClassDB::bind_method(D_METHOD("set_spacing", "type", "value"), &Font::set_spacing);
	ClassDB::bind_method(D_METHOD("get_spacing", "type"), &Font::get_spacing);

	ADD_GROUP("Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_mipmaps"), "set_use_mipmaps", "get_use_mipmaps");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_filter"), "set_use_filter", "get_use_filter");

	ADD_GROUP("Extra Spacing", "extra_spacing");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_top"), "set_spacing", "get_spacing", SPACING_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_bottom"), "set_spacing", "get_spacing", SPACING_BOTTOM);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_glyph"), "set_spacing", "get_spacing", SPACING_GLYPH);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_space_char"), "set_spacing", "get_spacing", SPACING_SPACE_CHAR);

	BIND_ENUM_CONSTANT(SPACING_TOP);
	BIND_ENUM_CONSTANT(SPACING_BOTTOM);
	BIND_ENUM_CONSTANT(SPACING_GLYPH);
	BIND_ENUM_CONSTANT(SPACING_SPACE_CHAR);
}

Font::Font() {
	use_filter = true;
	use_mipmaps = true;

	spacing_top = 0;
	spacing_bottom = 0;
	spacing_glyph = 0;
	spacing_space_char = 0;
}

Font::~Font() {
}
