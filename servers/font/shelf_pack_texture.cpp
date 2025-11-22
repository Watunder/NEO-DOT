/*************************************************************************/
/*  shelf_pack_texture.cpp                                               */
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

#include "shelf_pack_texture.h"

ShelfPackTexture::Position ShelfPackTexture::Shelf::alloc_shelf(int p_index, int p_w, int p_h) {
	if (p_w > w || p_h > h) {
		return ShelfPackTexture::Position{};
	}
	int xx = x;
	x += p_w;
	w -= p_w;
	return ShelfPackTexture::Position{ p_index, xx, y };
}

ShelfPackTexture::Position ShelfPackTexture::pack_rect(int p_index, int p_w, int p_h) {
	int y = 0;
	int waste = 0;
	List<Shelf>::Element *best_shelf = nullptr;
	int best_waste = INT_MAX;

	for (List<Shelf>::Element *E = shelves.front(); E; E = E->next()) {
		y += E->get().h;
		if (p_w > E->get().w) {
			continue;
		}
		if (p_h == E->get().h) {
			return E->get().alloc_shelf(p_index, p_w, p_h);
		}
		if (p_h > E->get().h) {
			continue;
		}
		if (p_h < E->get().h) {
			waste = (E->get().h - p_h) * p_w;
			if (waste < best_waste) {
				best_waste = waste;
				best_shelf = E;
			}
		}
	}
	if (best_shelf) {
		return best_shelf->get().alloc_shelf(p_index, p_w, p_h);
	}
	if (p_h <= (texture_size - y) && p_w <= texture_size) {
		List<Shelf>::Element *E = shelves.push_back(Shelf{ 0, y, texture_size, p_h });
		return E->get().alloc_shelf(p_index, p_w, p_h);
	}
	return ShelfPackTexture::Position{};
}
