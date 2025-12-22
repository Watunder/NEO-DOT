/*************************************************************************/
/*  freetype_wrapper.h                                                   */
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

#ifndef FREETYPE_WRAPPER_H
#define FREETYPE_WRAPPER_H

#include "configs/modules_enabled.gen.h"
#ifdef MODULE_FREETYPE_ENABLED

#include "core/hash_map.h"
#include "core/pool_vector.h"

#include <ft2build.h>
#include FT_CACHE_H
#include FT_FREETYPE_H

class FreeTypeWrapper {
public:
	struct FontID {
		uint32_t font_hash = 0;
		uint32_t font_face_index = 0;
	};

private:
	FT_Library ft_library;
	FTC_Manager ftc_manager;

	mutable HashMap<uint32_t, FontID *> font_id_map;
	HashMap<uint32_t, PoolVector<uint8_t>> font_buffer_map;

	FontID *_get_font_id(uint32_t p_font_hash, uint32_t p_font_face_index) const;

public:
	friend FT_Error _ftc_manager_requester(FTC_FaceID p_font_id, FT_Library p_library, FT_Pointer p_request_data, FT_Face *r_face);

	uint32_t store_buffer(const PoolVector<uint8_t> &p_font_buffer);
	FT_Face lookup_face(uint32_t p_font_hash, uint32_t p_font_face_index);
	FT_Size lookup_size(uint32_t p_font_hash, uint32_t p_font_face_index, int p_size, float p_oversampling);

	FreeTypeWrapper();
	~FreeTypeWrapper();
};

#endif

#endif
