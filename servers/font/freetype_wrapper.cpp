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

static _FORCE_INLINE_ FT_Error _ftc_manager_requester(FTC_FaceID p_font_id, FT_Library p_library, FT_Pointer p_request_data, FT_Face *r_face) {
	FreeTypeWrapper::FontInfo *font_info = (FreeTypeWrapper::FontInfo *)p_font_id;
	ERR_FAIL_COND_V(font_info->data.empty(), FT_Err_Invalid_File_Format);

	FreeTypeWrapper *freetype_wrapper = static_cast<FreeTypeWrapper *>(p_request_data);
	CRASH_COND(!freetype_wrapper);

	FT_Error error = FT_New_Memory_Face(p_library, font_info->data.read().ptr(), font_info->data.size(), font_info->id.font_index, r_face);
	if (!error) {
		font_info->face_count = (*r_face)->num_faces;

		freetype_wrapper->face_to_font_info[*r_face] = Ref<FreeTypeWrapper::FontInfo>(font_info);
	}

	return error;
}

Ref<FreeTypeWrapper::FontInfo> FreeTypeWrapper::_get_font_info(const FontID &p_font_id) const {
	if (font_infos.has(p_font_id)) {
		return font_infos[p_font_id];
	}

	Ref<FontInfo> new_font_info;
	new_font_info.instance();
	new_font_info->id = p_font_id;

	font_infos[p_font_id] = new_font_info;

	return new_font_info;
}

void FreeTypeWrapper::update_font_data(const FontID &p_font_id, const PoolVector<uint8_t> &p_font_data) {
	Ref<FontInfo> font_info = _get_font_info(p_font_id);

	if (font_info->data.empty()) {
		font_info->data = p_font_data;
	}
}

FT_Face FreeTypeWrapper::get_ft_face(const FontID &p_font_id) {
	Ref<FontInfo> font_info = _get_font_info(p_font_id);

	FT_Face ft_face = NULL;
	FT_Error error = FTC_Manager_LookupFace(ftc_manager, (FTC_FaceID)font_info.ptr(), &ft_face);

	ERR_FAIL_COND_V_MSG(error, ft_face, FT_Error_String(error));

	return ft_face;
}

FT_Size FreeTypeWrapper::get_ft_size(const FontID &p_font_id, int p_size, float p_oversampling) {
	Ref<FontInfo> font_info = _get_font_info(p_font_id);

	FTC_ScalerRec scaler;
	scaler.face_id = (FTC_FaceID)font_info.ptr();
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

Ref<FreeTypeWrapper::FontInfo> FreeTypeWrapper::get_font_info(const FT_Face &p_ft_face) const {
	ERR_FAIL_COND_V(!face_to_font_info.has(p_ft_face), Ref<FontInfo>());

	return face_to_font_info[p_ft_face];
}

Ref<FreeTypeWrapper::FontInfo> FreeTypeWrapper::get_font_info(const FontID &p_font_id) const {
	ERR_FAIL_COND_V(!font_infos.has(p_font_id), Ref<FontInfo>());

	return font_infos[p_font_id];
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

	font_infos.clear();
}

#endif
