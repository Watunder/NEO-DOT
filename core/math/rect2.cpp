/*************************************************************************/
/*  rect2.cpp                                                            */
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

#include "core/math/transform_2d.h" // Includes rect2.h but Rect2 needs Transform2D

bool Rect2::is_equal_approx(const Rect2 &p_rect) const {
	return position.is_equal_approx(p_rect.position) && size.is_equal_approx(p_rect.size);
}

bool Rect2::intersects_segment(const Point2 &p_from, const Point2 &p_to, Point2 *r_pos, Point2 *r_normal) const {
	real_t min = 0, max = 1;
	int axis = 0;
	real_t sign = 0;

	for (int i = 0; i < 2; i++) {
		real_t seg_from = p_from[i];
		real_t seg_to = p_to[i];
		real_t box_begin = position[i];
		real_t box_end = box_begin + size[i];
		real_t cmin, cmax;
		real_t csign;

		if (seg_from < seg_to) {
			if (seg_from > box_end || seg_to < box_begin)
				return false;
			real_t length = seg_to - seg_from;
			cmin = (seg_from < box_begin) ? ((box_begin - seg_from) / length) : 0;
			cmax = (seg_to > box_end) ? ((box_end - seg_from) / length) : 1;
			csign = -1.0;

		} else {
			if (seg_to > box_end || seg_from < box_begin)
				return false;
			real_t length = seg_to - seg_from;
			cmin = (seg_from > box_end) ? (box_end - seg_from) / length : 0;
			cmax = (seg_to < box_begin) ? (box_begin - seg_from) / length : 1;
			csign = 1.0;
		}

		if (cmin > min) {
			min = cmin;
			axis = i;
			sign = csign;
		}
		if (cmax < max)
			max = cmax;
		if (max < min)
			return false;
	}

	Vector2 rel = p_to - p_from;

	if (r_normal) {
		Vector2 normal;
		normal[axis] = sign;
		*r_normal = normal;
	}

	if (r_pos)
		*r_pos = p_from + rel * min;

	return true;
}

bool Rect2::intersects_transformed(const Transform2D &p_xform, const Rect2 &p_rect) const {
	//SAT intersection between local and transformed rect2

	Vector2 xf_points[4] = {
		p_xform.xform(p_rect.position),
		p_xform.xform(Vector2(p_rect.position.x + p_rect.size.x, p_rect.position.y)),
		p_xform.xform(Vector2(p_rect.position.x, p_rect.position.y + p_rect.size.y)),
		p_xform.xform(Vector2(p_rect.position.x + p_rect.size.x, p_rect.position.y + p_rect.size.y)),
	};

	real_t low_limit;

	//base rect2 first (faster)

	if (xf_points[0].y > position.y)
		goto next1;
	if (xf_points[1].y > position.y)
		goto next1;
	if (xf_points[2].y > position.y)
		goto next1;
	if (xf_points[3].y > position.y)
		goto next1;

	return false;

next1:

	low_limit = position.y + size.y;

	if (xf_points[0].y < low_limit)
		goto next2;
	if (xf_points[1].y < low_limit)
		goto next2;
	if (xf_points[2].y < low_limit)
		goto next2;
	if (xf_points[3].y < low_limit)
		goto next2;

	return false;

next2:

	if (xf_points[0].x > position.x)
		goto next3;
	if (xf_points[1].x > position.x)
		goto next3;
	if (xf_points[2].x > position.x)
		goto next3;
	if (xf_points[3].x > position.x)
		goto next3;

	return false;

next3:

	low_limit = position.x + size.x;

	if (xf_points[0].x < low_limit)
		goto next4;
	if (xf_points[1].x < low_limit)
		goto next4;
	if (xf_points[2].x < low_limit)
		goto next4;
	if (xf_points[3].x < low_limit)
		goto next4;

	return false;

next4:

	Vector2 xf_points2[4] = {
		position,
		Vector2(position.x + size.x, position.y),
		Vector2(position.x, position.y + size.y),
		Vector2(position.x + size.x, position.y + size.y),
	};

	real_t maxa = p_xform.elements[0].dot(xf_points2[0]);
	real_t mina = maxa;

	real_t dp = p_xform.elements[0].dot(xf_points2[1]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[0].dot(xf_points2[2]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[0].dot(xf_points2[3]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	real_t maxb = p_xform.elements[0].dot(xf_points[0]);
	real_t minb = maxb;

	dp = p_xform.elements[0].dot(xf_points[1]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[0].dot(xf_points[2]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[0].dot(xf_points[3]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	if (mina > maxb)
		return false;
	if (minb > maxa)
		return false;

	maxa = p_xform.elements[1].dot(xf_points2[0]);
	mina = maxa;

	dp = p_xform.elements[1].dot(xf_points2[1]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[1].dot(xf_points2[2]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	dp = p_xform.elements[1].dot(xf_points2[3]);
	maxa = MAX(dp, maxa);
	mina = MIN(dp, mina);

	maxb = p_xform.elements[1].dot(xf_points[0]);
	minb = maxb;

	dp = p_xform.elements[1].dot(xf_points[1]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[1].dot(xf_points[2]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	dp = p_xform.elements[1].dot(xf_points[3]);
	maxb = MAX(dp, maxb);
	minb = MIN(dp, minb);

	if (mina > maxb)
		return false;
	if (minb > maxa)
		return false;

	return true;
}
