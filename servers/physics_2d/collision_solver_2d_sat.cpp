/*************************************************************************/
/*  collision_solver_2d_sat.cpp                                          */
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

#include "collision_solver_2d_sat.h"

#include "core/math/geometry.h"

struct _CollectorCallback2D {
	CollisionSolver2DSW::CallbackResult callback;
	void *userdata;
	bool swap;
	bool collided;
	Vector2 normal;
	Vector2 *sep_axis;

	_FORCE_INLINE_ void call(const Vector2 &p_point_A, const Vector2 &p_point_B) {
		/*
		if (normal.dot(p_point_A) >= normal.dot(p_point_B))
			return;
		*/
		if (swap)
			callback(p_point_B, p_point_A, userdata);
		else
			callback(p_point_A, p_point_B, userdata);
	}
};

typedef void (*GenerateContactsFunc)(const Vector2 *, int, const Vector2 *, int, _CollectorCallback2D *);

_FORCE_INLINE_ static void _generate_contacts_point_point(const Vector2 *p_points_A, int p_point_count_A, const Vector2 *p_points_B, int p_point_count_B, _CollectorCallback2D *p_collector) {
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND(p_point_count_A != 1);
	ERR_FAIL_COND(p_point_count_B != 1);
#endif

	p_collector->call(*p_points_A, *p_points_B);
}

_FORCE_INLINE_ static void _generate_contacts_point_edge(const Vector2 *p_points_A, int p_point_count_A, const Vector2 *p_points_B, int p_point_count_B, _CollectorCallback2D *p_collector) {
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND(p_point_count_A != 1);
	ERR_FAIL_COND(p_point_count_B != 2);
#endif

	Vector2 closest_B = Geometry::get_closest_point_to_segment_uncapped_2d(*p_points_A, p_points_B);
	p_collector->call(*p_points_A, closest_B);
}

struct _generate_contacts_Pair {
	bool a;
	int idx;
	real_t d;
	_FORCE_INLINE_ bool operator<(const _generate_contacts_Pair &l) const { return d < l.d; }
};

_FORCE_INLINE_ static void _generate_contacts_edge_edge(const Vector2 *p_points_A, int p_point_count_A, const Vector2 *p_points_B, int p_point_count_B, _CollectorCallback2D *p_collector) {
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND(p_point_count_A != 2);
	ERR_FAIL_COND(p_point_count_B != 2); // circle is actually a 4x3 matrix
#endif

	Vector2 n = p_collector->normal;
	Vector2 t = n.tangent();
	real_t dA = n.dot(p_points_A[0]);
	real_t dB = n.dot(p_points_B[0]);

	_generate_contacts_Pair dvec[4];

	dvec[0].d = t.dot(p_points_A[0]);
	dvec[0].a = true;
	dvec[0].idx = 0;
	dvec[1].d = t.dot(p_points_A[1]);
	dvec[1].a = true;
	dvec[1].idx = 1;
	dvec[2].d = t.dot(p_points_B[0]);
	dvec[2].a = false;
	dvec[2].idx = 0;
	dvec[3].d = t.dot(p_points_B[1]);
	dvec[3].a = false;
	dvec[3].idx = 1;

	SortArray<_generate_contacts_Pair> sa;
	sa.sort(dvec, 4);

	for (int i = 1; i <= 2; i++) {
		if (dvec[i].a) {
			Vector2 a = p_points_A[dvec[i].idx];
			Vector2 b = n.plane_project(dB, a);
			if (n.dot(a) > n.dot(b) - CMP_EPSILON)
				continue;
			p_collector->call(a, b);
		} else {
			Vector2 b = p_points_B[dvec[i].idx];
			Vector2 a = n.plane_project(dA, b);
			if (n.dot(a) > n.dot(b) - CMP_EPSILON)
				continue;
			p_collector->call(a, b);
		}
	}
}

static void _generate_contacts_from_supports(const Vector2 *p_points_A, int p_point_count_A, const Vector2 *p_points_B, int p_point_count_B, _CollectorCallback2D *p_collector) {
#ifdef DEBUG_ENABLED
	ERR_FAIL_COND(p_point_count_A < 1);
	ERR_FAIL_COND(p_point_count_B < 1);
#endif

	static const GenerateContactsFunc generate_contacts_func_table[2][2] = {
		{
				_generate_contacts_point_point,
				_generate_contacts_point_edge,
		},
		{
				0,
				_generate_contacts_edge_edge,
		}
	};

	int pointcount_B;
	int pointcount_A;
	const Vector2 *points_A;
	const Vector2 *points_B;

	if (p_point_count_A > p_point_count_B) {
		//swap
		p_collector->swap = !p_collector->swap;
		p_collector->normal = -p_collector->normal;

		pointcount_B = p_point_count_A;
		pointcount_A = p_point_count_B;
		points_A = p_points_B;
		points_B = p_points_A;
	} else {
		pointcount_B = p_point_count_B;
		pointcount_A = p_point_count_A;
		points_A = p_points_A;
		points_B = p_points_B;
	}

	int version_A = (pointcount_A > 2 ? 2 : pointcount_A) - 1;
	int version_B = (pointcount_B > 2 ? 2 : pointcount_B) - 1;

	GenerateContactsFunc contacts_func = generate_contacts_func_table[version_A][version_B];
	ERR_FAIL_COND(!contacts_func);
	contacts_func(points_A, pointcount_A, points_B, pointcount_B, p_collector);
}

