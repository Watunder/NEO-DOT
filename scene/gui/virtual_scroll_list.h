/*************************************************************************/
/*  virtual_scroll_list.h                                                */
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

#ifndef VIRTUAL_SCROLL_LIST_H
#define VIRTUAL_SCROLL_LIST_H

#include "scene/gui/container.h"
#include "scene/gui/scroll_bar.h"

class VirtualScrollList : public Control {
	GDCLASS(VirtualScrollList, Control);

public:
	enum SelectMode {
		SELECT_SINGLE,
		SELECT_MULTI
	};

private:
	NodePath item_template_path;
	Control *item_template;
	Rect2 fixed_item_rect;

	Vector<Variant> item_datas;

	SelectMode select_mode;
	Vector<int> selected_items;
	int first_selected_item;
	bool has_selected_item;

	float scroll;

	void _gui_input(const Ref<InputEvent> &p_event);

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void draw_item(Control *p_item_template, const Rect2 &p_rect, const Variant &p_item_data);

	void set_item_template(const NodePath &p_item_template_path);
	NodePath get_item_template() const;

	Rect2 get_item_rect() const;
	int get_column_count() const;
	real_t get_end_position() const;
	int get_item_at_position(const Vector2 &p_pos) const;

	void set_select_mode(SelectMode p_mode);
	SelectMode get_select_mode() const;

	void add_item(const Variant &p_item_data);
	void remove_item(int p_idx);

	VirtualScrollList();
	~VirtualScrollList();
};

VARIANT_ENUM_CAST(VirtualScrollList::SelectMode);

#endif
