/*************************************************************************/
/*  font_driver_freetype.cpp                                             */
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

#include "font_driver_freetype.h"

#include "core/error_macros.h"
#include "core/hash_map.h"
#include "core/map.h"
#include "core/math/vector2.h"
#include "core/os/file_access.h"
#include "core/os/memory.h"
#include "scene/resources/freetype_font.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

#include "builtin_vector_fonts.gen.h"

#include "thirdparty/zstd/common/xxhash.h"

_FORCE_INLINE_ ShelfPackTexture::Position FontDriverFreeType::_find_texture_pos(const GlyphCacheKey &p_cache_key, int p_width, int p_height, int p_color_size, Image::Format p_image_format, int p_rect_range) {
	ShelfPackTexture::Position tex_pos{};

	Vector<ShelfPackTexture> *textures = texture_map.getptr(p_cache_key);
	if (textures) {
		ShelfPackTexture *glyph_texture = textures->ptrw();
		for (int i = 0; i < textures->size(); i++) {
			if (glyph_texture[i].image_format != p_image_format)
				continue;

			if (p_width > glyph_texture[i].texture_size || p_height > glyph_texture[i].texture_size)
				continue;

			tex_pos = glyph_texture[i].pack_rect(i, p_width, p_height);
			if (tex_pos.index != -1) {
				break;
			}
		}
	}

	if (tex_pos.index == -1) {
		int texture_size = MAX(p_cache_key.font_size * p_cache_key.font_oversampling * 8, ShelfPackTexture::MIN_TEXTURE_SIZE);

		if (p_width > texture_size)
			texture_size = p_width;
		if (p_height > texture_size)
			texture_size = p_height;

		texture_size = next_power_of_2(texture_size);
		texture_size = MIN(texture_size, ShelfPackTexture::MAX_TEXTURE_SIZE);

		ShelfPackTexture tex{};
		tex.image_format = p_image_format;
		tex.texture_size = texture_size;
		tex.image_data.resize(texture_size * texture_size * p_color_size);

		{
			PoolVector<uint8_t>::Write w = tex.image_data.write();
			ERR_FAIL_COND_V(texture_size * texture_size * p_color_size > tex.image_data.size(), tex_pos);

			if (p_color_size == 2) {
				for (int i = 0; i < texture_size * texture_size * p_color_size; i += 2) {
					w[i + 0] = 255;
					w[i + 1] = 0;
				}
			} else {
				for (int i = 0; i < texture_size * texture_size * p_color_size; i += 4) {
					w[i + 0] = 255;
					w[i + 1] = 255;
					w[i + 2] = 255;
					w[i + 3] = 0;
				}
			}
		}

		Vector<ShelfPackTexture> &textures = texture_map[p_cache_key];
		textures.push_back(tex);
		int texture_index = textures.size() - 1;
		tex_pos = textures.write[texture_index].pack_rect(texture_index, p_width, p_height);
	}

	return tex_pos;
}