template <class ShapeA, class ShapeB, bool castA = false, bool castB = false, bool withMargin = false>
class SeparatorAxisTest2D {
	const ShapeA *shape_A;
	const ShapeB *shape_B;
	const Transform2D *transform_A;
	const Transform2D *transform_B;
	real_t best_depth;
	Vector2 best_axis;
	int best_axis_count;
	int best_axis_index;
	Vector2 motion_A;
	Vector2 motion_B;
	real_t margin_A;
	real_t margin_B;
	_CollectorCallback2D *callback;

public:
	_FORCE_INLINE_ bool test_previous_axis() {
		if (callback && callback->sep_axis && *callback->sep_axis != Vector2()) {
			return test_axis(*callback->sep_axis);
		} else {
#ifdef DEBUG_ENABLED
			best_axis_count++;
#endif
		}
		return true;
	}

	_FORCE_INLINE_ bool test_cast() {
		if (castA) {
			Vector2 na = motion_A.normalized();
			if (!test_axis(na))
				return false;
			if (!test_axis(na.tangent()))
				return false;
		}

		if (castB) {
			Vector2 nb = motion_B.normalized();
			if (!test_axis(nb))
				return false;
			if (!test_axis(nb.tangent()))
				return false;
		}

		return true;
	}

	_FORCE_INLINE_ bool test_axis(const Vector2 &p_axis) {
		Vector2 axis = p_axis;

		if (Math::is_zero_approx(axis.x) &&
				Math::is_zero_approx(axis.y)) {
			// strange case, try an upwards separator
			axis = Vector2(0.0, 1.0);
		}

		real_t min_A, max_A, min_B, max_B;

		if (castA)
			shape_A->project_range_cast(motion_A, axis, *transform_A, min_A, max_A);
		else
			shape_A->project_range(axis, *transform_A, min_A, max_A);

		if (castB)
			shape_B->project_range_cast(motion_B, axis, *transform_B, min_B, max_B);
		else
			shape_B->project_range(axis, *transform_B, min_B, max_B);

		if (withMargin) {
			min_A -= margin_A;
			max_A += margin_A;
			min_B -= margin_B;
			max_B += margin_B;
		}

		min_B -= (max_A - min_A) * 0.5;
		max_B += (max_A - min_A) * 0.5;

		real_t dmin = min_B - (min_A + max_A) * 0.5;
		real_t dmax = max_B - (min_A + max_A) * 0.5;

		if (dmin > 0.0 || dmax < 0.0) {
			if (callback && callback->sep_axis)
				*callback->sep_axis = axis;
#ifdef DEBUG_ENABLED
			best_axis_count++;
#endif

			return false; // doesn't contain 0
		}

		//use the smallest depth

		dmin = Math::abs(dmin);

		if (dmax < dmin) {
			if (dmax < best_depth) {
				best_depth = dmax;
				best_axis = axis;
#ifdef DEBUG_ENABLED
				best_axis_index = best_axis_count;
#endif
			}
		} else {
			if (dmin < best_depth) {
				best_depth = dmin;
				best_axis = -axis; // keep it as A axis
#ifdef DEBUG_ENABLED
				best_axis_index = best_axis_count;
#endif
			}
		}

#ifdef DEBUG_ENABLED
		best_axis_count++;
#endif

		return true;
	}

	_FORCE_INLINE_ void generate_contacts() {
		// nothing to do, don't generate
		if (best_axis == Vector2(0.0, 0.0))
			return;

		if (callback) {
			callback->collided = true;

			if (!callback->callback)
				return; //only collide, no callback
		}
		static const int max_supports = 2;

		Vector2 supports_A[max_supports];
		int support_count_A;
		if (castA) {
			shape_A->get_supports_transformed_cast(motion_A, -best_axis, *transform_A, supports_A, support_count_A);
		} else {
			shape_A->get_supports(transform_A->basis_xform_inv(-best_axis).normalized(), supports_A, support_count_A);
			for (int i = 0; i < support_count_A; i++) {
				supports_A[i] = transform_A->xform(supports_A[i]);
			}
		}

		if (withMargin) {
			for (int i = 0; i < support_count_A; i++) {
				supports_A[i] += -best_axis * margin_A;
			}
		}

		Vector2 supports_B[max_supports];
		int support_count_B;
		if (castB) {
			shape_B->get_supports_transformed_cast(motion_B, best_axis, *transform_B, supports_B, support_count_B);
		} else {
			shape_B->get_supports(transform_B->basis_xform_inv(best_axis).normalized(), supports_B, support_count_B);
			for (int i = 0; i < support_count_B; i++) {
				supports_B[i] = transform_B->xform(supports_B[i]);
			}
		}

		if (withMargin) {
			for (int i = 0; i < support_count_B; i++) {
				supports_B[i] += best_axis * margin_B;
			}
		}
		if (callback) {
			callback->normal = best_axis;
			_generate_contacts_from_supports(supports_A, support_count_A, supports_B, support_count_B, callback);

			if (callback->sep_axis && *callback->sep_axis != Vector2())
				*callback->sep_axis = Vector2(); //invalidate previous axis (no test)
		}
	}

