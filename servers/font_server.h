/*************************************************************************/
/*  font_server.h                                                        */
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

#ifndef FONT_SERVER_H
#define FONT_SERVER_H

#include "core/image.h"
#include "core/math/vector2.h"
#include "core/pool_vector.h"
#include "core/reference.h"
#include "core/rid.h"

struct FontID {
	uint32_t font_hash = 0;
	uint16_t font_index = 0;

	_FORCE_INLINE_ bool operator==(const FontID &p_id) const {
		return (p_id.font_hash == font_hash &&
				p_id.font_index == font_index);
	}

	_FORCE_INLINE_ bool operator!=(const FontID &p_id) const {
		return (p_id.font_hash != font_hash ||
				p_id.font_index != font_index);
	}

	_FORCE_INLINE_ uint32_t hash() const {
		uint32_t h = font_hash;
		h = h * 31 + font_index;
		return h;
	}
};

struct FontIDHasher {
	static _FORCE_INLINE_ uint32_t hash(const FontID &p_id) {
		return HashMapHasherDefault::hash(p_id.hash());
	}
};

struct GlyphCacheKey {
	FontID font_id;
	uint16_t font_size = 0;
	uint8_t font_oversampling = 1;
	bool font_use_mipmaps = false;
	bool font_use_filter = false;
	uint32_t font_custom_flags = 0;

	_FORCE_INLINE_ bool operator==(const GlyphCacheKey &p_key) const {
		return (font_id == p_key.font_id &&
				font_size == p_key.font_size &&
				font_oversampling == p_key.font_oversampling &&
				font_use_mipmaps == p_key.font_use_mipmaps &&
				font_use_filter == p_key.font_use_filter &&
				font_custom_flags == p_key.font_custom_flags);
	}

	_FORCE_INLINE_ bool operator!=(const GlyphCacheKey &p_key) const {
		return (font_id != p_key.font_id ||
				font_size != p_key.font_size ||
				font_oversampling != p_key.font_oversampling ||
				font_use_mipmaps != p_key.font_use_mipmaps ||
				font_use_filter != p_key.font_use_filter ||
				font_custom_flags != p_key.font_custom_flags);
	}

	_FORCE_INLINE_ uint64_t hash() const {
		uint64_t h = font_id.hash();
		h = h * 31 + font_size;
		h = h * 31 + font_oversampling;
		h = h * 31 + (font_use_mipmaps ? 1 : 0);
		h = h * 31 + (font_use_filter ? 1 : 0);
		h = h * 31 + font_custom_flags;
		return h;
	}

	_FORCE_INLINE_ FontID get_font_id() const {
		return font_id;
	}

	_FORCE_INLINE_ GlyphCacheKey create_temp_key(const FontID &p_font_id) const {
		GlyphCacheKey temp_cache_key;
		temp_cache_key.font_id = p_font_id;
		temp_cache_key.font_size = font_size;
		temp_cache_key.font_oversampling = font_oversampling;
		temp_cache_key.font_use_mipmaps = font_use_mipmaps;
		temp_cache_key.font_use_filter = font_use_filter;
		temp_cache_key.font_custom_flags = font_custom_flags;
		return temp_cache_key;
	}
};

struct GlyphCacheKeyHasher {
	static _FORCE_INLINE_ uint32_t hash(const GlyphCacheKey &p_cache_key) {
		return HashMapHasherDefault::hash(p_cache_key.hash());
	}
};

/*************************************************************************/

struct FontInfo : Reference {
	FontID id;

	String family_name;
	String style_name;
	uint16_t face_count = 0;

	String path;
	PoolVector<uint8_t> data;
};

struct GlyphInfo {
	bool found = false;

	Vector2 texture_offset;
	Vector2 advance;

	int texture_index = -1;
	Size2 texture_size;
	Rect2 texture_rect_uv;
	Image::Format texture_format;

	uint32_t texture_flags = 0;

	GlyphCacheKey cache_key;
};

/*************************************************************************/

class FontDriver {
	static FontDriver *singleton;

public:
	virtual Error init() = 0;

