/*************************************************************************/
/*  freetype_font.h                                                      */
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

#ifndef FREETYPE_FONT_H
#define FREETYPE_FONT_H

#include "configs/modules_enabled.gen.h"
#ifdef MODULE_FREETYPE_ENABLED

#include "core/io/resource_loader.h"
#include "core/os/mutex.h"
#include "core/os/thread_safe.h"
#include "scene/resources/font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class FreeTypeFontHandle : public FontHandle {
	GDCLASS(FreeTypeFontHandle, FontHandle);

	friend class FreeTypeFont;

public:
	virtual void update_metrics(Size2 p_size, float p_oversampling = 1);
};

/*************************************************************************/

class FreeTypeFontData : public FontData {
	GDCLASS(FreeTypeFontData, FontData);

	PoolVector<uint8_t> buffer;

public:
	void _copy_internals_from(const FreeTypeFontData &p_font_data) {
		buffer = p_font_data.buffer;
	}

	virtual Ref<Resource> duplicate(bool p_subresources = false) const;

	virtual Error load_from_file(String p_path);
	virtual Error load_from_memory(const uint8_t *p_buffer, int p_size);

	PoolVector<uint8_t> get_buffer() const;
};

/*************************************************************************/

class FreeTypeFont : public Font {
	GDCLASS(FreeTypeFont, Font);

	friend class FreeTypeFontHandle;

public:
	enum Hinting {
		HINTING_NONE,
		HINTING_LIGHT,
		HINTING_NORMAL,
	};

private:
	Ref<FreeTypeFontData> data;
	Ref<FreeTypeFontHandle> handle;

	int size;
	Hinting hinting;
	bool force_autohinter;

protected:
	static void _bind_methods();

public:
	virtual Ref<FontData> get_data() const;
	virtual void set_data(const Ref<FontData> &p_data);

	virtual Ref<FontHandle> get_handle() const;

	virtual Size2 get_char_size(char32_t p_char) const;
	virtual Size2 get_string_size(const String &p_string) const;

	virtual bool get_use_mipmaps() const;
	virtual void set_use_mipmaps(bool p_enable);

	virtual bool get_use_filter() const;
	virtual void set_use_filter(bool p_enable);

	virtual float get_height() const;
	virtual float get_ascent() const;
	virtual float get_descent() const;

	int get_size() const;
	void set_size(int p_size);

	bool is_force_autohinter();
	void set_force_autohinter(bool p_force_autohinter);

	Hinting get_hinting() const;
	void set_hinting(Hinting p_hinting);

	FreeTypeFont();
};

VARIANT_ENUM_CAST(FreeTypeFont::Hinting);

/*************************************************************************/

class ResourceFormatLoaderFreeTypeFont : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif

#endif
