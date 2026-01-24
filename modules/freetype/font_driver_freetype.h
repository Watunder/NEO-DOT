/*************************************************************************/
/*  font_driver_freetype.h                                               */
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

#ifndef FONT_DRIVER_FREETYPE_H
#define FONT_DRIVER_FREETYPE_H

#include "core/hash_map.h"
#include "core/map.h"
#include "core/pool_vector.h"
#include "core/reference.h"
#include "servers/font_server.h"

#include "shelf_pack_texture.h"

#include <ft2build.h>
#include FT_CACHE_H
#include FT_FREETYPE_H

class FontDriverFreeType : public FontDriver {
	FT_Library ft_library;
	FTC_Manager ftc_manager;

	Vector<FontID> builtin_font_ids;

	mutable HashMap<FontID, Ref<FontInfo>, FontIDHasher> font_id_to_info;
	Map<FT_Face, Ref<FontInfo>> face_to_info;

	HashMap<GlyphCacheKey, Vector<ShelfPackTexture>, GlyphCacheKeyHasher> texture_map;
	HashMap<GlyphCacheKey, HashMap<uint32_t, GlyphInfo>, GlyphCacheKeyHasher> glyph_info_map;

	_FORCE_INLINE_ ShelfPackTexture::Position _find_texture_pos(const GlyphCacheKey &p_cache_key, int p_width, int p_height, int p_color_size, Image::Format p_image_format, int p_rect_range);
	_FORCE_INLINE_ GlyphInfo _rasterize_bitmap(const GlyphCacheKey &p_cache_key, const FT_Bitmap &p_bitmap, int p_rect_range = 1);

	friend _FORCE_INLINE_ FT_Error _ftc_manager_requester(FTC_FaceID p_font_info_ptr, FT_Library p_library, FT_Pointer p_request_data, FT_Face *r_face);
	friend _FORCE_INLINE_ void _ft_face_finalizer(void *p_ft_face);

	_FORCE_INLINE_ void _setup_builtin_fonts();

public:
	virtual Error init();

	virtual const char *get_name() const { return "FreeType"; }

	virtual FontID add_font_data(const PoolVector<uint8_t> &p_font_data);
	virtual FontID add_font_path(const String &p_font_path);

	FT_Face get_ft_face(const FontID &p_font_id) const;
	FT_Size get_ft_size(const FontID &p_font_id, int p_size, int p_oversampling) const;

	virtual Vector<FontID> get_builtin_font_ids() const;
	virtual Ref<FontInfo> get_font_info(const FontID &p_font_id) const;
	Ref<FontInfo> get_font_info(const FT_Face &p_ft_face) const;

	virtual uint32_t get_glyph_index(const FontID &p_font_id, char32_t p_char) const;
	virtual bool get_font_metrics(const FontID &p_font_id, int p_size, int p_oversampling, float &r_ascent, float &r_descent) const;
	virtual bool validate_font(const FontID &p_font_id) const;

	virtual void clear_glyph_cache(const GlyphCacheKey &p_cache_key);
	virtual GlyphInfo get_glyph_info(const GlyphCacheKey &p_cache_key, uint32_t p_glyph_index);
	virtual RID get_glyph_texture_rid(const GlyphInfo &p_glyph_info);

	FontDriverFreeType();
	virtual ~FontDriverFreeType();
};

#endif
