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

FreeTypeWrapper::FontID *FreeTypeWrapper::_get_font_id(uint32_t p_font_hash, uint32_t p_font_face_index) const {
	uint32_t key = HashMapHasherDefault::hash(((uint64_t)p_font_hash << 32) | p_font_face_index);
	if (font_id_map.has(key)) {
		return font_id_map.get(key);
	}

	FontID *font_id = memnew(FontID);
	font_id->font_hash = p_font_hash;
	font_id->font_face_index = p_font_face_index;

	font_id_map[key] = font_id;
	return font_id;
}

static _FORCE_INLINE_ FT_Error _ftc_manager_requester(FTC_FaceID p_font_id, FT_Library p_library, FT_Pointer p_request_data, FT_Face *r_face) {
	FreeTypeWrapper::FontID *font_id = (FreeTypeWrapper::FontID *)p_font_id;

	FreeTypeWrapper *freetype_wrapper = static_cast<FreeTypeWrapper *>(p_request_data);

	PoolVector<uint8_t> buffer;
	if (freetype_wrapper->font_buffer_map.has(font_id->font_hash)) {
		buffer = freetype_wrapper->font_buffer_map[font_id->font_hash];
	}

	ERR_FAIL_COND_V(buffer.empty(), FT_Err_Invalid_File_Format);

	FT_Error error = FT_New_Memory_Face(p_library, buffer.read().ptr(), buffer.size(), font_id->font_face_index, r_face);

	return error;
}

FT_Face FreeTypeWrapper::lookup_face(uint32_t p_font_hash, uint32_t p_font_face_index) {
	FontID *font_id = _get_font_id(p_font_hash, p_font_face_index);

	FT_Face ft_face = NULL;
	FT_Error error = FTC_Manager_LookupFace(ftc_manager, (FTC_FaceID)font_id, &ft_face);

	ERR_FAIL_COND_V_MSG(error, ft_face, FT_Error_String(error));

	return ft_face;
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

	FTC_ScalerRec scaler;
	scaler.face_id = (FTC_FaceID)font_id;
	scaler.width = p_size * 64.0 * p_oversampling;
	scaler.height = p_size * 64.0 * p_oversampling;
	scaler.pixel = 0;
	scaler.x_res = 0;
	scaler.y_res = 0;

	FT_Size ft_size = NULL;
	FT_Error error = FTC_Manager_LookupSize(ftc_manager, &scaler, &ft_size);
	ERR_FAIL_COND_V_MSG(error, ft_size, FT_Error_String(error));

	return ft_size;
}

FreeTypeWrapper::FreeTypeWrapper() {
	FT_Error error = FT_Init_FreeType(&ft_library);

	if (!error) {
		error = FTC_Manager_New(ft_library, 16, 32, 4 * 1024 * 1024, &_ftc_manager_requester, (FT_Pointer)this, &ftc_manager);
	}

	if (error) {
		ERR_PRINT("[FreeType] Failed to initialize: '" + String(FT_Error_String(error)) + "'.");
	}
}

FreeTypeWrapper::~FreeTypeWrapper() {
	if (ftc_manager) {
		FTC_Manager_Done(ftc_manager);
	}
	if (ft_library) {
		FT_Done_FreeType(ft_library);
	}

	const uint32_t *k = NULL;
	while ((k = font_id_map.next(k))) {
		memdelete(font_id_map[*k]);
	}
}

#endif
