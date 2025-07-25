/*************************************************************************/
/*  disjoint_set.h                                                       */
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

#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include "core/map.h"
#include "core/vector.h"

/**
	@author Marios Staikopoulos <marios@staik.net>
*/

/* This DisjointSet class uses Find with path compression and Union by rank */
template <typename T, class C = Comparator<T>, class AL = DefaultAllocator>
class DisjointSet {
	struct Element {
		T object;
		Element *parent = nullptr;
		int rank = 0;
	};

	typedef Map<T, Element *, C, AL> MapT;

	MapT elements;

	Element *get_parent(Element *element);

	_FORCE_INLINE_ Element *insert_or_get(T object);

public:
	~DisjointSet();

	_FORCE_INLINE_ void insert(T object) { (void)insert_or_get(object); }

	void create_union(T a, T b);

	void get_representatives(Vector<T> &out_roots);

	void get_members(Vector<T> &out_members, T representative);
};

/* FUNCTIONS */

template <typename T, class C, class AL>
DisjointSet<T, C, AL>::~DisjointSet() {
	for (typename MapT::Element *itr = elements.front(); itr != nullptr; itr = itr->next()) {
		memdelete_allocator<Element, AL>(itr->value());
	}
}

template <typename T, class C, class AL>
typename DisjointSet<T, C, AL>::Element *DisjointSet<T, C, AL>::get_parent(Element *element) {
	if (element->parent != element) {
		element->parent = get_parent(element->parent);
	}

	return element->parent;
}

template <typename T, class C, class AL>
typename DisjointSet<T, C, AL>::Element *DisjointSet<T, C, AL>::insert_or_get(T object) {
	typename MapT::Element *itr = elements.find(object);
	if (itr != nullptr) {
		return itr->value();
	}

	Element *new_element = memnew_allocator(Element, AL);
	new_element->object = object;
	new_element->parent = new_element;
	elements.insert(object, new_element);

	return new_element;
}

template <typename T, class C, class AL>
void DisjointSet<T, C, AL>::create_union(T a, T b) {
	Element *x = insert_or_get(a);
	Element *y = insert_or_get(b);

	Element *x_root = get_parent(x);
	Element *y_root = get_parent(y);

	// Already in the same set
	if (x_root == y_root)
		return;

	// Not in the same set, merge
	if (x_root->rank < y_root->rank) {
		SWAP(x_root, y_root);
	}

	// Merge y_root into x_root
	y_root->parent = x_root;
	if (x_root->rank == y_root->rank) {
		++x_root->rank;
	}
}

template <typename T, class C, class AL>
void DisjointSet<T, C, AL>::get_representatives(Vector<T> &out_representatives) {
	for (typename MapT::Element *itr = elements.front(); itr != nullptr; itr = itr->next()) {
		Element *element = itr->value();
		if (element->parent == element) {
			out_representatives.push_back(element->object);
		}
	}
}

template <typename T, class C, class AL>
void DisjointSet<T, C, AL>::get_members(Vector<T> &out_members, T representative) {
	typename MapT::Element *rep_itr = elements.find(representative);
	ERR_FAIL_COND(rep_itr == nullptr);

	Element *rep_element = rep_itr->value();
	ERR_FAIL_COND(rep_element->parent != rep_element);

	for (typename MapT::Element *itr = elements.front(); itr != nullptr; itr = itr->next()) {
		Element *parent = get_parent(itr->value());
		if (parent == rep_element) {
			out_members.push_back(itr->key());
		}
	}
}

#endif
