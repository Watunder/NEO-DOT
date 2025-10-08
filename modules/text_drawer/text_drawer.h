/*************************************************************************/
/*  text_drawer.h                                                        */
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

#ifndef TEXT_DRAWER_H
#define TEXT_DRAWER_H

#include "scene/resources/dynamic_font.h"

#include "glyph_manager.h"

class TextDrawer : public Object {
	GDCLASS(TextDrawer, Object);

	static TextDrawer *singleton;

	GlyphManager *glyph_manager = NULL;

	void _draw_glyph(const Ref<DynamicFont> &p_font, RID p_canvas_item, const Vector2 &p_pos, const uint32_t &p_index, const Vector2 &p_offset, const Color &p_modulate = Color(1, 1, 1)) const;

protected:
	static void _bind_methods();

public:
	static TextDrawer *get_singleton();

	Vector2 draw(const Ref<DynamicFont> &p_font, RID p_canvas_item, const Vector2 &p_pos, const String &p_text, const Color &p_modulate = Color(1, 1, 1), int p_clip_w = -1) const;

	void clear_cache();

	TextDrawer();
	~TextDrawer();
};

#endif
