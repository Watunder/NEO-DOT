/*************************************************************************/
/*  animation_blend_space_2d.h                                           */
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

#ifndef ANIMATION_BLEND_SPACE_2D_H
#define ANIMATION_BLEND_SPACE_2D_H

#include "scene/animation/animation_tree.h"

class AnimationNodeBlendSpace2D : public AnimationRootNode {
	GDCLASS(AnimationNodeBlendSpace2D, AnimationRootNode);

public:
	enum BlendMode {
		BLEND_MODE_INTERPOLATED,
		BLEND_MODE_DISCRETE,
		BLEND_MODE_DISCRETE_CARRY,
	};

protected:
	enum {
		MAX_BLEND_POINTS = 64
	};

	struct BlendPoint {
		StringName name;
		Ref<AnimationRootNode> node;
		Vector2 position;
	};

	BlendPoint blend_points[MAX_BLEND_POINTS];
	int blend_points_used;

	struct BlendTriangle {
		int points[3];
	};

	Vector<BlendTriangle> triangles;

	StringName blend_position;
	StringName closest;
	StringName length_internal;
	Vector2 max_space;
	Vector2 min_space;
	Vector2 snap;
	String x_label;
	String y_label;
	BlendMode blend_mode;

	void _add_blend_point(int p_index, const Ref<AnimationRootNode> &p_node);
	void _set_triangles(const Vector<int> &p_triangles);
	Vector<int> _get_triangles() const;

	void _blend_triangle(const Vector2 &p_pos, const Vector2 *p_points, float *r_weights);

	bool auto_triangles;
	bool trianges_dirty;

	void _update_triangles();
	void _queue_auto_triangles();

	void _tree_changed();

protected:
	virtual void _validate_property(PropertyInfo &property) const;
	static void _bind_methods();

public:
	virtual void get_parameter_list(List<PropertyInfo> *r_list) const;
	virtual Variant get_parameter_default_value(const StringName &p_parameter) const;

	virtual void get_child_nodes(List<ChildNode> *r_child_nodes);

	void add_blend_point(const Ref<AnimationRootNode> &p_node, const Vector2 &p_position, int p_at_index = -1);
	void set_blend_point_position(int p_point, const Vector2 &p_position);
	void set_blend_point_node(int p_point, const Ref<AnimationRootNode> &p_node);
	Vector2 get_blend_point_position(int p_point) const;
	Ref<AnimationRootNode> get_blend_point_node(int p_point) const;
	void remove_blend_point(int p_point);
	int get_blend_point_count() const;

	bool has_triangle(int p_x, int p_y, int p_z) const;
	void add_triangle(int p_x, int p_y, int p_z, int p_at_index = -1);
	int get_triangle_point(int p_triangle, int p_point);
	void remove_triangle(int p_triangle);
	int get_triangle_count() const;

	void set_min_space(const Vector2 &p_min);
	Vector2 get_min_space() const;

	void set_max_space(const Vector2 &p_max);
	Vector2 get_max_space() const;

	void set_snap(const Vector2 &p_snap);
	Vector2 get_snap() const;

	void set_x_label(const String &p_label);
	String get_x_label() const;

	void set_y_label(const String &p_label);
	String get_y_label() const;

	virtual float process(float p_time, bool p_seek);
	virtual String get_caption() const;

	Vector2 get_closest_point(const Vector2 &p_point);

	void set_auto_triangles(bool p_enable);
	bool get_auto_triangles() const;

	void set_blend_mode(BlendMode p_blend_mode);
	BlendMode get_blend_mode() const;

	virtual Ref<AnimationNode> get_child_by_name(const StringName &p_name);

	AnimationNodeBlendSpace2D();
	~AnimationNodeBlendSpace2D();
};

VARIANT_ENUM_CAST(AnimationNodeBlendSpace2D::BlendMode)

#endif // ANIMATION_BLEND_SPACE_2D_H
