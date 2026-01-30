/*************************************************************************/
/*  font_driver_bmfont.h                                                 */
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

#ifndef FONT_DRIVER_BMFONT_H
#define FONT_DRIVER_BMFONT_H

#include "core/hash_map.h"
#include "core/image.h"
#include "core/math/vector2.h"
#include "core/reference.h"
#include "core/rid.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "servers/font_server.h"

struct BMFontChar {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int xoffset = 0;
	int yoffset = 0;
	int xadvance = 0;
	int page = 0;
};

struct BMFontKerning {
	int first = 0;
	int second = 0;
	int amount = 0;
};

class BMFontData : public Reference {
public:
	int line_height = 0;
	int base = 0;
	int scale_w = 0;
	int scale_h = 0;
	int pages_count = 0;

	Vector<String> page_files;
	HashMap<uint32_t, BMFontChar> chars;
	HashMap<uint32_t, int> kernings;

	static Ref<BMFontData> load_from_buffer(const PoolVector<uint8_t> &p_data, const String &p_base_path, Error *r_error = NULL);
};

/*************************************************************************/

struct BMFontInstance {
	Ref<BMFontData> data;
	Vector<Ref<Image>> page_images;
};

class FontDriverBMFont : public FontDriver {
	HashMap<FontID, BMFontInstance, FontIDHasher> fonts;
	HashMap<GlyphCacheKey, Vector<RID>, GlyphCacheKeyHasher> texture_map;
	HashMap<GlyphCacheKey, HashMap<uint32_t, GlyphInfo>, GlyphCacheKeyHasher> glyph_info_map;

	Vector<FontID> builtin_font_ids;

	_FORCE_INLINE_ void _setup_builtin_fonts();

public:
	virtual Error init();

	virtual const char *get_name() const { return "BMFont"; }

	Error load_font_data(FontID &r_font_id, const PoolVector<uint8_t> &p_font_data, const Vector<PoolVector<uint8_t>> &p_page_data);
	virtual Error load_font_data(FontID &r_font_id, const PoolVector<uint8_t> &p_font_data);
	virtual Error load_font_file(FontID &r_font_id, const String &p_font_path);

	virtual Vector<FontID> get_builtin_font_ids() const;
	virtual Ref<FontInfo> get_font_info(const FontID &p_font_id) const;

	virtual bool owns_font(const FontID &p_font_id) const;
	virtual bool validate_font(const FontID &p_font_id) const;

	virtual uint32_t get_glyph_index(const FontID &p_font_id, char32_t p_char) const;
	virtual bool get_font_metrics(float &r_ascent, float &r_descent, const FontID &p_font_id, int p_size, int p_oversampling) const;
	virtual Vector2 get_font_kerning(const FontID &p_font_id, char32_t p_char, char32_t p_next_char, int p_size, int p_oversampling) const;

	virtual void clear_glyph_cache(const GlyphCacheKey &p_cache_key);
	virtual GlyphInfo get_glyph_info(const GlyphCacheKey &p_cache_key, uint32_t p_glyph_index);
	virtual RID get_glyph_texture_rid(const GlyphInfo &p_glyph_info);

	FontDriverBMFont();
	virtual ~FontDriverBMFont();
};

#endif