_FORCE_INLINE_ GlyphInfo FontDriverFreeType::_rasterize_bitmap(const GlyphCacheKey &p_cache_key, const FT_Bitmap &p_bitmap, int p_rect_range) {
	GlyphInfo glyph_info{};

	int rect_range = p_rect_range;
	int w = p_bitmap.width;
	int h = p_bitmap.rows;
	int mw = w + rect_range * 2;
	int mh = h + rect_range * 2;

	int color_size = 2;
	Image::Format image_format = Image::FORMAT_LA8;

	switch (p_bitmap.pixel_mode) {
		case FT_PIXEL_MODE_MONO:
		case FT_PIXEL_MODE_GRAY: {
			color_size = 2;
			image_format = Image::FORMAT_LA8;
		} break;
		case FT_PIXEL_MODE_BGRA: {
			color_size = 4;
			image_format = Image::FORMAT_RGBA8;
		} break;
	}

	ERR_FAIL_COND_V(mw > ShelfPackTexture::MAX_TEXTURE_SIZE, glyph_info);
	ERR_FAIL_COND_V(mh > ShelfPackTexture::MAX_TEXTURE_SIZE, glyph_info);

	ShelfPackTexture::Position tex_pos = _find_texture_pos(p_cache_key, mw, mh, color_size, image_format, rect_range);
	ERR_FAIL_COND_V(tex_pos.index < 0, glyph_info);

	glyph_info.texture_index = tex_pos.index;
	Vector<ShelfPackTexture> *textures = texture_map.getptr(p_cache_key);
	ERR_FAIL_COND_V(!textures, glyph_info);
	ShelfPackTexture &tex = textures->write[tex_pos.index];
	tex.dirty = true;

	{
		PoolVector<uint8_t>::Write wr = tex.image_data.write();

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int ofs = ((i + tex_pos.y + rect_range) * tex.texture_size + j + tex_pos.x + rect_range) * color_size;
				ERR_FAIL_COND_V(ofs >= tex.image_data.size(), glyph_info);
				switch (p_bitmap.pixel_mode) {
					case FT_PIXEL_MODE_MONO: {
						int byte = i * p_bitmap.pitch + (j >> 3);
						int bit = 1 << (7 - (j % 8));
						wr[ofs + 0] = 255;
						wr[ofs + 1] = (p_bitmap.buffer[byte] & bit) ? 255 : 0;
					} break;
					case FT_PIXEL_MODE_GRAY:
						wr[ofs + 0] = 255;
						wr[ofs + 1] = p_bitmap.buffer[i * p_bitmap.pitch + j];
						break;
					case FT_PIXEL_MODE_BGRA: {
						int ofs_color = i * p_bitmap.pitch + (j << 2);
						wr[ofs + 2] = p_bitmap.buffer[ofs_color + 0];
						wr[ofs + 1] = p_bitmap.buffer[ofs_color + 1];
						wr[ofs + 0] = p_bitmap.buffer[ofs_color + 2];
						wr[ofs + 3] = p_bitmap.buffer[ofs_color + 3];
					} break;
					default:
						ERR_FAIL_V_MSG(glyph_info, "Font uses unsupported pixel format: " + itos(p_bitmap.pixel_mode) + ".");
						break;
				}
			}
		}
	}

	glyph_info.found = true;
	glyph_info.texture_format = image_format;
	glyph_info.texture_rect_uv = Rect2(tex_pos.x + rect_range, tex_pos.y + rect_range, w, h);
	glyph_info.texture_size = glyph_info.texture_rect_uv.size / p_cache_key.font_oversampling;
	if (p_cache_key.font_use_mipmaps) {
		glyph_info.texture_flags |= Texture::FLAG_MIPMAPS;
	}
	if (p_cache_key.font_use_filter) {
		glyph_info.texture_flags |= Texture::FLAG_FILTER;
	}

	return glyph_info;
}

static _FORCE_INLINE_ void _ft_face_finalizer(void *p_ft_face) {
	FT_Face ft_face = (FT_Face)p_ft_face;
	if (!ft_face || !ft_face->generic.data) {
		return;
	}

	FontDriverFreeType *driver = static_cast<FontDriverFreeType *>(ft_face->generic.data);
	CRASH_COND(!driver);

	Ref<FontInfo> font_info = driver->face_to_info[ft_face];
	ERR_FAIL_COND(!font_info.is_valid());

	driver->face_to_info.erase(ft_face);

	bool has_other_faces = false;
	for (Map<FT_Face, Ref<FontInfo>>::Element *E = driver->face_to_info.front(); E; E = E->next()) {
		if (E->get() == font_info) {
			has_other_faces = true;
			break;
		}
	}

	if (!font_info->path.empty() && !has_other_faces) {
		font_info->data = PoolVector<uint8_t>();
	}
}

