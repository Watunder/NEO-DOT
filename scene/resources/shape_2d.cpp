/*************************************************************************/
/*  shape_2d.cpp                                                         */
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

#include "shape_2d.h"

#include "core/engine.h"
#include "core/project_settings.h"
#include "servers/physics_2d_server.h"

RID Shape2D::get_rid() const {
	return shape;
}

void Shape2D::set_custom_solver_bias(real_t p_bias) {
	custom_bias = p_bias;
	Physics2DServer::get_singleton()->shape_set_custom_solver_bias(shape, custom_bias);
}

real_t Shape2D::get_custom_solver_bias() const {
	return custom_bias;
}

bool Shape2D::collide_with_motion(const Transform2D &p_local_xform, const Vector2 &p_local_motion, const Ref<Shape2D> &p_shape, const Transform2D &p_shape_xform, const Vector2 &p_shape_motion) {
	ERR_FAIL_COND_V(p_shape.is_null(), false);
	int r;
	return Physics2DServer::get_singleton()->shape_collide(get_rid(), p_local_xform, p_local_motion, p_shape->get_rid(), p_shape_xform, p_shape_motion, NULL, 0, r);
}

bool Shape2D::collide(const Transform2D &p_local_xform, const Ref<Shape2D> &p_shape, const Transform2D &p_shape_xform) {
	ERR_FAIL_COND_V(p_shape.is_null(), false);
	int r;
	return Physics2DServer::get_singleton()->shape_collide(get_rid(), p_local_xform, Vector2(), p_shape->get_rid(), p_shape_xform, Vector2(), NULL, 0, r);
}

Array Shape2D::collide_with_motion_and_get_contacts(const Transform2D &p_local_xform, const Vector2 &p_local_motion, const Ref<Shape2D> &p_shape, const Transform2D &p_shape_xform, const Vector2 &p_shape_motion) {
	ERR_FAIL_COND_V(p_shape.is_null(), Array());
	const int max_contacts = 16;
	Vector2 result[max_contacts * 2];
	int contacts = 0;

	if (!Physics2DServer::get_singleton()->shape_collide(get_rid(), p_local_xform, p_local_motion, p_shape->get_rid(), p_shape_xform, p_shape_motion, result, max_contacts, contacts))
		return Array();

	Array results;
	results.resize(contacts * 2);
	for (int i = 0; i < contacts * 2; i++) {
		results[i] = result[i];
	}

	return results;
}
Array Shape2D::collide_and_get_contacts(const Transform2D &p_local_xform, const Ref<Shape2D> &p_shape, const Transform2D &p_shape_xform) {
	ERR_FAIL_COND_V(p_shape.is_null(), Array());
	const int max_contacts = 16;
	Vector2 result[max_contacts * 2];
	int contacts = 0;

	if (!Physics2DServer::get_singleton()->shape_collide(get_rid(), p_local_xform, Vector2(), p_shape->get_rid(), p_shape_xform, Vector2(), result, max_contacts, contacts))
		return Array();

	Array results;
	results.resize(contacts * 2);
	for (int i = 0; i < contacts * 2; i++) {
		results[i] = result[i];
	}

	return results;
}

void Shape2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_custom_solver_bias", "bias"), &Shape2D::set_custom_solver_bias);
	ClassDB::bind_method(D_METHOD("get_custom_solver_bias"), &Shape2D::get_custom_solver_bias);
	ClassDB::bind_method(D_METHOD("collide", "local_xform", "with_shape", "shape_xform"), &Shape2D::collide);
	ClassDB::bind_method(D_METHOD("collide_with_motion", "local_xform", "local_motion", "with_shape", "shape_xform", "shape_motion"), &Shape2D::collide_with_motion);
	ClassDB::bind_method(D_METHOD("collide_and_get_contacts", "local_xform", "with_shape", "shape_xform"), &Shape2D::collide_and_get_contacts);
	ClassDB::bind_method(D_METHOD("collide_with_motion_and_get_contacts", "local_xform", "local_motion", "with_shape", "shape_xform", "shape_motion"), &Shape2D::collide_with_motion_and_get_contacts);
	ClassDB::bind_method(D_METHOD("draw", "canvas_item", "color"), &Shape2D::draw);

	ADD_PROPERTY(PropertyInfo(Variant::REAL, "custom_solver_bias", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_custom_solver_bias", "get_custom_solver_bias");
}

bool Shape2D::is_collision_outline_enabled() {
#ifdef TOOLS_ENABLED
	if (Engine::get_singleton()->is_editor_hint()) {
		return true;
	}
#endif
	return GLOBAL_DEF("debug/shapes/collision/draw_2d_outlines", true);
}

Shape2D::Shape2D(const RID &p_rid) {
	shape = p_rid;
	custom_bias = 0;
}

Shape2D::~Shape2D() {
	Physics2DServer::get_singleton()->free(shape);
}
