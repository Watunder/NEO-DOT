/*************************************************************************/
/*  string_builder.h                                                     */
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

#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include "core/ustring.h"

#include "core/vector.h"

class StringBuilder {
	uint32_t string_length;

	Vector<String> strings;
	Vector<const char *> c_strings;

	// -1 means it's a Godot String
	// a natural number means C string.
	Vector<int32_t> appended_strings;

public:
	StringBuilder &append(const String &p_string);
	StringBuilder &append(const char *p_cstring);

	_FORCE_INLINE_ StringBuilder &operator+(const String &p_string) {
		return append(p_string);
	}

	_FORCE_INLINE_ StringBuilder &operator+(const char *p_cstring) {
		return append(p_cstring);
	}

	_FORCE_INLINE_ void operator+=(const String &p_string) {
		append(p_string);
	}

	_FORCE_INLINE_ void operator+=(const char *p_cstring) {
		append(p_cstring);
	}

	_FORCE_INLINE_ int num_strings_appended() const {
		return appended_strings.size();
	}

	_FORCE_INLINE_ uint32_t get_string_length() const {
		return string_length;
	}

	String as_string() const;

	_FORCE_INLINE_ operator String() const {
		return as_string();
	}

	StringBuilder() {
		string_length = 0;
	}
};

#endif // STRING_BUILDER_H
