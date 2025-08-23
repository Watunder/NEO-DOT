/*************************************************************************/
/*  virtual_scroll_list.cpp                                              */
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

#include "virtual_scroll_list.h"

#include "core/engine.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/label.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/dynamic_font.h"
#include "scene/resources/font.h"

void VirtualScrollList::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_gui_input"), &VirtualScrollList::_gui_input);

	ClassDB::bind_method(D_METHOD("set_item_template", "item_template"), &VirtualScrollList::set_item_template);
	ClassDB::bind_method(D_METHOD("get_item_template"), &VirtualScrollList::get_item_template);
	ClassDB::bind_method(D_METHOD("add_item", "item"), &VirtualScrollList::add_item);
	ClassDB::bind_method(D_METHOD("remove_item"), &VirtualScrollList::remove_item);
	ClassDB::bind_method(D_METHOD("set_select_mode", "mode"), &VirtualScrollList::set_select_mode);
	ClassDB::bind_method(D_METHOD("get_select_mode"), &VirtualScrollList::get_select_mode);

	BIND_ENUM_CONSTANT(SELECT_SINGLE);
	BIND_ENUM_CONSTANT(SELECT_MULTI);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "item_template", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Control"), "set_item_template", "get_item_template");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "select_mode", PROPERTY_HINT_ENUM, "Single,Multi"), "set_select_mode", "get_select_mode");

	ADD_SIGNAL(MethodInfo("item_selected", PropertyInfo(Variant::INT, "index")));
	ADD_SIGNAL(MethodInfo("multi_selected", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::BOOL, "selected")));
}

void VirtualScrollList::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			item_template = Object::cast_to<Control>(get_node(item_template_path));
			set_process(true);
		} break;

		case NOTIFICATION_PROCESS: {
			if (!item_template)
				return;

			if (!item_datas.empty()) {
				if (scroll < 0) {
					scroll = Math::lerp(scroll, 0, get_process_delta_time() * 10);
					update();
				}
			} else {
				real_t end_pos = get_end_position();
				if (end_pos > get_size().y) {
					if (scroll + get_size().y > end_pos) {
						scroll = Math::lerp(scroll, end_pos - get_size().y, get_process_delta_time() * 10);
						update();
					}
				} else if (scroll > 0) {
					scroll = Math::lerp(scroll, 0, get_process_delta_time() * 10);
					update();
				}
			}
		} break;

		case NOTIFICATION_RESIZED: {
			call_deferred("update");
		} break;

		case NOTIFICATION_DRAW: {
			if (!item_template || item_datas.empty())
				return;

			Ref<StyleBox> bg = get_stylebox("bg");
			draw_style_box(bg, Rect2(Point2(), get_size()));

			if (has_focus()) {
				VisualServer::get_singleton()->canvas_item_add_clip_ignore(get_canvas_item(), true);
				draw_style_box(get_stylebox("bg_focus"), Rect2(Point2(), get_size()));
				VisualServer::get_singleton()->canvas_item_add_clip_ignore(get_canvas_item(), false);
			}

			int vseparation = get_constant("vseparation");
			int hseparation = get_constant("hseparation");

			fixed_item_rect = Rect2(Point2(vseparation, hseparation), item_template->get_size());
			fixed_item_rect.size.width -= vseparation * 2;
			fixed_item_rect.size.height += hseparation;

			int cols = get_column_count();
			if (cols <= 0)
				return;

			int visible_rows = Math::ceil(get_size().y / fixed_item_rect.size.height) + 1;
			int start_index = MAX(0, Math::floor(scroll / fixed_item_rect.size.height) * cols);
			int end_index = MIN(item_datas.size(), start_index + (visible_rows * cols));
			if (start_index >= end_index)
				return;

			Ref<StyleBox> sbsel = has_focus() ? get_stylebox("selected_focus") : get_stylebox("selected");
			Ref<StyleBox> cursor = has_focus() ? get_stylebox("cursor") : get_stylebox("cursor_unfocused");
			Color guide_color = get_color("guide_color");

			for (int i = start_index; i < end_index; i++) {
				int col = i % cols;
				int row = i / cols;

				Rect2 item_rect = fixed_item_rect;

				item_rect.position.y -= scroll;
				item_rect.position += Vector2(col * item_rect.size.width, row * item_rect.size.height);

				has_selected_item = (selected_items.find(i) != -1);
				if (has_selected_item) {
					draw_style_box(sbsel, item_rect);
				}
				if (select_mode == SELECT_MULTI) {
					if (selected_items.size() > 0 && selected_items[0] == i) {
						draw_style_box(cursor, item_rect);
						first_selected_item = i;
					} else if (selected_items.size() <= 0 && first_selected_item == i) {
						draw_style_box(cursor, item_rect);
					}
				}

				draw_item(item_template, item_rect, item_datas[i]);
				draw_line(Vector2(bg->get_margin(MARGIN_LEFT), item_rect.position.y - hseparation / 2),
						Vector2(item_rect.size.width, item_rect.position.y - hseparation / 2), guide_color);
			}
		} break;
	}
}

