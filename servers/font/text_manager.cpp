/*************************************************************************/
/*  text_manager.cpp                                                     */
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

#include "text_manager.h"

#include <graphemebreak.h>

Vector<String> TextManager::get_graphemes(const String &p_text) const {
	Vector<String> graphemes;

	const uint32_t *text = (const uint32_t *)p_text.c_str();
	int text_len = p_text.length();

	Vector<char> breaks;
	breaks.resize(text_len + 1);

	set_graphemebreaks_utf32(text, text_len, "", breaks.ptrw());

	for (int i = 0; i < text_len;) {
		int start = i;
		int end = i;
		while (end < text_len && breaks[end] == GRAPHEMEBREAK_NOBREAK) {
			end++;
		}
		end++;
		i = end;
		int len = end - start;

		if (len > 0) {
			String grapheme = p_text.substr(start, len);
			graphemes.push_back(grapheme);
		}
	}

	return graphemes;
}