	static FontDriver *get_singleton();
	void set_singleton();

	virtual const char *get_name() const = 0;

	virtual FontID add_font_data(const PoolVector<uint8_t> &p_font_data) = 0;
	virtual FontID add_font_path(const String &p_font_path) = 0;

	virtual Vector<FontID> get_builtin_font_ids() const = 0;
	virtual Ref<FontInfo> get_font_info(const FontID &p_font_id) const = 0;

	virtual uint32_t get_glyph_index(const FontID &p_font_id, char32_t p_char) const = 0;
	virtual bool get_font_metrics(const FontID &p_font_id, int p_size, int p_oversampling, float &r_ascent, float &r_descent) const = 0;
	virtual bool validate_font(const FontID &p_font_id) const = 0;

	virtual void clear_glyph_cache(const GlyphCacheKey &p_cache_key) = 0;
	virtual GlyphInfo get_glyph_info(const GlyphCacheKey &p_cache_key, uint32_t p_glyph_index) = 0;
	virtual RID get_glyph_texture_rid(const GlyphInfo &p_glyph_info) = 0;

	FontDriver() {}
	virtual ~FontDriver() {}
};

class FontDriverManager {
	enum {
		MAX_DRIVERS = 10
	};

	static FontDriver *drivers[MAX_DRIVERS];
	static int driver_count;

public:
	static void add_driver(FontDriver *p_driver);
	static void initialize(int p_driver);
	static int get_driver_count();
	static FontDriver *get_driver(int p_driver);
};

class FontServer : public Object {
	GDCLASS(FontServer, Object);

public:
	enum SpacingType {
		SPACING_TOP,
		SPACING_BOTTOM,
		SPACING_GLYPH,
		SPACING_SPACE_CHAR,
	};

	struct Font : RID_Data {
		GlyphCacheKey cache_key;
		Vector<GlyphCacheKey> temp_cache_keys;

		float ascent;
		float descent;

		int spacing_top;
		int spacing_bottom;
		int spacing_glyph;
		int spacing_space_char;

		Font() {
			ascent = 0;
			descent = 1;

			spacing_top = 0;
			spacing_bottom = 0;
			spacing_glyph = 0;
			spacing_space_char = 0;
		}
	};

	mutable RID_Owner<Font> font_owner;

private:
	static FontServer *singleton;

	_FORCE_INLINE_ void _font_clear_caches(Font *p_font);
	_FORCE_INLINE_ bool _font_update_metrics(Font *p_font);

protected:
	static void _bind_methods();

public:
	static FontServer *get_singleton();

	RID font_create(int p_size = 1, int p_custom_flags = 0, bool p_use_mipmaps = true, bool p_use_filter = true);
	void font_free(RID p_font);
	void font_set_size(RID p_font, int p_size);
	void font_set_use_mipmaps(RID p_font, bool p_use_mipmaps);
	void font_set_use_filter(RID p_font, bool p_use_filter);
	void font_set_custom_flags(RID p_font, int p_custom_flags);
	bool font_set_data(RID p_font, const PoolVector<uint8_t> &p_font_data);
	bool font_set_path(RID p_font, const String &p_font_path);
	bool font_set_builtin_data(RID p_font, int p_builtin_index = 0);
	bool font_set_index(RID p_font, int p_font_index);
	float font_get_ascent(RID p_font) const;
	float font_get_descent(RID p_font) const;
	int font_get_oversampling(RID p_font) const;
	GlyphCacheKey font_get_cache_key(RID p_font) const;
	PoolVector<uint8_t> font_get_data(RID p_font) const;

	void font_set_spacing(RID p_font, SpacingType p_spcing_type, int p_spacing);
	int font_get_spacing(RID p_font, SpacingType p_spcing_type) const;

	GlyphInfo font_get_glyph_info(RID p_font, char32_t p_char) const;
	RID font_get_glyph_texture_rid(RID p_font, const GlyphInfo &p_glyph_info) const;

	FontServer();
	~FontServer();
};

VARIANT_ENUM_CAST(FontServer::SpacingType);

#endif
