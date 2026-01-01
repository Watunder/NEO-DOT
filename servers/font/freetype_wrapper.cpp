/*************************************************************************/
/*  freetype_wrapper.cpp                                                 */
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

#include "freetype_wrapper.h"

#ifdef MODULE_FREETYPE_ENABLED

#include "thirdparty/zstd/common/xxhash.h"

FreeTypeWrapper::FontID *FreeTypeWrapper::_get_font_id(uint32_t p_font_hash, uint32_t p_font_face_index) {
	uint64_t key = ((uint64_t)p_font_hash << 32) | p_font_face_index;
	if (font_id_map.has(key)) {
		return font_id_map[key];
	}

	FontID *font_id = memnew(FontID);
	font_id->font_hash = p_font_hash;
	font_id->font_face_index = p_font_face_index;

	font_id_map[key] = font_id;
	return font_id;
}

FT_Face FreeTypeWrapper::lookup_face(uint32_t p_font_hash, uint32_t p_font_face_index) {
	FontID *font_id = _get_font_id(p_font_hash, p_font_face_index);

	FT_Face ft_face = NULL;

	if (font_id_to_face.has(font_id)) {
		ft_face = font_id_to_face[font_id];
		if (ft_face) {
			return ft_face;
		}
	}

	PoolVector<uint8_t> buffer;
	if (font_buffer_map.has(font_id->font_hash)) {
		buffer = font_buffer_map[font_id->font_hash];
	}

	ERR_FAIL_COND_V_MSG(buffer.empty(), ft_face, FT_Error_String(FT_Err_Invalid_File_Format));

	FT_Error error = FT_New_Memory_Face(ft_library, buffer.read().ptr(), buffer.size(), font_id->font_face_index, &ft_face);
	if (!error) {
		face_to_font_id[ft_face] = font_id;
		font_id_to_face[font_id] = ft_face;
	}

	ERR_FAIL_COND_V_MSG(error, ft_face, FT_Error_String(error));

	return ft_face;
}

FreeTypeWrapper::FontID *FreeTypeWrapper::get_font_id(const FT_Face &p_ft_face) const {
	if (face_to_font_id.has(p_ft_face)) {
		return face_to_font_id[p_ft_face];
	}

	return NULL;
}

uint32_t FreeTypeWrapper::store_buffer(const PoolVector<uint8_t> &p_font_buffer) {
	uint32_t font_hash = XXH32(p_font_buffer.read().ptr(), p_font_buffer.size(), 0);

	if (!font_buffer_map.has(font_hash)) {
		font_buffer_map[font_hash] = p_font_buffer;
	}

	return font_hash;
}

FT_Size FreeTypeWrapper::lookup_size(uint32_t p_font_hash, uint32_t p_font_face_index, int p_size, float p_oversampling) {
	FontID *font_id = _get_font_id(p_font_hash, p_font_face_index);

	FT_Size ft_size = NULL;
	FT_Face ft_face = NULL;

	if (font_id_to_face.has(font_id)) {
		ft_face = font_id_to_face[font_id];
		if (ft_face) {
			ft_size = ft_face->size;
		}
	}

	FT_Error error = FT_Err_Ok;

	if (FT_HAS_COLOR(ft_face) && ft_face->num_fixed_sizes > 0) {
		int best_match = 0;
		int diff = ABS(p_size - ((int64_t)ft_face->available_sizes[0].width));
		for (int i = 1; i < ft_face->num_fixed_sizes; i++) {
			int ndiff = ABS(p_size - ((int64_t)ft_face->available_sizes[i].width));
			if (ndiff < diff) {
				best_match = i;
				diff = ndiff;
			}
		}

		error = FT_Select_Size(ft_face, best_match);
	} else {
		FT_Size_RequestRec req;
		req.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
		req.width = p_size * 64.0 * p_oversampling;
		req.height = p_size * 64.0 * p_oversampling;
		req.horiResolution = 0;
		req.vertResolution = 0;

		error = FT_Request_Size(ft_face, &req);
	}

	ERR_FAIL_COND_V_MSG(error, ft_size, FT_Error_String(error));

	return ft_size;
}

FreeTypeWrapper::FreeTypeWrapper() {
	FT_Error error = FT_Init_FreeType(&ft_library);
	if (error) {
		ERR_PRINT("[FreeType] Failed to initialize: '" + String(FT_Error_String(error)) + "'.");
	}
}

FreeTypeWrapper::~FreeTypeWrapper() {
	if (ft_library) {
		FT_Done_FreeType(ft_library);
	}

	const uint64_t *k = NULL;
	while ((k = font_id_map.next(k))) {
		memdelete(font_id_map[*k]);
	}
}

#endif
