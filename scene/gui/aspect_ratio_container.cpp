/*************************************************************************/
/*  aspect_ratio_container.cpp                                           */
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

#include "aspect_ratio_container.h"

Size2 AspectRatioContainer::get_minimum_size() const {
	Size2 ms;
	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c) {
			continue;
		}
		if (c->is_set_as_toplevel()) {
			continue;
		}
		if (!c->is_visible()) {
			continue;
		}
		Size2 minsize = c->get_combined_minimum_size();
		ms.width = MAX(ms.width, minsize.width);
		ms.height = MAX(ms.height, minsize.height);
	}
	return ms;
}

void AspectRatioContainer::set_ratio(float p_ratio) {
	ratio = p_ratio;
	queue_sort();
}

void AspectRatioContainer::set_stretch_mode(StretchMode p_mode) {
	stretch_mode = p_mode;
	queue_sort();
}

void AspectRatioContainer::set_alignment_horizontal(AlignMode p_alignment_horizontal) {
	alignment_horizontal = p_alignment_horizontal;
	queue_sort();
}

void AspectRatioContainer::set_alignment_vertical(AlignMode p_alignment_vertical) {
	alignment_vertical = p_alignment_vertical;
	queue_sort();
}

void AspectRatioContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_SORT_CHILDREN: {
			Size2 size = get_size();
			for (int i = 0; i < get_child_count(); i++) {
				Control *c = Object::cast_to<Control>(get_child(i));
				if (!c) {
					continue;
				}
				if (c->is_set_as_toplevel()) {
					continue;
				}
				Size2 child_minsize = c->get_combined_minimum_size();
				Size2 child_size = Size2(ratio, 1.0);
				float scale_factor = 1.0;

				switch (stretch_mode) {
					case STRETCH_WIDTH_CONTROLS_HEIGHT: {
						scale_factor = size.x / child_size.x;
					} break;
					case STRETCH_HEIGHT_CONTROLS_WIDTH: {
						scale_factor = size.y / child_size.y;
					} break;
					case STRETCH_FIT: {
						scale_factor = MIN(size.x / child_size.x, size.y / child_size.y);
					} break;
					case STRETCH_COVER: {
						scale_factor = MAX(size.x / child_size.x, size.y / child_size.y);
					} break;
				}
				child_size *= scale_factor;
				child_size.x = MAX(child_size.x, child_minsize.x);
				child_size.y = MAX(child_size.y, child_minsize.y);

				float align_x = 0.5;
				switch (alignment_horizontal) {
					case ALIGN_BEGIN: {
						align_x = 0.0;
					} break;
					case ALIGN_CENTER: {
						align_x = 0.5;
					} break;
					case ALIGN_END: {
						align_x = 1.0;
					} break;
				}
				float align_y = 0.5;
				switch (alignment_vertical) {
					case ALIGN_BEGIN: {
						align_y = 0.0;
					} break;
					case ALIGN_CENTER: {
						align_y = 0.5;
					} break;
					case ALIGN_END: {
						align_y = 1.0;
					} break;
				}
				Vector2 offset = (size - child_size) * Vector2(align_x, align_y);

				fit_child_in_rect(c, Rect2(offset, child_size));
			}
		} break;
	}
}

void AspectRatioContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_ratio", "ratio"), &AspectRatioContainer::set_ratio);
	ClassDB::bind_method(D_METHOD("get_ratio"), &AspectRatioContainer::get_ratio);

	ClassDB::bind_method(D_METHOD("set_stretch_mode", "stretch_mode"), &AspectRatioContainer::set_stretch_mode);
	ClassDB::bind_method(D_METHOD("get_stretch_mode"), &AspectRatioContainer::get_stretch_mode);

	ClassDB::bind_method(D_METHOD("set_alignment_horizontal", "alignment_horizontal"), &AspectRatioContainer::set_alignment_horizontal);
	ClassDB::bind_method(D_METHOD("get_alignment_horizontal"), &AspectRatioContainer::get_alignment_horizontal);

	ClassDB::bind_method(D_METHOD("set_alignment_vertical", "alignment_vertical"), &AspectRatioContainer::set_alignment_vertical);
	ClassDB::bind_method(D_METHOD("get_alignment_vertical"), &AspectRatioContainer::get_alignment_vertical);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "ratio"), "set_ratio", "get_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stretch_mode", PROPERTY_HINT_ENUM, "Width controls height,Height controls width,Fit,Cover"), "set_stretch_mode", "get_stretch_mode");

	ADD_GROUP("Alignment", "alignment_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "alignment_horizontal", PROPERTY_HINT_ENUM, "Begin,Center,End"), "set_alignment_horizontal", "get_alignment_horizontal");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "alignment_vertical", PROPERTY_HINT_ENUM, "Begin,Center,End"), "set_alignment_vertical", "get_alignment_vertical");

	BIND_ENUM_CONSTANT(STRETCH_WIDTH_CONTROLS_HEIGHT);
	BIND_ENUM_CONSTANT(STRETCH_HEIGHT_CONTROLS_WIDTH);
	BIND_ENUM_CONSTANT(STRETCH_FIT);
	BIND_ENUM_CONSTANT(STRETCH_COVER);

	BIND_ENUM_CONSTANT(ALIGN_BEGIN);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_END);
}