	_FORCE_INLINE_ SeparatorAxisTest2D(const ShapeA *p_shape_A, const Transform2D &p_transform_a, const ShapeB *p_shape_B, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_A = Vector2(), const Vector2 &p_motion_B = Vector2(), real_t p_margin_A = 0, real_t p_margin_B = 0) {
		margin_A = p_margin_A;
		margin_B = p_margin_B;
		best_depth = 1e15;
		shape_A = p_shape_A;
		shape_B = p_shape_B;
		transform_A = &p_transform_a;
		transform_B = &p_transform_b;
		motion_A = p_motion_A;
		motion_B = p_motion_B;

		callback = p_collector;
#ifdef DEBUG_ENABLED
		best_axis_count = 0;
		best_axis_index = -1;
#endif
	}
};

/****** SAT TESTS *******/

#define TEST_POINT(m_a, m_b)                                                                \
	((!separator.test_axis(((m_a) - (m_b)).normalized())) ||                                \
			(castA && !separator.test_axis(((m_a) + p_motion_a - (m_b)).normalized())) ||   \
			(castB && !separator.test_axis(((m_a) - ((m_b) + p_motion_b)).normalized())) || \
			(castA && castB && !separator.test_axis(((m_a) + p_motion_a - ((m_b) + p_motion_b)).normalized())))

typedef void (*CollisionFunc)(const Shape2DSW *, const Transform2D &, const Shape2DSW *, const Transform2D &, _CollectorCallback2D *p_collector, const Vector2 &, const Vector2 &, real_t, real_t);

