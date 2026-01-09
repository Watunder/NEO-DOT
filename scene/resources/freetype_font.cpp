/*************************************************************************/
/*  freetype_font.cpp                                                    */
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

#include "freetype_font.h"

#include "core/io/file_access_memory.h"
#include "core/os/file_access.h"
#include "servers/font_server.h"

void FreeTypeFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "path"), &FreeTypeFont::load);
	ClassDB::bind_method(D_METHOD("get_load_path"), &FreeTypeFont::get_load_path);

	ClassDB::bind_method(D_METHOD("set_face_index", "index"), &FreeTypeFont::set_face_index);
	ClassDB::bind_method(D_METHOD("get_face_index"), &FreeTypeFont::get_face_index);

	ClassDB::bind_method(D_METHOD("set_face_size", "size"), &FreeTypeFont::set_face_size);
	ClassDB::bind_method(D_METHOD("get_face_size"), &FreeTypeFont::get_face_size);

	ClassDB::bind_method(D_METHOD("set_hinting", "mode"), &FreeTypeFont::set_hinting);
	ClassDB::bind_method(D_METHOD("get_hinting"), &FreeTypeFont::get_hinting);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_path", PROPERTY_HINT_FILE, "*.ttf,*.ttc,*.otf,*.otc"), "load", "get_load_path");

	ADD_GROUP("Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "face_index", PROPERTY_HINT_RANGE, "0,255,1"), "set_face_index", "get_face_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "face_size", PROPERTY_HINT_RANGE, "1,1024,1"), "set_face_size", "get_face_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hinting", PROPERTY_HINT_ENUM, "None,Auto,Light,Normal"), "set_hinting", "get_hinting");

	BIND_ENUM_CONSTANT(HINTING_NONE);
	BIND_ENUM_CONSTANT(HINTING_AUTO);
	BIND_ENUM_CONSTANT(HINTING_LIGHT);
	BIND_ENUM_CONSTANT(HINTING_NORMAL);

	ADD_GROUP("Extra Spacing", "extra_spacing");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_top"), "set_spacing", "get_spacing", FontServer::SPACING_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_bottom"), "set_spacing", "get_spacing", FontServer::SPACING_BOTTOM);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_glyph"), "set_spacing", "get_spacing", FontServer::SPACING_GLYPH);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_space_char"), "set_spacing", "get_spacing", FontServer::SPACING_SPACE_CHAR);
}

bool FreeTypeFont::_update_data(const PoolVector<uint8_t> &p_data) {
	FontServer::get_singleton()->font_clear_caches(font);
	if (!FontServer::get_singleton()->font_update_data(font, p_data)) {
		return false;
	}
	if (!FontServer::get_singleton()->font_update_metrics(font)) {
		return false;
	}

	emit_changed();
	_change_notify();

	return true;
}

void FreeTypeFont::reload_from_file() {
	load(path_to_file);
}

RID FreeTypeFont::get_rid() const {
	return font;
}

void FreeTypeFont::set_data(const PoolVector<uint8_t> &p_data) {
	_update_data(p_data);
}

PoolVector<uint8_t> FreeTypeFont::get_data() const {
	return FontServer::get_singleton()->font_get_data(font);
}

Error FreeTypeFont::load(String p_path) {
	Error err = OK;

	PoolVector<uint8_t> data;

	if (!p_path.empty()) {
		FileAccess *f = FileAccess::open(p_path, FileAccess::READ, &err);
		ERR_FAIL_COND_V_MSG(err, err, "Cannot open file '" + p_path + "'.");

		int len = f->get_len();
		data.resize(len);
		PoolVector<uint8_t>::Write w = data.write();
		int r = f->get_buffer(w.ptr(), len);
		f->close();
		memdelete(f);
		ERR_FAIL_COND_V(r != len, ERR_CANT_OPEN);
	}

	if (_update_data(data)) {
		path_to_file = p_path;
	}

	return err;
}

String FreeTypeFont::get_load_path() const {
	return path_to_file;
}

int FreeTypeFont::get_face_index() const {
	return face_index;
}

