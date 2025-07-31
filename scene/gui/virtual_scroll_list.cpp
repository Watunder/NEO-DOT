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
	ClassDB::bind_method("_gui_input", &VirtualScrollList::_gui_input);

	ClassDB::bind_method(D_METHOD("set_item_template", "item_template"), &VirtualScrollList::set_item_template);
	ClassDB::bind_method(D_METHOD("get_item_template"), &VirtualScrollList::get_item_template);
	ClassDB::bind_method(D_METHOD("add_item", "item"), &VirtualScrollList::add_item);
	ClassDB::bind_method(D_METHOD("remove_item"), &VirtualScrollList::remove_item);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "item_template", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "Control"), "set_item_template", "get_item_template");

	ADD_SIGNAL(MethodInfo("item_selected", PropertyInfo(Variant::INT, "index")));
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

			item_template->set_position(Vector2());
			item_template->set_size(get_item_size());

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
		case NOTIFICATION_DRAW: {
			if (!item_template || item_datas.empty())
				return;

			Rect2 template_box = item_template->get_rect();
			int cols = get_column_count();
			if (cols <= 0)
				return;

			int start_index = MAX(0, Math::floor(scroll / template_box.size.y) * cols);
			int visible_rows = Math::ceil(get_size().y / template_box.size.y) + 1;
			int end_index = MIN(item_datas.size(), start_index + (visible_rows * cols));

			if (start_index >= end_index)
				return;

			for (int i = start_index; i < end_index; i++) {
				int col = i % cols;
				int row = i / cols;

				Rect2 item_box = template_box;
				Vector2 new_pos = item_box.position;
				new_pos.y -= scroll;
				new_pos += Vector2(col * template_box.size.x, row * template_box.size.y);
				item_box.position = new_pos;

				Color color = (i == selected_idx) ? Color(1, 0, 0) : Color(0, 0, 0, 0);
				draw_rect(item_box, color, false, 8.0f);

				draw_item(item_template, item_box, item_datas[i]);
			}
		} break;
	}
}

void VirtualScrollList::draw_item(Control *p_item_template, const Rect2 &p_box, const Variant &p_item_data) {
	Rect2 item_box(p_box.position + p_item_template->get_position(), p_item_template->get_size());

	String item_template_type = p_item_template->get_class();
	if (item_template_type == "Label") {
		Label *label = Object::cast_to<Label>(p_item_template);

		String text;
		String name = label->get_name();
		text = label->get_text();

		Ref<Font> font = label->get_font("font");
		real_t font_height;
		String font_type = font->get_class();
		if (font_type == "DynamicFont") {
			Ref<DynamicFont> dynamic_font = font;
			font_height = dynamic_font->get_size();
		} else if (font_type == "BitmapFont") {
			Ref<BitmapFont> bitmap_font = font;
			font_height = bitmap_font->get_height();
			print_line(itos(font_height));
		}

		draw_string(font, item_box.position + Vector2(0, font_height), text);
	} else if (item_template_type == "ColorRect") {
		ColorRect *color_rect = Object::cast_to<ColorRect>(p_item_template);
		draw_rect(item_box, color_rect->get_frame_color(), true);
	} else if (item_template_type == "TextureRect") {
		TextureRect *texture_rect = Object::cast_to<TextureRect>(p_item_template);
		draw_texture_rect(texture_rect->get_texture(), item_box, false, texture_rect->get_modulate());
	}

	draw_rect(item_box, Color(1, 1, 1), false, 1.0f);

	for (int i = 0; i < p_item_template->get_child_count(); i++) {
		Control *item_template = Object::cast_to<Control>(p_item_template->get_child(i));
		if (item_template) {
			draw_item(item_template, item_box, p_item_data);
		}
	}
}

void VirtualScrollList::_gui_input(const Ref<InputEvent> &p_event) {
	if (!item_template || item_datas.empty())
		return;

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->get_button_index() == BUTTON_LEFT) {
			pressed = mb->is_pressed();
			if (pressed) {
				selected_idx = get_item_at_position(mb->get_position());
				emit_signal("item_selected", selected_idx);
			}
			update();
		} else if (mb->get_button_index() == BUTTON_WHEEL_DOWN) {
			scroll += scroll_tick_amount;
			update();
		} else if (mb->get_button_index() == BUTTON_WHEEL_UP) {
			scroll -= scroll_tick_amount;
			update();
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && pressed) {
		selected_idx = get_item_at_position(mm->get_position());
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

Vector2 VirtualScrollList::get_item_size() const {
	return Vector2(get_size().x, item_template->get_size().y);
}

int VirtualScrollList::get_column_count() const {
	return Math::floor(get_size().x / get_item_size().x);
}

real_t VirtualScrollList::get_end_position() const {
	int cols = get_column_count();
	if (cols == 0)
		return 0;

	return (item_datas.size() * item_template->get_size().y) / cols;
}

int VirtualScrollList::get_item_at_position(const Vector2 &p_pos) const {
	Vector2 pos = p_pos + Vector2(0.0f, scroll);
	Vector2 item_size = get_item_size();
	int cols = get_column_count();
	real_t width = item_size.x * cols;
	real_t height = get_end_position();

	if (!Rect2(Vector2(), Vector2(width, height)).has_point(pos)) {
		return -1;
	}

	int col = Math::floor(pos.x / item_size.x);
	int row = Math::floor(pos.y / item_size.y);

	return (row * cols) + col;
}

void VirtualScrollList::add_item(const Variant &p_item) {
	item_datas.push_back(p_item);
	update();
}

void VirtualScrollList::remove_item(int p_idx) {
	item_datas.remove(p_idx);
}

VirtualScrollList::VirtualScrollList() {
}

VirtualScrollList::~VirtualScrollList() {
}
