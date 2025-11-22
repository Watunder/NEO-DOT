/*************************************************************************/
/*  font_server.h                                                        */
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

#ifndef FONT_SERVER_H
#define FONT_SERVER_H

#include "core/os/thread_safe.h"
#include "scene/resources/font.h"

#include "font/glyph_manager.h"
#include "font/text_manager.h"

class FontServer : public Object {
	GDCLASS(FontServer, Object);

	_THREAD_SAFE_CLASS_

private:
	static FontServer *singleton;

	GlyphManager *glyph_manager = NULL;
	TextManger *text_manager = NULL;

protected:
	static void _bind_methods();

public:
	static FontServer *get_singleton();

	float draw_char(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, char32_t p_char, const Color &p_modulate = Color(1, 1, 1)) const;
	void draw_string(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, const String &p_text, const Color &p_modulate = Color(1, 1, 1), int p_clip_w = -1) const;
	void draw_string_aligned(RID p_canvas_item, const Ref<Font> &p_font, const Point2 &p_pos, HAlign p_align, float p_width, const String &p_text, const Color &p_modulate = Color(1, 1, 1)) const;

	void clear_glyph_cache(const FontHandle::CacheKey &p_cache_key);

	FontServer();
	~FontServer();
};

#endif
