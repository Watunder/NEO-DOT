/*************************************************************************/
/*  shelf_pack_texture.h                                                 */
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

#ifndef SHELF_PACK_TEXTURE_H
#define SHELF_PACK_TEXTURE_H

#include "scene/resources/texture.h"

struct ShelfPackTexture {
	enum {
		MIN_TEXTURE_SIZE = 256,
		MAX_TEXTURE_SIZE = 4096,
	};

	struct Position {
		int index = -1;
		int x = 0;
		int y = 0;
	};

	struct Shelf {
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;

		Position alloc_shelf(int p_index, int p_w, int p_h);
	};

	List<Shelf> shelves;
	bool dirty = true;

	int texture_size = MIN_TEXTURE_SIZE;
	PoolVector<uint8_t> image_data;
	Ref<ImageTexture> texture;
	Image::Format image_format;

	Position pack_rect(int p_index, int p_w, int p_h);
};

#endif
