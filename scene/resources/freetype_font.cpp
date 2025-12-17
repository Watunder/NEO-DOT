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

#include "configs/modules_enabled.gen.h"
#ifdef MODULE_FREETYPE_ENABLED

#include "freetype_font.h"

#include "core/io/file_access_memory.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "servers/font_server.h"

#include "thirdparty/zstd/common/xxhash.h"

FT_Face FreeTypeFontHandle::get_ft_face() const {
	return ft_face;
}

FT_Size FreeTypeFontHandle::get_ft_size() const {
	return ft_size;
}

Error FreeTypeFontHandle::update_cache(int p_size, float p_oversampling) {
	if (!cache_key.font_hash) {
		return ERR_UNCONFIGURED;
	}

	if (oversampling != p_oversampling) {
		oversampling = p_oversampling;
	}

	if (!ft_face) {
		ft_face = FontServer::get_singleton()->lookup_face(cache_key.font_hash);
	}

	if (ft_face) {
		ft_size = FontServer::get_singleton()->lookup_size(cache_key.font_hash, p_size, oversampling);
		ascent = (ft_size->metrics.ascender / 64.0) / oversampling;
		descent = (-ft_size->metrics.descender / 64.0) / oversampling;
	}

	ERR_FAIL_COND_V(!(ft_face && ft_size), ERR_UNCONFIGURED);

	return OK;
}

/*************************************************************************/

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

	return OK;
}

Error FreeTypeFontData::load_from_memory(const uint8_t *p_buffer, int p_size) {
	FileAccessMemory *f = memnew(FileAccessMemory);

	f->open_custom(p_buffer, p_size);

	buffer.resize(p_size);
	f->get_buffer(buffer.write().ptr(), p_size);

	f->close();
	memdelete(f);

	return OK;
}

PoolVector<uint8_t> FreeTypeFontData::get_buffer() const {
	return buffer;
}

/*************************************************************************/

void FreeTypeFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_size", "size"), &FreeTypeFont::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &FreeTypeFont::get_size);

	ClassDB::bind_method(D_METHOD("set_force_autohinter", "force_autohinter"), &FreeTypeFont::set_force_autohinter);
	ClassDB::bind_method(D_METHOD("is_force_autohinter"), &FreeTypeFont::is_force_autohinter);

	ClassDB::bind_method(D_METHOD("set_hinting", "mode"), &FreeTypeFont::set_hinting);
	ClassDB::bind_method(D_METHOD("get_hinting"), &FreeTypeFont::get_hinting);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "FreeTypeFontData"), "set_data", "get_data");

	ADD_GROUP("Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "1,1024,1"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_autohinter"), "set_force_autohinter", "is_force_autohinter");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hinting", PROPERTY_HINT_ENUM, "None,Light,Normal"), "set_hinting", "get_hinting");

	BIND_ENUM_CONSTANT(HINTING_NONE);
	BIND_ENUM_CONSTANT(HINTING_LIGHT);
	BIND_ENUM_CONSTANT(HINTING_NORMAL);
}

Ref<FontData> FreeTypeFont::get_data() const {
	return data;
}

void FreeTypeFont::set_data(const Ref<FontData> &p_data) {
	data = p_data;

	Ref<FreeTypeFontHandle> font_handle;
	font_handle.instance();

	font_handle->cache_key.font_size = size;
	font_handle->cache_key.font_use_mipmaps = use_mipmaps;
	font_handle->cache_key.font_use_filter = use_filter;
	font_handle->cache_key.font_force_autohinter = force_autohinter;
	font_handle->cache_key.font_hinting = hinting;

	if (data.is_valid()) {
		PoolVector<uint8_t> buffer = data->get_buffer();
		if (buffer.size()) {
			font_handle->cache_key.font_hash = XXH32(buffer.read().ptr(), buffer.size(), 0);
			FontServer::get_singleton()->store_font_data(font_handle->cache_key.font_hash, data);

			font_handle->update_cache(size);
			handle = font_handle;
		}
	} else if (handle.is_valid()) {
		handle.unref();
	}

	emit_changed();
	_change_notify();
}

Ref<FontHandle> FreeTypeFont::get_handle() const {
	return handle;
}

int FreeTypeFont::get_size() const {
	return size;
}

void FreeTypeFont::set_size(int p_size) {
	if (size == p_size)
		return;
	size = p_size;

	if (handle.is_valid()) {
		handle->cache_key.font_size = p_size;
		handle->update_cache(size);
	}

	emit_changed();
	_change_notify();
}

float FreeTypeFont::get_height() const {
	if (!handle.is_valid())
		return 1;

	return handle->get_height() + spacing_top + spacing_bottom;
};

float FreeTypeFont::get_ascent() const {
	if (!handle.is_valid())
		return 0;

	return handle->get_ascent() + spacing_top;
};

float FreeTypeFont::get_descent() const {
	if (!handle.is_valid())
		return 1;

	return handle->get_descent() + spacing_bottom;
};

Size2 FreeTypeFont::get_char_size(char32_t p_char) const {
	if (handle.is_valid()) {
		return FontServer::get_singleton()->get_char_size(Ref<Font>(this), p_char);
	} else {
		return Size2(1, 1);
	}
}

Size2 FreeTypeFont::get_string_size(const String &p_string) const {
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

bool FreeTypeFont::get_use_mipmaps() const {
	return use_mipmaps;
}

void FreeTypeFont::set_use_mipmaps(bool p_enable) {
	if (use_mipmaps == p_enable)
		return;
	use_mipmaps = p_enable;

	if (handle.is_valid()) {
		handle->cache_key.font_use_mipmaps = use_mipmaps;
	}

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

	if (handle.is_valid()) {
		handle->cache_key.font_use_filter = use_filter;
	}

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

	if (handle.is_valid()) {
		handle->cache_key.font_force_autohinter = force_autohinter ? 1 : 0;
	}

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

	if (handle.is_valid()) {
		handle->cache_key.font_hinting = hinting;
	}

	emit_changed();
	_change_notify();
}

FreeTypeFont::FreeTypeFont() {
	size = 16;
	hinting = FreeTypeFont::HINTING_NORMAL;
	force_autohinter = false;
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

#endif
