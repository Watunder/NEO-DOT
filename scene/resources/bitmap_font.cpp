/*************************************************************************/
/*  bitmap_font.cpp                                                      */
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

#include "bitmap_font.h"

#include "servers/font_server.h"
#include "servers/text/text_helper.h"

void BitmapFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load", "path"), &BitmapFont::load);
	ClassDB::bind_method(D_METHOD("get_load_path"), &BitmapFont::get_load_path);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "load_path", PROPERTY_HINT_FILE, "*.fnt"), "load", "get_load_path");
}

void BitmapFont::reload_from_file() {
	load(path_to_file);
}

RID BitmapFont::get_rid() const {
	return font;
}

Error BitmapFont::load(String p_path) {
	Error err = OK;

	if (!FontServer::get_singleton()->font_set_path(font, p_path)) {
		return ERR_FILE_CANT_OPEN;
	}
	path_to_file = p_path;

	emit_changed();
	_change_notify();

	return err;
}

String BitmapFont::get_load_path() const {
	return path_to_file;
}

Size2 BitmapFont::get_char_size(char32_t p_char) const {
	return TextHelper::get_char_size(font, p_char);
}

Size2 BitmapFont::get_string_size(const String &p_string) const {
	return TextHelper::get_string_size(font, p_string);
}

float BitmapFont::get_height() const {
	return get_ascent() + get_descent();
}

float BitmapFont::get_ascent() const {
	return FontServer::get_singleton()->font_get_ascent(font);
}

float BitmapFont::get_descent() const {
	return FontServer::get_singleton()->font_get_descent(font);
}

bool BitmapFont::get_use_mipmaps() const {
	return use_mipmaps;
}

void BitmapFont::set_use_mipmaps(bool p_enable) {
	if (use_mipmaps == p_enable) {
		return;
	}
	use_mipmaps = p_enable;

	FontServer::get_singleton()->font_set_use_mipmaps(font, use_mipmaps);

	emit_changed();
	_change_notify();
}

bool BitmapFont::get_use_filter() const {
	return use_filter;
}

void BitmapFont::set_use_filter(bool p_enable) {
	if (use_filter == p_enable) {
		return;
	}
	use_filter = p_enable;

	FontServer::get_singleton()->font_set_use_filter(font, use_filter);

	emit_changed();
	_change_notify();
}

int BitmapFont::get_spacing(int p_type) const {
	return FontServer::get_singleton()->font_get_spacing(font, (FontServer::SpacingType)p_type);
}

void BitmapFont::set_spacing(int p_type, int p_value) {
	FontServer::get_singleton()->font_set_spacing(font, (FontServer::SpacingType)p_type, p_value);

	emit_changed();
	_change_notify();
}

BitmapFont::BitmapFont() {
	use_mipmaps = true;
	use_filter = true;

	font = FontServer::get_singleton()->font_create(16, use_mipmaps, use_filter);
}

BitmapFont::~BitmapFont() {
	FontServer::get_singleton()->font_free(font);
}

/*************************************************************************/

Ref<Resource> ResourceFormatLoaderBitmapFont::load(const String &p_path, const String &p_original_path, Error *r_error) {
	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;

	Ref<BitmapFont> font;
	font.instance();
	Error err = font->load(p_path);
	ERR_FAIL_COND_V(err != OK, Ref<Resource>());

	if (r_error)
		*r_error = OK;

	return font;
}

void ResourceFormatLoaderBitmapFont::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("fnt");
}

bool ResourceFormatLoaderBitmapFont::handles_type(const String &p_type) const {
	return ClassDB::is_parent_class(p_type, "Font");
}

String ResourceFormatLoaderBitmapFont::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "fnt")
		return "BitmapFont";
	return "";
}