static _FORCE_INLINE_ FT_Error _ftc_manager_requester(FTC_FaceID p_font_info_ptr, FT_Library p_library, FT_Pointer p_request_data, FT_Face *r_face) {
	FontInfo *font_info_ptr = (FontInfo *)p_font_info_ptr;

	if (font_info_ptr->data.empty() && !font_info_ptr->path.empty()) {
		Error err;
		FileAccess *f = FileAccess::open(font_info_ptr->path, FileAccess::READ, &err);
		if (f && err == OK) {
			int len = f->get_len();
			font_info_ptr->data.resize(len);
			PoolVector<uint8_t>::Write w = font_info_ptr->data.write();
			int r = f->get_buffer(w.ptr(), len);
			f->close();
			memdelete(f);
			if (r != len) {
				return FT_Err_Invalid_File_Format;
			}
		} else {
			if (f) {
				f->close();
				memdelete(f);
			}
			return FT_Err_Cannot_Open_Resource;
		}
	}

	ERR_FAIL_COND_V(font_info_ptr->data.empty(), FT_Err_Invalid_File_Format);

	FontDriverFreeType *driver = static_cast<FontDriverFreeType *>(p_request_data);
	CRASH_COND(!driver);

	FT_Error error = FT_New_Memory_Face(p_library, font_info_ptr->data.read().ptr(), font_info_ptr->data.size(), font_info_ptr->id.font_index, r_face);
	if (!error) {
		font_info_ptr->face_count = (*r_face)->num_faces;
		font_info_ptr->family_name = (*r_face)->family_name;
		font_info_ptr->style_name = (*r_face)->style_name;
		driver->face_to_info[*r_face] = Ref<FontInfo>(font_info_ptr);

		(*r_face)->generic.data = driver;
		(*r_face)->generic.finalizer = _ft_face_finalizer;
	}

	return error;
}

