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
#include "core/os/os.h"
#include "servers/font_server.h"

#include "servers/font/builtin_fonts.gen.h"

Ref<FreeTypeFontData> FreeTypeFontData::default_font_data;

Ref<FreeTypeFontData> FreeTypeFontData::get_default() {
	if (!default_font_data.is_valid()) {
		default_font_data.instance();
		default_font_data->load_from_memory(_font_NotoSansUI_Regular, _font_NotoSansUI_Regular_size);
	}

	return default_font_data;
}

void FreeTypeFontData::clear_default() {
	default_font_data.unref();
}

Ref<Resource> FreeTypeFontData::duplicate(bool p_subresources) const {
	Ref<FreeTypeFontData> copy;
	copy.instance();
	copy->_copy_internals_from(*this);
	return copy;
}

Error FreeTypeFontData::load_from_file(String p_path) {
	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);

	int size = f->get_len();
	buffer.resize(size);
	f->get_buffer(buffer.write().ptr(), size);

	f->close();
	memdelete(f);

	emit_changed();
	return OK;
}

Error FreeTypeFontData::load_from_memory(const uint8_t *p_buffer, int p_size) {
	FileAccessMemory *f = memnew(FileAccessMemory);

	f->open_custom(p_buffer, p_size);

	buffer.resize(p_size);
	f->get_buffer(buffer.write().ptr(), p_size);

	f->close();
	memdelete(f);

	emit_changed();
	return OK;
}

bool FreeTypeFontData::empty() const {
	return buffer.size() == 0;
}

PoolVector<uint8_t> FreeTypeFontData::get_buffer() const {
	return buffer;
}

/*************************************************************************/

void FreeTypeFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_data_changed"), &FreeTypeFont::_data_changed);

	ClassDB::bind_method(D_METHOD("set_data", "data"), &FreeTypeFont::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &FreeTypeFont::get_data);

	ClassDB::bind_method(D_METHOD("set_size", "size"), &FreeTypeFont::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &FreeTypeFont::get_size);

	ClassDB::bind_method(D_METHOD("set_force_autohinter", "force_autohinter"), &FreeTypeFont::set_force_autohinter);
	ClassDB::bind_method(D_METHOD("is_force_autohinter"), &FreeTypeFont::is_force_autohinter);

	ClassDB::bind_method(D_METHOD("set_hinting", "mode"), &FreeTypeFont::set_hinting);
	ClassDB::bind_method(D_METHOD("get_hinting"), &FreeTypeFont::get_hinting);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "FreeTypeFontData"), "set_data", "get_data");

	ADD_GROUP("Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "1,1000,1"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_autohinter"), "set_force_autohinter", "is_force_autohinter");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hinting", PROPERTY_HINT_ENUM, "None,Light,Normal"), "set_hinting", "get_hinting");

	BIND_ENUM_CONSTANT(HINTING_NONE);
	BIND_ENUM_CONSTANT(HINTING_LIGHT);
	BIND_ENUM_CONSTANT(HINTING_NORMAL);

	ADD_GROUP("Extra Spacing", "extra_spacing");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_top"), "set_spacing", "get_spacing", FontServer::SPACING_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_bottom"), "set_spacing", "get_spacing", FontServer::SPACING_BOTTOM);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_glyph"), "set_spacing", "get_spacing", FontServer::SPACING_GLYPH);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "extra_spacing_space_char"), "set_spacing", "get_spacing", FontServer::SPACING_SPACE_CHAR);
}

void FreeTypeFont::_data_changed() {
	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_data(font, data->get_buffer());
	float oversampling = FontServer::get_singleton()->font_get_oversampling(font);
	FontServer::get_singleton()->font_update_metrics(font, oversampling);

	emit_changed();
	_change_notify();
}

Ref<FreeTypeFontData> FreeTypeFont::get_data() const {
	return data;
}

void FreeTypeFont::set_data(const Ref<FreeTypeFontData> &p_data) {
	ERR_FAIL_COND(!p_data.is_valid() || p_data->empty());

	if (data == p_data)
		return;

	if (data.is_valid()) {
		data->disconnect("changed", this, "_data_changed");
	}

	data = p_data;

	if (data.is_valid()) {
		data->connect("changed", this, "_data_changed");
	}

	_data_changed();
}

RID FreeTypeFont::get_rid() const {
	return font;
}

int FreeTypeFont::get_size() const {
	return size;
}

void FreeTypeFont::set_size(int p_size) {
	if (size == p_size)
		return;
	size = p_size;

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_size(font, size);
	float oversampling = FontServer::get_singleton()->font_get_oversampling(font);
	FontServer::get_singleton()->font_update_metrics(font, oversampling);

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
};

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
	if (use_mipmaps == p_enable)
		return;
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
	if (use_filter == p_enable)
		return;
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

bool FreeTypeFont::is_force_autohinter() {
	return force_autohinter;
}

void FreeTypeFont::set_force_autohinter(bool p_force_autohinter) {
	if (force_autohinter == p_force_autohinter)
		return;
	force_autohinter = p_force_autohinter;

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_force_autohinter(font, force_autohinter);

	emit_changed();
	_change_notify();
}

FreeTypeFont::Hinting FreeTypeFont::get_hinting() const {
	return hinting;
}

void FreeTypeFont::set_hinting(Hinting p_hinting) {
	if (hinting == p_hinting)
		return;
	hinting = p_hinting;

	FontServer::get_singleton()->font_clear_caches(font);
	FontServer::get_singleton()->font_set_hinting(font, hinting);

	emit_changed();
	_change_notify();
}

FreeTypeFont::FreeTypeFont() {
	size = 16;
	hinting = FreeTypeFont::HINTING_NORMAL;
	force_autohinter = false;

	font = FontServer::get_singleton()->font_create();

	set_data(FreeTypeFontData::get_default());
}

FreeTypeFont::~FreeTypeFont() {
	FontServer::get_singleton()->font_free(font);
}

/*************************************************************************/

Ref<Resource> ResourceFormatLoaderFreeTypeFont::load(const String &p_path, const String &p_original_path, Error *r_error) {
	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;

	Ref<FreeTypeFontData> font_data;
	font_data.instance();
	font_data->load_from_file(p_path);

	if (r_error)
		*r_error = OK;

	return font_data;
}

void ResourceFormatLoaderFreeTypeFont::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("ttf");
	p_extensions->push_back("otf");
}

bool ResourceFormatLoaderFreeTypeFont::handles_type(const String &p_type) const {
	return (p_type == "FreeTypeFontData");
}

String ResourceFormatLoaderFreeTypeFont::get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "ttf" || el == "otf")
		return "FreeTypeFontData";
	return "";
}
