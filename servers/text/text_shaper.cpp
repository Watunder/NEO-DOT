/*************************************************************************/
/*  text_shaper.cpp                                                      */
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

#include "text_shaper.h"

#include "core/error_macros.h"

TextShaper *TextShaper::singleton = NULL;

TextShaper *TextShaper::get_singleton() {
	return singleton;
}

void TextShaper::set_singleton() {
	singleton = this;
}

TextShaper *TextShaperManager::shapers[MAX_SHAPERS];
int TextShaperManager::shaper_count = 0;

void TextShaperManager::add_shaper(TextShaper *p_shaper) {
	ERR_FAIL_COND(shaper_count >= MAX_SHAPERS);
	shapers[shaper_count++] = p_shaper;
}

void TextShaperManager::initialize(int p_shaper) {
	int failed_shaper = -1;

	if (p_shaper >= 0 && p_shaper < shaper_count) {
		if (shapers[p_shaper]->init() == OK) {
			shapers[p_shaper]->set_singleton();
			return;
		} else {
			failed_shaper = p_shaper;
		}
	}

	for (int i = 0; i < shaper_count; i++) {
		if (i == failed_shaper) {
			continue;
		}

		if (shapers[i]->init() == OK) {
			shapers[i]->set_singleton();
			break;
		}
	}
}

int TextShaperManager::get_shaper_count() {
	return shaper_count;
}

TextShaper *TextShaperManager::get_shaper(int p_shaper) {
	ERR_FAIL_INDEX_V(p_shaper, shaper_count, NULL);
	return shapers[p_shaper];
}
