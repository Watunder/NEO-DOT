/*************************************************************************/
/*  AABB.hpp                                                             */
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

#ifndef AABB_H
#define AABB_H

#include "Vector3.hpp"

#include "Plane.hpp"

#include <cstdlib>

namespace godot {

class AABB {
public:
	Vector3 position;
	Vector3 size;

	real_t get_area() const; /// get area
	inline bool has_no_area() const {
		return (size.x <= CMP_EPSILON || size.y <= CMP_EPSILON || size.z <= CMP_EPSILON);
	}

	inline bool has_no_surface() const {
		return (size.x <= CMP_EPSILON && size.y <= CMP_EPSILON && size.z <= CMP_EPSILON);
	}

	inline const Vector3 &get_position() const { return position; }
	inline void set_position(const Vector3 &p_position) { position = p_position; }
	inline const Vector3 &get_size() const { return size; }
	inline void set_size(const Vector3 &p_size) { size = p_size; }

	bool operator==(const AABB &p_rval) const;
	bool operator!=(const AABB &p_rval) const;

	bool intersects(const AABB &p_aabb) const; /// Both AABBs overlap
	bool intersects_inclusive(const AABB &p_aabb) const; /// Both AABBs (or their faces) overlap
	bool encloses(const AABB &p_aabb) const; /// p_aabb is completely inside this

	AABB merge(const AABB &p_with) const;
	void merge_with(const AABB &p_aabb); ///merge with another AABB
	AABB intersection(const AABB &p_aabb) const; ///get box where two intersect, empty if no intersection occurs
	bool intersects_segment(const Vector3 &p_from, const Vector3 &p_to, Vector3 *r_clip = nullptr, Vector3 *r_normal = nullptr) const;
	bool intersects_ray(const Vector3 &p_from, const Vector3 &p_dir, Vector3 *r_clip = nullptr, Vector3 *r_normal = nullptr) const;
	bool smits_intersect_ray(const Vector3 &from, const Vector3 &p_dir, real_t t0, real_t t1) const;

	bool intersects_convex_shape(const Plane *p_plane, int p_plane_count) const;
	bool intersects_plane(const Plane &p_plane) const;

	bool has_point(const Vector3 &p_point) const;
	Vector3 get_support(const Vector3 &p_normal) const;

	Vector3 get_longest_axis() const;
	int get_longest_axis_index() const;
	real_t get_longest_axis_size() const;

	Vector3 get_shortest_axis() const;
	int get_shortest_axis_index() const;
	real_t get_shortest_axis_size() const;

	AABB grow(real_t p_by) const;
	void grow_by(real_t p_amount);

	void get_edge(int p_edge, Vector3 &r_from, Vector3 &r_to) const;
	Vector3 get_endpoint(int p_point) const;

	AABB expand(const Vector3 &p_vector) const;
	void project_range_in_plane(const Plane &p_plane, real_t &r_min, real_t &r_max) const;
	void expand_to(const Vector3 &p_vector); /** expand to contain a point if necesary */

	operator String() const;

	inline AABB() {}
	inline AABB(const Vector3 &p_pos, const Vector3 &p_size) {
		position = p_pos;
		size = p_size;
	}
};

} // namespace godot

#endif // RECT3_H