template <bool castA, bool castB, bool withMargin>
static void _collision_segment_segment(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const SegmentShape2DSW *segment_A = static_cast<const SegmentShape2DSW *>(p_a);
	const SegmentShape2DSW *segment_B = static_cast<const SegmentShape2DSW *>(p_b);

	SeparatorAxisTest2D<SegmentShape2DSW, SegmentShape2DSW, castA, castB, withMargin> separator(segment_A, p_transform_a, segment_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;
	//this collision is kind of pointless

	if (!separator.test_cast())
		return;

	if (!separator.test_axis(segment_A->get_xformed_normal(p_transform_a)))
		return;
	if (!separator.test_axis(segment_B->get_xformed_normal(p_transform_b)))
		return;

	if (withMargin) {
		//points grow to circles

		if (TEST_POINT(p_transform_a.xform(segment_A->get_a()), p_transform_b.xform(segment_B->get_a())))
			return;
		if (TEST_POINT(p_transform_a.xform(segment_A->get_a()), p_transform_b.xform(segment_B->get_b())))
			return;
		if (TEST_POINT(p_transform_a.xform(segment_A->get_b()), p_transform_b.xform(segment_B->get_a())))
			return;
		if (TEST_POINT(p_transform_a.xform(segment_A->get_b()), p_transform_b.xform(segment_B->get_b())))
			return;
	}

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_segment_circle(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const SegmentShape2DSW *segment_A = static_cast<const SegmentShape2DSW *>(p_a);
	const CircleShape2DSW *circle_B = static_cast<const CircleShape2DSW *>(p_b);

	SeparatorAxisTest2D<SegmentShape2DSW, CircleShape2DSW, castA, castB, withMargin> separator(segment_A, p_transform_a, circle_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//segment normal
	if (!separator.test_axis(
				(p_transform_a.xform(segment_A->get_b()) - p_transform_a.xform(segment_A->get_a())).normalized().tangent()))
		return;

	//endpoint a vs circle
	if (TEST_POINT(p_transform_a.xform(segment_A->get_a()), p_transform_b.get_origin()))
		return;
	//endpoint b vs circle
	if (TEST_POINT(p_transform_a.xform(segment_A->get_b()), p_transform_b.get_origin()))
		return;

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_segment_rectangle(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const SegmentShape2DSW *segment_A = static_cast<const SegmentShape2DSW *>(p_a);
	const RectangleShape2DSW *rectangle_B = static_cast<const RectangleShape2DSW *>(p_b);

	SeparatorAxisTest2D<SegmentShape2DSW, RectangleShape2DSW, castA, castB, withMargin> separator(segment_A, p_transform_a, rectangle_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	if (!separator.test_axis(segment_A->get_xformed_normal(p_transform_a)))
		return;

	if (!separator.test_axis(p_transform_b.elements[0].normalized()))
		return;

	if (!separator.test_axis(p_transform_b.elements[1].normalized()))
		return;

	if (withMargin) {
		Transform2D inv = p_transform_b.affine_inverse();

		Vector2 a = p_transform_a.xform(segment_A->get_a());
		Vector2 b = p_transform_a.xform(segment_A->get_b());

		if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, a)))
			return;
		if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, b)))
			return;

		if (castA) {
			if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, a + p_motion_a)))
				return;
			if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, b + p_motion_a)))
				return;
		}

		if (castB) {
			if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, a - p_motion_b)))
				return;
			if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, b - p_motion_b)))
				return;
		}

		if (castA && castB) {
			if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, a - p_motion_b + p_motion_a)))
				return;
			if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, inv, b - p_motion_b + p_motion_a)))
				return;
		}
	}

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_segment_capsule(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const SegmentShape2DSW *segment_A = static_cast<const SegmentShape2DSW *>(p_a);
	const CapsuleShape2DSW *capsule_B = static_cast<const CapsuleShape2DSW *>(p_b);

	SeparatorAxisTest2D<SegmentShape2DSW, CapsuleShape2DSW, castA, castB, withMargin> separator(segment_A, p_transform_a, capsule_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	if (!separator.test_axis(segment_A->get_xformed_normal(p_transform_a)))
		return;

	if (!separator.test_axis(p_transform_b.elements[0].normalized()))
		return;

	if (TEST_POINT(p_transform_a.xform(segment_A->get_a()), (p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * 0.5)))
		return;
	if (TEST_POINT(p_transform_a.xform(segment_A->get_a()), (p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * -0.5)))
		return;
	if (TEST_POINT(p_transform_a.xform(segment_A->get_b()), (p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * 0.5)))
		return;
	if (TEST_POINT(p_transform_a.xform(segment_A->get_b()), (p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * -0.5)))
		return;

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_segment_convex_polygon(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const SegmentShape2DSW *segment_A = static_cast<const SegmentShape2DSW *>(p_a);
	const ConvexPolygonShape2DSW *convex_B = static_cast<const ConvexPolygonShape2DSW *>(p_b);

	SeparatorAxisTest2D<SegmentShape2DSW, ConvexPolygonShape2DSW, castA, castB, withMargin> separator(segment_A, p_transform_a, convex_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	if (!separator.test_axis(segment_A->get_xformed_normal(p_transform_a)))
		return;

	for (int i = 0; i < convex_B->get_point_count(); i++) {
		if (!separator.test_axis(convex_B->get_xformed_segment_normal(p_transform_b, i)))
			return;

		if (withMargin) {
			if (TEST_POINT(p_transform_a.xform(segment_A->get_a()), p_transform_b.xform(convex_B->get_point(i))))
				return;
			if (TEST_POINT(p_transform_a.xform(segment_A->get_b()), p_transform_b.xform(convex_B->get_point(i))))
				return;
		}
	}

	separator.generate_contacts();
}

/////////

template <bool castA, bool castB, bool withMargin>
static void _collision_circle_circle(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const CircleShape2DSW *circle_A = static_cast<const CircleShape2DSW *>(p_a);
	const CircleShape2DSW *circle_B = static_cast<const CircleShape2DSW *>(p_b);

	SeparatorAxisTest2D<CircleShape2DSW, CircleShape2DSW, castA, castB, withMargin> separator(circle_A, p_transform_a, circle_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	if (TEST_POINT(p_transform_a.get_origin(), p_transform_b.get_origin()))
		return;

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_circle_rectangle(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const CircleShape2DSW *circle_A = static_cast<const CircleShape2DSW *>(p_a);
	const RectangleShape2DSW *rectangle_B = static_cast<const RectangleShape2DSW *>(p_b);

	SeparatorAxisTest2D<CircleShape2DSW, RectangleShape2DSW, castA, castB, withMargin> separator(circle_A, p_transform_a, rectangle_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	const Vector2 &sphere = p_transform_a.elements[2];
	const Vector2 *axis = &p_transform_b.elements[0];
	//const Vector2& half_extents = rectangle_B->get_half_extents();

	if (!separator.test_axis(axis[0].normalized()))
		return;

	if (!separator.test_axis(axis[1].normalized()))
		return;

	Transform2D binv = p_transform_b.affine_inverse();
	{
		if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, binv, sphere)))
			return;
	}

	if (castA) {
		Vector2 sphereofs = sphere + p_motion_a;
		if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, binv, sphereofs)))
			return;
	}

	if (castB) {
		Vector2 sphereofs = sphere - p_motion_b;
		if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, binv, sphereofs)))
			return;
	}

	if (castA && castB) {
		Vector2 sphereofs = sphere - p_motion_b + p_motion_a;
		if (!separator.test_axis(rectangle_B->get_circle_axis(p_transform_b, binv, sphereofs)))
			return;
	}

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_circle_capsule(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const CircleShape2DSW *circle_A = static_cast<const CircleShape2DSW *>(p_a);
	const CapsuleShape2DSW *capsule_B = static_cast<const CapsuleShape2DSW *>(p_b);

	SeparatorAxisTest2D<CircleShape2DSW, CapsuleShape2DSW, castA, castB, withMargin> separator(circle_A, p_transform_a, capsule_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//capsule axis
	if (!separator.test_axis(p_transform_b.elements[0].normalized()))
		return;

	//capsule endpoints
	if (TEST_POINT(p_transform_a.get_origin(), (p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * 0.5)))
		return;
	if (TEST_POINT(p_transform_a.get_origin(), (p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * -0.5)))
		return;

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_circle_convex_polygon(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const CircleShape2DSW *circle_A = static_cast<const CircleShape2DSW *>(p_a);
	const ConvexPolygonShape2DSW *convex_B = static_cast<const ConvexPolygonShape2DSW *>(p_b);

	SeparatorAxisTest2D<CircleShape2DSW, ConvexPolygonShape2DSW, castA, castB, withMargin> separator(circle_A, p_transform_a, convex_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//poly faces and poly points vs circle
	for (int i = 0; i < convex_B->get_point_count(); i++) {
		if (TEST_POINT(p_transform_a.get_origin(), p_transform_b.xform(convex_B->get_point(i))))
			return;

		if (!separator.test_axis(convex_B->get_xformed_segment_normal(p_transform_b, i)))
			return;
	}

	separator.generate_contacts();
}

/////////

template <bool castA, bool castB, bool withMargin>
static void _collision_rectangle_rectangle(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const RectangleShape2DSW *rectangle_A = static_cast<const RectangleShape2DSW *>(p_a);
	const RectangleShape2DSW *rectangle_B = static_cast<const RectangleShape2DSW *>(p_b);

	SeparatorAxisTest2D<RectangleShape2DSW, RectangleShape2DSW, castA, castB, withMargin> separator(rectangle_A, p_transform_a, rectangle_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//box faces A
	if (!separator.test_axis(p_transform_a.elements[0].normalized()))
		return;

	if (!separator.test_axis(p_transform_a.elements[1].normalized()))
		return;

	//box faces B
	if (!separator.test_axis(p_transform_b.elements[0].normalized()))
		return;

	if (!separator.test_axis(p_transform_b.elements[1].normalized()))
		return;

	if (withMargin) {
		Transform2D invA = p_transform_a.affine_inverse();
		Transform2D invB = p_transform_b.affine_inverse();

		if (!separator.test_axis(rectangle_A->get_box_axis(p_transform_a, invA, rectangle_B, p_transform_b, invB)))
			return;

		if (castA || castB) {
			Transform2D aofs = p_transform_a;
			aofs.elements[2] += p_motion_a;

			Transform2D bofs = p_transform_b;
			bofs.elements[2] += p_motion_b;

			Transform2D aofsinv = aofs.affine_inverse();
			Transform2D bofsinv = bofs.affine_inverse();

			if (castA) {
				if (!separator.test_axis(rectangle_A->get_box_axis(aofs, aofsinv, rectangle_B, p_transform_b, invB)))
					return;
			}

			if (castB) {
				if (!separator.test_axis(rectangle_A->get_box_axis(p_transform_a, invA, rectangle_B, bofs, bofsinv)))
					return;
			}

			if (castA && castB) {
				if (!separator.test_axis(rectangle_A->get_box_axis(aofs, aofsinv, rectangle_B, bofs, bofsinv)))
					return;
			}
		}
	}

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_rectangle_capsule(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const RectangleShape2DSW *rectangle_A = static_cast<const RectangleShape2DSW *>(p_a);
	const CapsuleShape2DSW *capsule_B = static_cast<const CapsuleShape2DSW *>(p_b);

	SeparatorAxisTest2D<RectangleShape2DSW, CapsuleShape2DSW, castA, castB, withMargin> separator(rectangle_A, p_transform_a, capsule_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//box faces
	if (!separator.test_axis(p_transform_a.elements[0].normalized()))
		return;

	if (!separator.test_axis(p_transform_a.elements[1].normalized()))
		return;

	//capsule axis
	if (!separator.test_axis(p_transform_b.elements[0].normalized()))
		return;

	//box endpoints to capsule circles

	Transform2D boxinv = p_transform_a.affine_inverse();

	for (int i = 0; i < 2; i++) {
		{
			Vector2 capsule_endpoint = p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * (i == 0 ? 0.5 : -0.5);

			if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, capsule_endpoint)))
				return;
		}

		if (castA) {
			Vector2 capsule_endpoint = p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * (i == 0 ? 0.5 : -0.5);
			capsule_endpoint -= p_motion_a;

			if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, capsule_endpoint)))
				return;
		}

		if (castB) {
			Vector2 capsule_endpoint = p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * (i == 0 ? 0.5 : -0.5);
			capsule_endpoint += p_motion_b;

			if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, capsule_endpoint)))
				return;
		}

		if (castA && castB) {
			Vector2 capsule_endpoint = p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * (i == 0 ? 0.5 : -0.5);
			capsule_endpoint -= p_motion_a;
			capsule_endpoint += p_motion_b;

			if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, capsule_endpoint)))
				return;
		}
	}

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_rectangle_convex_polygon(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const RectangleShape2DSW *rectangle_A = static_cast<const RectangleShape2DSW *>(p_a);
	const ConvexPolygonShape2DSW *convex_B = static_cast<const ConvexPolygonShape2DSW *>(p_b);

	SeparatorAxisTest2D<RectangleShape2DSW, ConvexPolygonShape2DSW, castA, castB, withMargin> separator(rectangle_A, p_transform_a, convex_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//box faces
	if (!separator.test_axis(p_transform_a.elements[0].normalized()))
		return;

	if (!separator.test_axis(p_transform_a.elements[1].normalized()))
		return;

	//convex faces
	Transform2D boxinv;
	if (withMargin) {
		boxinv = p_transform_a.affine_inverse();
	}
	for (int i = 0; i < convex_B->get_point_count(); i++) {
		if (!separator.test_axis(convex_B->get_xformed_segment_normal(p_transform_b, i)))
			return;

		if (withMargin) {
			//all points vs all points need to be tested if margin exist
			if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, p_transform_b.xform(convex_B->get_point(i)))))
				return;
			if (castA) {
				if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, p_transform_b.xform(convex_B->get_point(i)) - p_motion_a)))
					return;
			}
			if (castB) {
				if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, p_transform_b.xform(convex_B->get_point(i)) + p_motion_b)))
					return;
			}
			if (castA && castB) {
				if (!separator.test_axis(rectangle_A->get_circle_axis(p_transform_a, boxinv, p_transform_b.xform(convex_B->get_point(i)) + p_motion_b - p_motion_a)))
					return;
			}
		}
	}

	separator.generate_contacts();
}