void FreeTypeFont::set_face_index(int p_index) {
	if (face_index == p_index) {
		return;
	}

	FontServer::get_singleton()->font_clear_caches(font);
	if (!FontServer::get_singleton()->font_update_index(font, p_index)) {
		return;
	}
	if (!FontServer::get_singleton()->font_update_metrics(font)) {
		return;
	}

	face_index = p_index;

	emit_changed();
	_change_notify();
};

int FreeTypeFont::get_face_size() const {
	return face_size;
}

void FreeTypeFont::set_face_size(int p_size) {
	if (face_size == p_size) {
		return;
	}

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_size(font, p_size);
	if (!FontServer::get_singleton()->font_update_metrics(font)) {
		return;
	}

	face_size = p_size;

	emit_changed();
	_change_notify();
}

float FreeTypeFont::get_height() const {
	return get_ascent() + get_descent();
};

float FreeTypeFont::get_ascent() const {
	return FontServer::get_singleton()->font_get_ascent(font);
};

float FreeTypeFont::get_descent() const {
	return FontServer::get_singleton()->font_get_descent(font);
}

Size2 FreeTypeFont::get_char_size(char32_t p_char) const {
	return FontServer::get_singleton()->font_get_char_size(font, p_char);
}

Size2 FreeTypeFont::get_string_size(const String &p_string) const {
	return FontServer::get_singleton()->font_get_string_size(font, p_string);
}

bool FreeTypeFont::get_use_mipmaps() const {
	return use_mipmaps;
}

void FreeTypeFont::set_use_mipmaps(bool p_enable) {
	if (use_mipmaps == p_enable) {
		return;
	}
	use_mipmaps = p_enable;

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_use_mipmaps(font, use_mipmaps);

	emit_changed();
	_change_notify();
}

bool FreeTypeFont::get_use_filter() const {
	return use_filter;
}

void FreeTypeFont::set_use_filter(bool p_enable) {
	if (use_filter == p_enable) {
		return;
	}
	use_filter = p_enable;

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_use_filter(font, use_filter);

	emit_changed();
	_change_notify();
}

int FreeTypeFont::get_spacing(int p_type) const {
	return FontServer::get_singleton()->font_get_spacing(font, (FontServer::SpacingType)p_type);
}

void FreeTypeFont::set_spacing(int p_type, int p_value) {
	FontServer::get_singleton()->font_set_spacing(font, (FontServer::SpacingType)p_type, p_value);

	emit_changed();
	_change_notify();
}

FreeTypeFont::Hinting FreeTypeFont::get_hinting() const {
	return hinting;
}

void FreeTypeFont::set_hinting(Hinting p_hinting) {
	if (hinting == p_hinting) {
		return;
	}
	hinting = p_hinting;

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_custom_flags(font, hinting);

	emit_changed();
	_change_notify();
}

FreeTypeFont::FreeTypeFont() {
	use_mipmaps = true;
	use_filter = true;
	face_index = 0;
	face_size = 16;
	hinting = FreeTypeFont::HINTING_NORMAL;

	font = FontServer::get_singleton()->font_create(face_size, hinting);
	FontServer::get_singleton()->font_update_metrics(font);
}

FreeTypeFont::~FreeTypeFont() {
	FontServer::get_singleton()->font_free(font);
}

/*************************************************************************/

Ref<Resource> ResourceFormatLoaderFreeTypeFont::load(const String &p_path, const String &p_original_path, Error *r_error) {
	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;

	Ref<FreeTypeFont> font;
	font.instance();
	Error err = font->load(p_path);
	ERR_FAIL_COND_V(err != OK, Ref<Resource>());

	if (r_error)
		*r_error = OK;

	return font;
}

void ResourceFormatLoaderFreeTypeFont::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ttf");
	p_extensions->push_back("ttc");
	p_extensions->push_back("otf");
	p_extensions->push_back("otc");
}

bool ResourceFormatLoaderFreeTypeFont::handles_type(const String &p_type) const {
	return (p_type == "FreeTypeFont");
}

String ResourceFormatLoaderFreeTypeFont::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "ttf" || el == "ttc" || el == "otf" || el == "otc")
		return "FreeTypeFont";
	return "";
}