_FORCE_INLINE_ void FontDriverFreeType::_setup_builtin_fonts() {
	{
		PoolVector<uint8_t> NotoSansUI_Regular;
		NotoSansUI_Regular.resize(_font_NotoSansUI_Regular_size);
		copymem(NotoSansUI_Regular.write().ptr(), _font_NotoSansUI_Regular, _font_NotoSansUI_Regular_size);

		FontID font_id = add_font_data(NotoSansUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoEmoji_Regular;
		NotoEmoji_Regular.resize(_font_NotoEmoji_Regular_size);
		copymem(NotoEmoji_Regular.write().ptr(), _font_NotoEmoji_Regular, _font_NotoEmoji_Regular_size);

		FontID font_id = add_font_data(NotoEmoji_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> DroidSansFallback;
		DroidSansFallback.resize(_font_DroidSansFallback_size);
		copymem(DroidSansFallback.write().ptr(), _font_DroidSansFallback, _font_DroidSansFallback_size);

		FontID font_id = add_font_data(DroidSansFallback);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> DroidSansJapanese;
		DroidSansJapanese.resize(_font_DroidSansJapanese_size);
		copymem(DroidSansJapanese.write().ptr(), _font_DroidSansJapanese, _font_DroidSansJapanese_size);

		FontID font_id = add_font_data(DroidSansJapanese);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoNaskhArabicUI_Regular;
		NotoNaskhArabicUI_Regular.resize(_font_NotoNaskhArabicUI_Regular_size);
		copymem(NotoNaskhArabicUI_Regular.write().ptr(), _font_NotoNaskhArabicUI_Regular, _font_NotoNaskhArabicUI_Regular_size);

		FontID font_id = add_font_data(NotoNaskhArabicUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoSansHebrew_Regular;
		NotoSansHebrew_Regular.resize(_font_NotoSansHebrew_Regular_size);
		copymem(NotoSansHebrew_Regular.write().ptr(), _font_NotoSansHebrew_Regular, _font_NotoSansHebrew_Regular_size);

		FontID font_id = add_font_data(NotoSansHebrew_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoSansThaiUI_Regular;
		NotoSansThaiUI_Regular.resize(_font_NotoSansThaiUI_Regular_size);
		copymem(NotoSansThaiUI_Regular.write().ptr(), _font_NotoSansThaiUI_Regular, _font_NotoSansThaiUI_Regular_size);

		FontID font_id = add_font_data(NotoSansThaiUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
	{
		PoolVector<uint8_t> NotoSansDevanagariUI_Regular;
		NotoSansDevanagariUI_Regular.resize(_font_NotoSansDevanagariUI_Regular_size);
		copymem(NotoSansDevanagariUI_Regular.write().ptr(), _font_NotoSansDevanagariUI_Regular, _font_NotoSansDevanagariUI_Regular_size);

		FontID font_id = add_font_data(NotoSansDevanagariUI_Regular);
		builtin_font_ids.push_back(font_id);
	}
}

Error FontDriverFreeType::init() {
	FT_Error error = FT_Init_FreeType(&ft_library);
	if (!error) {
		error = FTC_Manager_New(ft_library, 16, 32, 20 * 1024 * 1024, &_ftc_manager_requester, (FT_Pointer)this, &ftc_manager);
	}
	if (error) {
		ERR_PRINT("[FreeType] Failed to initialize: '" + String(FT_Error_String(error)) + "'.");
	}

	_setup_builtin_fonts();

	return OK;
}

FontID FontDriverFreeType::add_font_data(const PoolVector<uint8_t> &p_font_data) {
	ERR_FAIL_COND_V(p_font_data.empty(), FontID{});

	FontID font_id;
	font_id.font_hash = XXH32(p_font_data.read().ptr(), p_font_data.size(), 0);

	Ref<FontInfo> *font_info = font_id_to_info.getptr(font_id);
	if (!font_info) {
		Ref<FontInfo> new_font_info;
		new_font_info.instance();
		new_font_info->id = font_id;
		new_font_info->data = p_font_data;

		font_id_to_info[font_id] = new_font_info;
	} else {
		if ((*font_info)->data.empty()) {
			(*font_info)->data = p_font_data;
		}
	}

	return font_id;
}

FontID FontDriverFreeType::add_font_path(const String &p_font_path) {
	ERR_FAIL_COND_V(p_font_path.empty(), FontID{});

	Error err;
	FileAccess *f = FileAccess::open(p_font_path, FileAccess::READ, &err);
	ERR_FAIL_COND_V_MSG(err != OK, FontID{}, "Cannot open font file '" + p_font_path + "'.");

	int len = f->get_len();
	PoolVector<uint8_t> font_data;
	font_data.resize(len);
	PoolVector<uint8_t>::Write w = font_data.write();
	int r = f->get_buffer(w.ptr(), len);
	f->close();
	memdelete(f);
	ERR_FAIL_COND_V_MSG(r != len, FontID{}, "Failed to read font file '" + p_font_path + "'.");

	FontID font_id;
	font_id.font_hash = XXH32(font_data.read().ptr(), font_data.size(), 0);

	Ref<FontInfo> *font_info = font_id_to_info.getptr(font_id);
	if (!font_info) {
		Ref<FontInfo> new_font_info;
		new_font_info.instance();
		new_font_info->id = font_id;
		new_font_info->data = font_data;
		new_font_info->path = p_font_path;

		font_id_to_info[font_id] = new_font_info;
	} else {
		if ((*font_info)->path.empty()) {
			(*font_info)->path = p_font_path;
		}
	}

	return font_id;
}

FT_Face FontDriverFreeType::get_ft_face(const FontID &p_font_id) const {
	Ref<FontInfo> *font_info = font_id_to_info.getptr(p_font_id);
	ERR_FAIL_COND_V(!font_info, NULL);

	FT_Face ft_face = NULL;
	FT_Error error = FTC_Manager_LookupFace(ftc_manager, (FTC_FaceID)font_info->ptr(), &ft_face);
	ERR_FAIL_COND_V_MSG(error, NULL, FT_Error_String(error));

	return ft_face;
}

FT_Size FontDriverFreeType::get_ft_size(const FontID &p_font_id, int p_size, int p_oversampling) const {
	Ref<FontInfo> *font_info = font_id_to_info.getptr(p_font_id);
	ERR_FAIL_COND_V(!font_info, NULL);

	FTC_ScalerRec scaler;
	scaler.face_id = (FTC_FaceID)font_info->ptr();
	scaler.width = p_size * 64.0 * p_oversampling;
	scaler.height = p_size * 64.0 * p_oversampling;
	scaler.pixel = 0;
	scaler.x_res = 0;
	scaler.y_res = 0;

	FT_Size ft_size = NULL;
	FT_Error error = FTC_Manager_LookupSize(ftc_manager, &scaler, &ft_size);
	ERR_FAIL_COND_V_MSG(error, NULL, FT_Error_String(error));

	return ft_size;
}

Vector<FontID> FontDriverFreeType::get_builtin_font_ids() const {
	return builtin_font_ids;
}

Ref<FontInfo> FontDriverFreeType::get_font_info(const FontID &p_font_id) const {
	ERR_FAIL_COND_V(!font_id_to_info.has(p_font_id), Ref<FontInfo>());

	return font_id_to_info[p_font_id];
}

Ref<FontInfo> FontDriverFreeType::get_font_info(const FT_Face &p_ft_face) const {
	ERR_FAIL_COND_V(!face_to_info.has(p_ft_face), Ref<FontInfo>());

	return face_to_info[p_ft_face];
}

void FontDriverFreeType::clear_glyph_cache(const GlyphCacheKey &p_cache_key) {
	if (glyph_info_map.has(p_cache_key)) {
		glyph_info_map.erase(p_cache_key);
	}

	if (texture_map.has(p_cache_key)) {
		texture_map.erase(p_cache_key);
	}
}

GlyphInfo FontDriverFreeType::get_glyph_info(const GlyphCacheKey &p_cache_key, uint32_t p_glyph_index) {
	GlyphInfo glyph_info{};

	FontID font_id = p_cache_key.get_font_id();

	FT_Face ft_face = get_ft_face(font_id);
	ERR_FAIL_COND_V(!ft_face, glyph_info);

	FT_Size ft_size = get_ft_size(font_id, p_cache_key.font_size, p_cache_key.font_oversampling);
	ERR_FAIL_COND_V(!ft_size, glyph_info);

	HashMap<uint32_t, GlyphInfo> *glyph_map = glyph_info_map.getptr(p_cache_key);
	if (glyph_map && glyph_map->has(p_glyph_index)) {
		return (*glyph_map)[p_glyph_index];
	}

	int load_flags = FT_HAS_COLOR(ft_face) ? FT_LOAD_COLOR : FT_LOAD_DEFAULT;
	switch (p_cache_key.font_custom_flags) {
		case FreeTypeFont::HINTING_NONE:
			load_flags |= FT_LOAD_NO_HINTING;
			break;
		case FreeTypeFont::HINTING_AUTO:
			load_flags |= FT_LOAD_FORCE_AUTOHINT;
			break;
		case FreeTypeFont::HINTING_LIGHT:
			load_flags |= FT_LOAD_TARGET_LIGHT;
			break;
		default:
			load_flags |= FT_LOAD_TARGET_NORMAL;
			break;
	}

	int error = FT_Load_Glyph(ft_face, p_glyph_index, load_flags);

	FT_GlyphSlot ft_glyph_slot = ft_face->glyph;
	if (!error) {
		error = FT_Render_Glyph(ft_glyph_slot, FT_RENDER_MODE_NORMAL);
	}
	if (!error) {
		glyph_info = _rasterize_bitmap(p_cache_key, ft_glyph_slot->bitmap);
		glyph_info.cache_key = p_cache_key;
		glyph_info.texture_offset = Vector2(ft_glyph_slot->bitmap_left, -ft_glyph_slot->bitmap_top) / p_cache_key.font_oversampling;
		glyph_info.advance = Vector2(ft_glyph_slot->advance.x / 64.0, ft_glyph_slot->advance.y / 64.0) / p_cache_key.font_oversampling;
	}

	if (glyph_info.found) {
		HashMap<uint32_t, GlyphInfo> &glyph_map = glyph_info_map[p_cache_key];
		glyph_map[p_glyph_index] = glyph_info;
	}

	return glyph_info;
}

RID FontDriverFreeType::get_glyph_texture_rid(const GlyphInfo &p_glyph_info) {
	Vector<ShelfPackTexture> *textures = texture_map.getptr(p_glyph_info.cache_key);
	ERR_FAIL_COND_V(!textures, RID());
	ERR_FAIL_INDEX_V(p_glyph_info.texture_index, textures->size(), RID());

	if ((*textures)[p_glyph_info.texture_index].dirty) {
		ShelfPackTexture &tex = textures->write[p_glyph_info.texture_index];
		tex.dirty = false;
		Ref<Image> img = memnew(Image(tex.texture_size, tex.texture_size, 0, tex.image_format, tex.image_data));
		if (!tex.texture_rid.is_valid()) {
			tex.texture_rid = VisualServer::get_singleton()->texture_create_from_image(img, Texture::FLAG_VIDEO_SURFACE | p_glyph_info.texture_flags);
		} else {
			VisualServer::get_singleton()->texture_set_data(tex.texture_rid, img);
		}
	}

	return (*textures)[p_glyph_info.texture_index].texture_rid;
}

uint32_t FontDriverFreeType::get_glyph_index(const FontID &p_font_id, char32_t p_char) const {
	FT_Face ft_face = get_ft_face(p_font_id);
	if (!ft_face) {
		return 0;
	}

	return FT_Get_Char_Index(ft_face, p_char);
}

bool FontDriverFreeType::get_font_metrics(const FontID &p_font_id, int p_size, int p_oversampling, float &r_ascent, float &r_descent) const {
	FT_Size ft_size = get_ft_size(p_font_id, p_size, p_oversampling);
	if (!ft_size) {
		return false;
	}

	r_ascent = (ft_size->metrics.ascender / 64.0) / p_oversampling;
	r_descent = (-ft_size->metrics.descender / 64.0) / p_oversampling;
	return true;
}

bool FontDriverFreeType::validate_font(const FontID &p_font_id) const {
	if (!font_id_to_info.has(p_font_id)) {
		Ref<FontInfo> *base_font_info = font_id_to_info.getptr(FontID{ p_font_id.font_hash, 0 });
		if (!base_font_info) {
			return false;
		}

		ERR_FAIL_INDEX_V(p_font_id.font_index, (*base_font_info)->face_count, false);

		Ref<FontInfo> new_font_info;
		new_font_info.instance();
		new_font_info->id = p_font_id;
		new_font_info->data = (*base_font_info)->data;
		new_font_info->path = (*base_font_info)->path;
		new_font_info->face_count = (*base_font_info)->face_count;
		new_font_info->family_name = (*base_font_info)->family_name;
		new_font_info->style_name = (*base_font_info)->style_name;

		font_id_to_info[p_font_id] = new_font_info;
	}

	FT_Face ft_face = get_ft_face(p_font_id);
	return (ft_face != NULL);
}

FontDriverFreeType::FontDriverFreeType() {
	ft_library = NULL;
	ftc_manager = NULL;
}

FontDriverFreeType::~FontDriverFreeType() {
	if (ftc_manager) {
		FTC_Manager_Done(ftc_manager);
	}
	if (ft_library) {
		FT_Done_FreeType(ft_library);
	}

	font_id_to_info.clear();
	face_to_info.clear();
}