void VirtualScrollList::draw_item(Control *p_item_template, const Rect2 &p_rect, const Variant &p_item_data) {
	Rect2 item_rect(p_rect.position + p_item_template->get_position(), p_item_template->get_size());

	String item_template_type = p_item_template->get_class();
	if (item_template_type == "Label") {
		Label *label = Object::cast_to<Label>(p_item_template);

		String name = label->get_name();
		String text = label->get_text();

		Ref<Font> font = label->get_font("font");
		int font_height = font->get_height();
		Color font_color = has_selected_item ? get_color("font_color_selected") : get_color("font_color");

		draw_string(font, item_rect.position + Vector2(0, font_height), text, font_color);
	} else if (item_template_type == "ColorRect") {
		ColorRect *color_rect = Object::cast_to<ColorRect>(p_item_template);
		draw_rect(item_rect, color_rect->get_frame_color(), true);
	} else if (item_template_type == "TextureRect") {
		TextureRect *texture_rect = Object::cast_to<TextureRect>(p_item_template);
		draw_texture_rect(texture_rect->get_texture(), item_rect, false, texture_rect->get_modulate());
	}

	for (int i = 0; i < p_item_template->get_child_count(); i++) {
		Control *item_template = Object::cast_to<Control>(p_item_template->get_child(i));
		if (item_template) {
			draw_item(item_template, item_rect, p_item_data);
		}
	}
}

void VirtualScrollList::_gui_input(const Ref<InputEvent> &p_event) {
	if (!item_template || item_datas.empty())
		return;

	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid() && (mb->get_button_index() == BUTTON_LEFT && mb->is_pressed())) {
		int item = get_item_at_position(mb->get_position());
		if (select_mode == SELECT_MULTI && mb->get_command()) {
			if (selected_items.find(item) != -1) {
				selected_items.erase(item);
				emit_signal("multi_selected", item, false);
			} else {
				selected_items.push_back(item);
				emit_signal("multi_selected", item, true);
			}
		} else if (select_mode == SELECT_MULTI && mb->get_shift() && selected_items.size() > 0) {
			int from = first_selected_item;
			int to = item;
			if (to < from) {
				SWAP(from, to);
			}
			for (int i = from; i <= to; i++) {
				if (selected_items.find(i) == -1) {
					selected_items.push_back(i);
				}
				emit_signal("multi_selected", i, true);
			}
		} else {
			selected_items.clear();
			selected_items.push_back(item);
			if (select_mode == SELECT_SINGLE) {
				emit_signal("item_selected", item);
			} else {
				emit_signal("multi_selected", item, true);
			}
		}
		update();
	}

	if (mb.is_valid() && mb->get_button_index() == BUTTON_WHEEL_UP && mb->is_pressed()) {
		scroll -= 20;
		update();
	}
	if (mb.is_valid() && mb->get_button_index() == BUTTON_WHEEL_DOWN && mb->is_pressed()) {
		scroll += 20;
		update();
	}
}

void VirtualScrollList::set_item_template(const NodePath &p_item_template_path) {
	if (item_template_path == p_item_template_path)
		return;

	item_template_path = p_item_template_path;

	update();
}

NodePath VirtualScrollList::get_item_template() const {
	return item_template_path;
}

Rect2 VirtualScrollList::get_item_rect() const {
	return fixed_item_rect;
}

int VirtualScrollList::get_column_count() const {
	return Math::floor(get_size().width / get_item_rect().size.width);
}

real_t VirtualScrollList::get_end_position() const {
	int cols = get_column_count();
	if (cols == 0)
		return 0;

	return (item_datas.size() * get_item_rect().size.height) / cols;
}

int VirtualScrollList::get_item_at_position(const Vector2 &p_pos) const {
	Rect2 item_rect = get_item_rect();
	Point2 pos = p_pos + Point2(item_rect.position.x, scroll - item_rect.position.y);
	int cols = get_column_count();
	real_t width = item_rect.size.width * cols;
	real_t height = get_end_position();

	if (!Rect2(Point2(), Size2(width, height)).has_point(pos)) {
		return -1;
	}

	int col = Math::floor(pos.x / item_rect.size.width);
	int row = Math::floor(pos.y / item_rect.size.height);

	return (row * cols) + col;
}

void VirtualScrollList::add_item(const Variant &p_item) {
	item_datas.push_back(p_item);
	update();
}

void VirtualScrollList::remove_item(int p_idx) {
	item_datas.remove(p_idx);
}

void VirtualScrollList::set_select_mode(SelectMode p_mode) {
	select_mode = p_mode;
	update();
}

VirtualScrollList::SelectMode VirtualScrollList::get_select_mode() const {
	return select_mode;
}

VirtualScrollList::VirtualScrollList() {
	select_mode = SELECT_SINGLE;
	item_template = NULL;
	first_selected_item = -1;
	has_selected_item = false;

	scroll = 0.0f;

	set_focus_mode(FOCUS_ALL);
	set_clip_contents(true);
}

VirtualScrollList::~VirtualScrollList() {
}
