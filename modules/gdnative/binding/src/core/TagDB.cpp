/*************************************************************************/
/*  TagDB.cpp                                                            */
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

#include "TagDB.hpp"

#include <unordered_map>

#include <GodotGlobal.hpp>

namespace godot {

namespace _TagDB {

std::unordered_map<size_t, size_t> parent_to;

void register_type(size_t type_tag, size_t base_type_tag) {
	if (type_tag == base_type_tag) {
		return;
	}
	parent_to[type_tag] = base_type_tag;
}

bool is_type_known(size_t type_tag) {
	return parent_to.find(type_tag) != parent_to.end();
}

void register_global_type(const char *name, size_t type_tag, size_t base_type_tag) {
	godot::nativescript_1_1_api->godot_nativescript_set_global_type_tag(godot::_RegisterState::language_index, name, (const void *)type_tag);

	register_type(type_tag, base_type_tag);
}

bool is_type_compatible(size_t ask_tag, size_t have_tag) {
	if (have_tag == 0)
		return false;

	size_t tag = have_tag;

	while (tag != 0) {
		if (tag == ask_tag)
			return true;

		tag = parent_to[tag];
	}

	return false;
}

} // namespace _TagDB

} // namespace godot
