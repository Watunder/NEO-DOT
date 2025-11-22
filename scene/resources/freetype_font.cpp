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

FT_Face FreeTypeFontHandle::get_face() const {
	return face;
}

int FreeTypeFontHandle::get_face_count() const {
	return (face && face->num_faces || 0);
}

Error FreeTypeFontHandle::update_size(int p_size) {
	ERR_FAIL_COND_V(!(face && face->size), ERR_UNAVAILABLE);

	int error = FT_Err_Ok;

	int color_font_scale = 1;
	if (FT_HAS_COLOR(face) && face->num_fixed_sizes > 0) {
		int best_match = 0;
		int diff = ABS(p_size - ((int64_t)face->available_sizes[0].width));
		color_font_scale = float(p_size * oversampling) / face->available_sizes[0].width;
		for (int i = 1; i < face->num_fixed_sizes; i++) {
			int ndiff = ABS(p_size - ((int64_t)face->available_sizes[i].width));
			if (ndiff < diff) {
				best_match = i;
				diff = ndiff;
				color_font_scale = float(p_size * oversampling) / face->available_sizes[i].width;
			}
		}
		error = FT_Select_Size(face, best_match);
	} else {
		error = FT_Set_Pixel_Sizes(face, 0, p_size * oversampling);
	}

	ERR_FAIL_COND_V(error != FT_Err_Ok, ERR_UNCONFIGURED);

	ascent = (face->size->metrics.ascender / 64.0) / oversampling * color_font_scale;
	descent = (-face->size->metrics.descender / 64.0) / oversampling * color_font_scale;

	cache_key.font_size = p_size;

	return OK;
}

void FreeTypeFontHandle::update_oversampling(float p_ratio) {
	if (oversampling == p_ratio)
		return;
	oversampling = p_ratio;

	if (update_size(cache_key.font_size) == OK) {
		FontServer::get_singleton()->clear_glyph_cache(cache_key);
	}
}

FreeTypeFontHandle::FreeTypeFontHandle() {
}

FreeTypeFontHandle::~FreeTypeFontHandle() {
	if (face) {
		FT_Done_Face(face);
	}
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

	ClassDB::bind_method(D_METHOD("set_antialiasing", "antialiasing"), &FreeTypeFont::set_antialiasing);
	ClassDB::bind_method(D_METHOD("get_antialiasing"), &FreeTypeFont::get_antialiasing);

	ADD_GROUP("Font", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data", PROPERTY_HINT_RESOURCE_TYPE, "FreeTypeFontData"), "set_data", "get_data");

	ADD_GROUP("Settings", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "size", PROPERTY_HINT_RANGE, "1,1024,1"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "force_autohinter"), "set_force_autohinter", "is_force_autohinter");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hinting", PROPERTY_HINT_ENUM, "None,Light,Normal"), "set_hinting", "get_hinting");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "antialiasing", PROPERTY_HINT_ENUM, "None,Normal"), "set_antialiasing", "get_antialiasing");

	BIND_ENUM_CONSTANT(ANTIALIASING_NONE);
	BIND_ENUM_CONSTANT(ANTIALIASING_NORMAL);

	BIND_ENUM_CONSTANT(HINTING_NONE);
	BIND_ENUM_CONSTANT(HINTING_LIGHT);
	BIND_ENUM_CONSTANT(HINTING_NORMAL);
}

void FreeTypeFont::set_data(const Ref<FontData> &p_data) {
	data = p_data;

	Ref<FreeTypeFontHandle> font_handle;
	font_handle.instance();

	Ref<FreeTypeFontData> font_data = data;
	if (font_data.is_valid()) {
		PoolVector<uint8_t> buffer = font_data->get_buffer();
		if (buffer.size()) {
			font_handle->buffer = buffer;
			font_handle->cache_key.font_hash = XXH32(font_handle->buffer.read().ptr(), font_handle->buffer.size(), 0);

			FT_New_Memory_Face(library, font_handle->buffer.read().ptr(), font_handle->buffer.size(), 0, &font_handle->face);
			font_handle->update_size(size);
		}
	}

	handle = font_handle;

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

	Ref<FreeTypeFontHandle> font_handle = handle;
	if (font_handle.is_valid()) {
		font_handle->update_size(size);
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
}

FreeTypeFont::Hinting FreeTypeFont::get_hinting() const {
	return hinting;
}

void FreeTypeFont::set_hinting(Hinting p_hinting) {
	if (hinting == p_hinting)
		return;
	hinting = p_hinting;
}

FreeTypeFont::Antialiasing FreeTypeFont::get_antialiasing() const {
	return antialiasing;
}

void FreeTypeFont::set_antialiasing(Antialiasing p_antialiasing) {
	if (antialiasing == p_antialiasing)
		return;
	antialiasing = p_antialiasing;
}

Mutex FreeTypeFont::font_mutex;

SelfList<FreeTypeFont>::List *FreeTypeFont::fonts = NULL;

FT_Library FreeTypeFont::library = NULL;

FreeTypeFont::FreeTypeFont() :
		font_list(this) {
	font_mutex.lock();
	fonts->add(&font_list);
	font_mutex.unlock();

	size = 1;

	antialiasing = FreeTypeFont::ANTIALIASING_NORMAL;
	hinting = FreeTypeFont::HINTING_NORMAL;
	force_autohinter = false;
}

FreeTypeFont::~FreeTypeFont() {
	font_mutex.lock();
	fonts->remove(&font_list);
	font_mutex.unlock();
}

void FreeTypeFont::initialize_fonts() {
	fonts = memnew(SelfList<FreeTypeFont>::List());

	FT_Init_FreeType(&library);
}

void FreeTypeFont::finish_fonts() {
	memdelete(fonts);
	fonts = NULL;
}

void FreeTypeFont::update_oversampling(float p_ratio) {
	Vector<Ref<FreeTypeFont>> changed;

	font_mutex.lock();

	SelfList<FreeTypeFont> *E = fonts->first();
	while (E) {
		if (E->self()->handle.is_valid()) {
			Ref<FreeTypeFontHandle> font_hanlde = E->self()->handle;
			if (font_hanlde.is_valid()) {
				font_hanlde->update_oversampling(p_ratio);
			}

			changed.push_back(Ref<FreeTypeFont>(E->self()));
		}

		E = E->next();
	}

	font_mutex.unlock();

	for (int i = 0; i < changed.size(); i++) {
		changed.write[i]->emit_changed();
	}
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