/////////

template <bool castA, bool castB, bool withMargin>
static void _collision_capsule_capsule(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const CapsuleShape2DSW *capsule_A = static_cast<const CapsuleShape2DSW *>(p_a);
	const CapsuleShape2DSW *capsule_B = static_cast<const CapsuleShape2DSW *>(p_b);

	SeparatorAxisTest2D<CapsuleShape2DSW, CapsuleShape2DSW, castA, castB, withMargin> separator(capsule_A, p_transform_a, capsule_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//capsule axis

	if (!separator.test_axis(p_transform_b.elements[0].normalized()))
		return;

	if (!separator.test_axis(p_transform_a.elements[0].normalized()))
		return;

	//capsule endpoints

	for (int i = 0; i < 2; i++) {
		Vector2 capsule_endpoint_A = p_transform_a.get_origin() + p_transform_a.elements[1] * capsule_A->get_height() * (i == 0 ? 0.5 : -0.5);

		for (int j = 0; j < 2; j++) {
			Vector2 capsule_endpoint_B = p_transform_b.get_origin() + p_transform_b.elements[1] * capsule_B->get_height() * (j == 0 ? 0.5 : -0.5);

			if (TEST_POINT(capsule_endpoint_A, capsule_endpoint_B))
				return;
		}
	}

	separator.generate_contacts();
}

template <bool castA, bool castB, bool withMargin>
static void _collision_capsule_convex_polygon(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const CapsuleShape2DSW *capsule_A = static_cast<const CapsuleShape2DSW *>(p_a);
	const ConvexPolygonShape2DSW *convex_B = static_cast<const ConvexPolygonShape2DSW *>(p_b);

	SeparatorAxisTest2D<CapsuleShape2DSW, ConvexPolygonShape2DSW, castA, castB, withMargin> separator(capsule_A, p_transform_a, convex_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	//capsule axis

	if (!separator.test_axis(p_transform_a.elements[0].normalized()))
		return;

	//poly vs capsule
	for (int i = 0; i < convex_B->get_point_count(); i++) {
		Vector2 cpoint = p_transform_b.xform(convex_B->get_point(i));

		for (int j = 0; j < 2; j++) {
			Vector2 capsule_endpoint_A = p_transform_a.get_origin() + p_transform_a.elements[1] * capsule_A->get_height() * (j == 0 ? 0.5 : -0.5);

			if (TEST_POINT(capsule_endpoint_A, cpoint))
				return;
		}

		if (!separator.test_axis(convex_B->get_xformed_segment_normal(p_transform_b, i)))
			return;
	}

	separator.generate_contacts();
}

/////////

template <bool castA, bool castB, bool withMargin>
static void _collision_convex_polygon_convex_polygon(const Shape2DSW *p_a, const Transform2D &p_transform_a, const Shape2DSW *p_b, const Transform2D &p_transform_b, _CollectorCallback2D *p_collector, const Vector2 &p_motion_a, const Vector2 &p_motion_b, real_t p_margin_A, real_t p_margin_B) {
	const ConvexPolygonShape2DSW *convex_A = static_cast<const ConvexPolygonShape2DSW *>(p_a);
	const ConvexPolygonShape2DSW *convex_B = static_cast<const ConvexPolygonShape2DSW *>(p_b);

	SeparatorAxisTest2D<ConvexPolygonShape2DSW, ConvexPolygonShape2DSW, castA, castB, withMargin> separator(convex_A, p_transform_a, convex_B, p_transform_b, p_collector, p_motion_a, p_motion_b, p_margin_A, p_margin_B);

	if (!separator.test_previous_axis())
		return;

	if (!separator.test_cast())
		return;

	for (int i = 0; i < convex_A->get_point_count(); i++) {
		if (!separator.test_axis(convex_A->get_xformed_segment_normal(p_transform_a, i)))
			return;
	}

	for (int i = 0; i < convex_B->get_point_count(); i++) {
		if (!separator.test_axis(convex_B->get_xformed_segment_normal(p_transform_b, i)))
			return;
	}

	if (withMargin) {
		for (int i = 0; i < convex_A->get_point_count(); i++) {
			for (int j = 0; j < convex_B->get_point_count(); j++) {
				if (TEST_POINT(p_transform_a.xform(convex_A->get_point(i)), p_transform_b.xform(convex_B->get_point(j))))
					return;
			}
		}
	}

	separator.generate_contacts();
}

////////

bool sat_2d_calculate_penetration(const Shape2DSW *p_shape_A, const Transform2D &p_transform_A, const Vector2 &p_motion_A, const Shape2DSW *p_shape_B, const Transform2D &p_transform_B, const Vector2 &p_motion_B, CollisionSolver2DSW::CallbackResult p_result_callback, void *p_userdata, bool p_swap, Vector2 *sep_axis, real_t p_margin_A, real_t p_margin_B) {
	Physics2DServer::ShapeType type_A = p_shape_A->get_type();

	ERR_FAIL_COND_V(type_A == Physics2DServer::SHAPE_LINE, false);
	//ERR_FAIL_COND_V(type_A==Physics2DServer::SHAPE_RAY,false);
	ERR_FAIL_COND_V(p_shape_A->is_concave(), false);

	Physics2DServer::ShapeType type_B = p_shape_B->get_type();

	ERR_FAIL_COND_V(type_B == Physics2DServer::SHAPE_LINE, false);
	//ERR_FAIL_COND_V(type_B==Physics2DServer::SHAPE_RAY,false);
	ERR_FAIL_COND_V(p_shape_B->is_concave(), false);

	static const CollisionFunc collision_table[5][5] = {
		{ _collision_segment_segment<false, false, false>,
				_collision_segment_circle<false, false, false>,
				_collision_segment_rectangle<false, false, false>,
				_collision_segment_capsule<false, false, false>,
				_collision_segment_convex_polygon<false, false, false> },
		{ 0,
				_collision_circle_circle<false, false, false>,
				_collision_circle_rectangle<false, false, false>,
				_collision_circle_capsule<false, false, false>,
				_collision_circle_convex_polygon<false, false, false> },
		{ 0,
				0,
				_collision_rectangle_rectangle<false, false, false>,
				_collision_rectangle_capsule<false, false, false>,
				_collision_rectangle_convex_polygon<false, false, false> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<false, false, false>,
				_collision_capsule_convex_polygon<false, false, false> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<false, false, false> }

	};

	static const CollisionFunc collision_table_castA[5][5] = {
		{ _collision_segment_segment<true, false, false>,
				_collision_segment_circle<true, false, false>,
				_collision_segment_rectangle<true, false, false>,
				_collision_segment_capsule<true, false, false>,
				_collision_segment_convex_polygon<true, false, false> },
		{ 0,
				_collision_circle_circle<true, false, false>,
				_collision_circle_rectangle<true, false, false>,
				_collision_circle_capsule<true, false, false>,
				_collision_circle_convex_polygon<true, false, false> },
		{ 0,
				0,
				_collision_rectangle_rectangle<true, false, false>,
				_collision_rectangle_capsule<true, false, false>,
				_collision_rectangle_convex_polygon<true, false, false> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<true, false, false>,
				_collision_capsule_convex_polygon<true, false, false> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<true, false, false> }

	};

	static const CollisionFunc collision_table_castB[5][5] = {
		{ _collision_segment_segment<false, true, false>,
				_collision_segment_circle<false, true, false>,
				_collision_segment_rectangle<false, true, false>,
				_collision_segment_capsule<false, true, false>,
				_collision_segment_convex_polygon<false, true, false> },
		{ 0,
				_collision_circle_circle<false, true, false>,
				_collision_circle_rectangle<false, true, false>,
				_collision_circle_capsule<false, true, false>,
				_collision_circle_convex_polygon<false, true, false> },
		{ 0,
				0,
				_collision_rectangle_rectangle<false, true, false>,
				_collision_rectangle_capsule<false, true, false>,
				_collision_rectangle_convex_polygon<false, true, false> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<false, true, false>,
				_collision_capsule_convex_polygon<false, true, false> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<false, true, false> }

	};

	static const CollisionFunc collision_table_castA_castB[5][5] = {
		{ _collision_segment_segment<true, true, false>,
				_collision_segment_circle<true, true, false>,
				_collision_segment_rectangle<true, true, false>,
				_collision_segment_capsule<true, true, false>,
				_collision_segment_convex_polygon<true, true, false> },
		{ 0,
				_collision_circle_circle<true, true, false>,
				_collision_circle_rectangle<true, true, false>,
				_collision_circle_capsule<true, true, false>,
				_collision_circle_convex_polygon<true, true, false> },
		{ 0,
				0,
				_collision_rectangle_rectangle<true, true, false>,
				_collision_rectangle_capsule<true, true, false>,
				_collision_rectangle_convex_polygon<true, true, false> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<true, true, false>,
				_collision_capsule_convex_polygon<true, true, false> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<true, true, false> }

	};

	static const CollisionFunc collision_table_margin[5][5] = {
		{ _collision_segment_segment<false, false, true>,
				_collision_segment_circle<false, false, true>,
				_collision_segment_rectangle<false, false, true>,
				_collision_segment_capsule<false, false, true>,
				_collision_segment_convex_polygon<false, false, true> },
		{ 0,
				_collision_circle_circle<false, false, true>,
				_collision_circle_rectangle<false, false, true>,
				_collision_circle_capsule<false, false, true>,
				_collision_circle_convex_polygon<false, false, true> },
		{ 0,
				0,
				_collision_rectangle_rectangle<false, false, true>,
				_collision_rectangle_capsule<false, false, true>,
				_collision_rectangle_convex_polygon<false, false, true> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<false, false, true>,
				_collision_capsule_convex_polygon<false, false, true> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<false, false, true> }

	};

	static const CollisionFunc collision_table_castA_margin[5][5] = {
		{ _collision_segment_segment<true, false, true>,
				_collision_segment_circle<true, false, true>,
				_collision_segment_rectangle<true, false, true>,
				_collision_segment_capsule<true, false, true>,
				_collision_segment_convex_polygon<true, false, true> },
		{ 0,
				_collision_circle_circle<true, false, true>,
				_collision_circle_rectangle<true, false, true>,
				_collision_circle_capsule<true, false, true>,
				_collision_circle_convex_polygon<true, false, true> },
		{ 0,
				0,
				_collision_rectangle_rectangle<true, false, true>,
				_collision_rectangle_capsule<true, false, true>,
				_collision_rectangle_convex_polygon<true, false, true> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<true, false, true>,
				_collision_capsule_convex_polygon<true, false, true> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<true, false, true> }

	};

	static const CollisionFunc collision_table_castB_margin[5][5] = {
		{ _collision_segment_segment<false, true, true>,
				_collision_segment_circle<false, true, true>,
				_collision_segment_rectangle<false, true, true>,
				_collision_segment_capsule<false, true, true>,
				_collision_segment_convex_polygon<false, true, true> },
		{ 0,
				_collision_circle_circle<false, true, true>,
				_collision_circle_rectangle<false, true, true>,
				_collision_circle_capsule<false, true, true>,
				_collision_circle_convex_polygon<false, true, true> },
		{ 0,
				0,
				_collision_rectangle_rectangle<false, true, true>,
				_collision_rectangle_capsule<false, true, true>,
				_collision_rectangle_convex_polygon<false, true, true> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<false, true, true>,
				_collision_capsule_convex_polygon<false, true, true> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<false, true, true> }

	};

	static const CollisionFunc collision_table_castA_castB_margin[5][5] = {
		{ _collision_segment_segment<true, true, true>,
				_collision_segment_circle<true, true, true>,
				_collision_segment_rectangle<true, true, true>,
				_collision_segment_capsule<true, true, true>,
				_collision_segment_convex_polygon<true, true, true> },
		{ 0,
				_collision_circle_circle<true, true, true>,
				_collision_circle_rectangle<true, true, true>,
				_collision_circle_capsule<true, true, true>,
				_collision_circle_convex_polygon<true, true, true> },
		{ 0,
				0,
				_collision_rectangle_rectangle<true, true, true>,
				_collision_rectangle_capsule<true, true, true>,
				_collision_rectangle_convex_polygon<true, true, true> },
		{ 0,
				0,
				0,
				_collision_capsule_capsule<true, true, true>,
				_collision_capsule_convex_polygon<true, true, true> },
		{ 0,
				0,
				0,
				0,
				_collision_convex_polygon_convex_polygon<true, true, true> }

	};

	_CollectorCallback2D callback;
	callback.callback = p_result_callback;
	callback.swap = p_swap;
	callback.userdata = p_userdata;
	callback.collided = false;
	callback.sep_axis = sep_axis;

	const Shape2DSW *A = p_shape_A;
	const Shape2DSW *B = p_shape_B;
	const Transform2D *transform_A = &p_transform_A;
	const Transform2D *transform_B = &p_transform_B;
	const Vector2 *motion_A = &p_motion_A;
	const Vector2 *motion_B = &p_motion_B;
	real_t margin_A = p_margin_A, margin_B = p_margin_B;

	if (type_A > type_B) {
		SWAP(A, B);
		SWAP(transform_A, transform_B);
		SWAP(type_A, type_B);
		SWAP(motion_A, motion_B);
		SWAP(margin_A, margin_B);
		callback.swap = !callback.swap;
	}

	CollisionFunc collision_func;

	if (p_margin_A || p_margin_B) {
		if (*motion_A == Vector2() && *motion_B == Vector2()) {
			collision_func = collision_table_margin[type_A - 2][type_B - 2];
		} else if (*motion_A != Vector2() && *motion_B == Vector2()) {
			collision_func = collision_table_castA_margin[type_A - 2][type_B - 2];
		} else if (*motion_A == Vector2() && *motion_B != Vector2()) {
			collision_func = collision_table_castB_margin[type_A - 2][type_B - 2];
		} else {
			collision_func = collision_table_castA_castB_margin[type_A - 2][type_B - 2];
		}
	} else {
		if (*motion_A == Vector2() && *motion_B == Vector2()) {
			collision_func = collision_table[type_A - 2][type_B - 2];
		} else if (*motion_A != Vector2() && *motion_B == Vector2()) {
			collision_func = collision_table_castA[type_A - 2][type_B - 2];
		} else if (*motion_A == Vector2() && *motion_B != Vector2()) {
			collision_func = collision_table_castB[type_A - 2][type_B - 2];
		} else {
			collision_func = collision_table_castA_castB[type_A - 2][type_B - 2];
		}
	}

	ERR_FAIL_COND_V(!collision_func, false);

	collision_func(A, *transform_A, B, *transform_B, &callback, *motion_A, *motion_B, margin_A, margin_B);

	return callback.collided;
}
