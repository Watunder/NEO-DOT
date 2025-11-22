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

#include "servers/font_server.h"

FontHandle::CacheKey FontHandle::get_cache_key() const {
	return cache_key;
};

float FontHandle::get_height() const {
	return ascent + descent;
}

float FontHandle::get_ascent() const {
	return ascent;
}

float FontHandle::get_descent() const {
	return descent;
}

float FontHandle::get_oversampling() const {
	return oversampling;
}

FontHandle::FontHandle() {
	ascent = 0;
	descent = 1;
	oversampling = 1;
}

/*************************************************************************/

void FontData::_bind_methods() {
}

/*************************************************************************/

Ref<FontHandle> Font::get_handle() const {
	return handle;
}

Ref<FontData> Font::get_data() const {
	return data;
}

void Font::set_data(const Ref<FontData> &p_data) {
	data = p_data;

	emit_changed();
	_change_notify();
}

Size2 Font::get_char_size(char32_t p_char) const {
	if (handle.is_valid()) {
		return Size2(handle->get_height() / 2, handle->get_height() / 2);
	} else {
		return Size2(1, 1);
	}
}

Size2 Font::get_string_size(const String &p_string) const {
	if (p_string.length() == 0) {
		return Size2(0, get_height());
	}

	float width = 0;

	const char32_t *sptr = &p_string[0];
	for (int i = 0; i < p_string.length(); i++) {
		width += get_char_size(sptr[i]).width;
	}

	return Size2(width, get_height());
}

bool Font::get_use_mipmaps() const {
	return use_mipmaps;
}

void Font::set_use_mipmaps(bool p_enable) {
	if (use_mipmaps == p_enable)
		return;
	use_mipmaps = p_enable;

	if (handle.is_valid()) {
		handle->cache_key.font_use_mipmaps = use_mipmaps;
	}

	emit_changed();
	_change_notify();
}

bool Font::get_use_filter() const {
	return use_filter;
}

void Font::set_use_filter(bool p_enable) {
	if (use_filter == p_enable)
		return;
	use_filter = p_enable;

	if (handle.is_valid()) {
		handle->cache_key.font_use_filter = use_filter;
	}

	emit_changed();
	_change_notify();
}

int Font::get_spacing(int p_type) const {
	if (p_type == SPACING_TOP) {
		return spacing_top;
	} else if (p_type == SPACING_BOTTOM) {
		return spacing_bottom;
	} else if (p_type == SPACING_CHAR) {
		return spacing_char;
	} else if (p_type == SPACING_SPACE) {
		return spacing_space;
	}

	return 0;
}

void Font::set_spacing(int p_type, int p_value) {
	if (p_type == SPACING_TOP) {
		spacing_top = p_value;
	} else if (p_type == SPACING_BOTTOM) {
		spacing_bottom = p_value;
	} else if (p_type == SPACING_CHAR) {
		spacing_char = p_value;
	} else if (p_type == SPACING_SPACE) {
		spacing_space = p_value;
	}

	emit_changed();
	_change_notify();
}

float Font::get_height() const {
	if (!handle.is_valid())
		return 1;

	return handle->get_height() + spacing_top + spacing_bottom;
};

float Font::get_ascent() const {
	if (!handle.is_valid())
		return 0;

	return handle->get_ascent() + spacing_top;
};

float Font::get_descent() const {
	if (!handle.is_valid())
		return 1;

	return handle->get_descent() + spacing_bottom;
};

bool Font::is_distance_field_hint() const {
	return false;
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
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_char"), "set_spacing", "get_spacing", SPACING_CHAR);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_space"), "set_spacing", "get_spacing", SPACING_SPACE);

	BIND_ENUM_CONSTANT(SPACING_TOP);
	BIND_ENUM_CONSTANT(SPACING_BOTTOM);
	BIND_ENUM_CONSTANT(SPACING_CHAR);
	BIND_ENUM_CONSTANT(SPACING_SPACE);
}

Font::Font() {
	use_filter = true;
	use_mipmaps = true;

	spacing_top = 0;
	spacing_bottom = 0;
	spacing_char = 0;
	spacing_space = 0;
}
