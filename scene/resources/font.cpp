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

void Font::_bind_methods() {
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
}

Font::Font() {
	use_filter = true;
	use_mipmaps = true;
}

Font::~Font() {
}
