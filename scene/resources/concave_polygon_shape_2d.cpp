/*************************************************************************/
/*  concave_polygon_shape_2d.cpp                                         */
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

#include "concave_polygon_shape_2d.h"

#include "servers/physics_2d_server.h"
#include "servers/visual_server.h"

bool ConcavePolygonShape2D::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	PoolVector<Vector2> s = get_segments();
	int len = s.size();
	if (len == 0 || (len % 2) == 1)
		return false;

	PoolVector<Vector2>::Read r = s.read();
	for (int i = 0; i < len; i += 2) {
		Vector2 closest = Geometry::get_closest_point_to_segment_2d(p_point, &r[i]);
		if (p_point.distance_to(closest) < p_tolerance)
			return true;
	}

	return false;
}

void ConcavePolygonShape2D::set_segments(const PoolVector<Vector2> &p_segments) {
	Physics2DServer::get_singleton()->shape_set_data(get_rid(), p_segments);
	emit_changed();
}

PoolVector<Vector2> ConcavePolygonShape2D::get_segments() const {
	return Physics2DServer::get_singleton()->shape_get_data(get_rid());
}

void ConcavePolygonShape2D::draw(const RID &p_to_rid, const Color &p_color) {
	PoolVector<Vector2> s = get_segments();
	int len = s.size();
	if (len == 0 || (len % 2) == 1)
		return;

	PoolVector<Vector2>::Read r = s.read();
	for (int i = 0; i < len; i += 2) {
		VisualServer::get_singleton()->canvas_item_add_line(p_to_rid, r[i], r[i + 1], p_color, 2);
	}
}

Rect2 ConcavePolygonShape2D::get_rect() const {
	PoolVector<Vector2> s = get_segments();
	int len = s.size();
	if (len == 0)
		return Rect2();

	Rect2 rect;

	PoolVector<Vector2>::Read r = s.read();
	for (int i = 0; i < len; i++) {
		if (i == 0)
			rect.position = r[i];
		else
			rect.expand_to(r[i]);
	}

	return rect;
}

void ConcavePolygonShape2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_segments", "segments"), &ConcavePolygonShape2D::set_segments);
	ClassDB::bind_method(D_METHOD("get_segments"), &ConcavePolygonShape2D::get_segments);

	ADD_PROPERTY(PropertyInfo(Variant::POOL_VECTOR2_ARRAY, "segments"), "set_segments", "get_segments");
}

ConcavePolygonShape2D::ConcavePolygonShape2D() :
		Shape2D(Physics2DServer::get_singleton()->concave_polygon_shape_create()) {
	PoolVector<Vector2> empty;
	set_segments(empty);
}
