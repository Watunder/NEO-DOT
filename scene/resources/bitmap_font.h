/*************************************************************************/
/*  bitmap_font.h                                                        */
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

#ifndef BITMAP_FONT_H
#define BITMAP_FONT_H

#include "core/io/resource_loader.h"
#include "scene/resources/font.h"

class BitmapFont : public Font {
	GDCLASS(BitmapFont, Font);

private:
	RID font;
	String path_to_file;
	bool use_mipmaps;
	bool use_filter;

protected:
	static void _bind_methods();

public:
	virtual void reload_from_file();
	virtual RID get_rid() const;

	Error load(String p_path);
	String get_load_path() const;

	virtual Size2 get_char_size(char32_t p_char) const;
	virtual Size2 get_string_size(const String &p_string) const;

	virtual bool get_use_mipmaps() const;
	virtual void set_use_mipmaps(bool p_enable);

	virtual bool get_use_filter() const;
	virtual void set_use_filter(bool p_enable);

	virtual int get_spacing(int p_type) const;
	virtual void set_spacing(int p_type, int p_value);

	virtual float get_height() const;
	virtual float get_ascent() const;
	virtual float get_descent() const;

	BitmapFont();
	~BitmapFont();
};

/*************************************************************************/

class ResourceFormatLoaderBitmapFont : public ResourceFormatLoader {
public:
	virtual Ref<Resource> load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

#endif
