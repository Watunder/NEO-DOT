/*************************************************************************/
/*  font_driver_bmfont.cpp                                               */
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

#include "font_driver_bmfont.h"

#include "builtin_bitmap_fonts.gen.h"

#include "core/error_macros.h"
#include "core/image.h"
#include "core/io/image_loader.h"
#include "core/io/xml_parser.h"
#include "core/os/copymem.h"
#include "core/os/file_access.h"
#include "core/os/os.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

#include "thirdparty/zstd/common/xxhash.h"

static _FORCE_INLINE_ bool _parse_key_value(const String &line, int &r_pos, String &r_key, String &r_value) {
	int len = line.length();
	while (r_pos < len && (line[r_pos] == ' ' || line[r_pos] == '\t')) {
		r_pos++;
	}
	if (r_pos >= len) {
		return false;
	}
	int key_start = r_pos;
	while (r_pos < len && line[r_pos] != '=') {
		r_pos++;
	}
	if (r_pos >= len) {
		return false;
	}
	r_key = line.substr(key_start, r_pos - key_start).strip_edges();
	r_pos++;
	if (r_pos < len && line[r_pos] == '"') {
		r_pos++;
		int val_start = r_pos;
		while (r_pos < len && line[r_pos] != '"') {
			r_pos++;
		}
		r_value = line.substr(val_start, r_pos - val_start);
		if (r_pos < len) {
			r_pos++;
		}
	} else {
		int val_start = r_pos;
		while (r_pos < len && line[r_pos] != ' ' && line[r_pos] != '\t') {
			r_pos++;
		}
		r_value = line.substr(val_start, r_pos - val_start).strip_edges();
	}
	return true;
}

static _FORCE_INLINE_ String _get_attr(const HashMap<String, String> &attrs, const String &p_key, const String &p_default) {
	const String *v = attrs.getptr(p_key);
	return v ? *v : p_default;
}

static _FORCE_INLINE_ bool _next_line(const char *src, int len, int &pos, String &line) {
	line = String();
	while (pos < len && (src[pos] == '\r' || src[pos] == '\n')) {
		pos++;
	}
	if (pos >= len) {
		return false;
	}
	int start = pos;
	while (pos < len && src[pos] != '\r' && src[pos] != '\n') {
		pos++;
	}
	line = String::utf8(&src[start], pos - start);
	return true;
}

static _FORCE_INLINE_ void _parse_line_attrs(const String &line, const char *tag, int tag_len, HashMap<String, String> &out) {
	out.clear();
	if (line.length() < (unsigned)tag_len || line.substr(0, tag_len) != String(tag)) {
		return;
	}
	int pos = tag_len;
	String key, value;
	while (_parse_key_value(line, pos, key, value)) {
		out[key] = value;
	}
}

static _FORCE_INLINE_ Ref<BMFontData> _load_bmfont_text(const PoolVector<uint8_t> &p_data, const String &p_base_path, Error *r_error) {
	Ref<BMFontData> bm_data;
	bm_data.instance();

	PoolVector<uint8_t>::Read r = p_data.read();
	const char *src = (const char *)r.ptr();
	int len = p_data.size();

	HashMap<String, String> attrs;
	String line;
	int pos = 0;

	while (_next_line(src, len, pos, line)) {
		if (line.empty()) {
			continue;
		}

		if (line.begins_with("common ")) {
			_parse_line_attrs(line, "common", 6, attrs);
			bm_data->line_height = _get_attr(attrs, "lineHeight", "0").to_int();
			bm_data->base = _get_attr(attrs, "base", "0").to_int();
			bm_data->scale_w = _get_attr(attrs, "scaleW", "0").to_int();
			bm_data->scale_h = _get_attr(attrs, "scaleH", "0").to_int();
			bm_data->pages_count = _get_attr(attrs, "pages", "0").to_int();
		} else if (line.begins_with("page ")) {
			_parse_line_attrs(line, "page", 4, attrs);
			String file = _get_attr(attrs, "file", "");
			if (!file.empty()) {
				bm_data->page_files.push_back(p_base_path + file);
			}
		} else if (line.begins_with("char ")) {
			_parse_line_attrs(line, "char", 4, attrs);
			BMFontChar ch;
			ch.x = _get_attr(attrs, "x", "0").to_int();
			ch.y = _get_attr(attrs, "y", "0").to_int();
			ch.width = _get_attr(attrs, "width", "0").to_int();
			ch.height = _get_attr(attrs, "height", "0").to_int();
			ch.xoffset = _get_attr(attrs, "xoffset", "0").to_int();
			ch.yoffset = _get_attr(attrs, "yoffset", "0").to_int();
			ch.xadvance = _get_attr(attrs, "xadvance", "0").to_int();
			ch.page = _get_attr(attrs, "page", "0").to_int();
			uint32_t glyph_index = (uint32_t)_get_attr(attrs, "id", "0").to_int();
			bm_data->chars[glyph_index] = ch;
		} else if (line.begins_with("kerning ")) {
			_parse_line_attrs(line, "kerning", 7, attrs);
			int first = _get_attr(attrs, "first", "0").to_int();
			int second = _get_attr(attrs, "second", "0").to_int();
			int amount = _get_attr(attrs, "amount", "0").to_int();
			bm_data->kernings[(uint32_t)first << 16 | (uint32_t)(second & 0xffff)] = amount;
		}
	}

	if (r_error) {
		*r_error = OK;
	}
	return bm_data;
}

static _FORCE_INLINE_ Ref<BMFontData> _load_bmfont_xml(const PoolVector<uint8_t> &p_data, const String &p_base_path, Error *r_error) {
	Ref<BMFontData> bm_data;
	bm_data.instance();

	Vector<uint8_t> buf;
	buf.resize(p_data.size());
	if (p_data.size() > 0) {
		PoolVector<uint8_t>::Read r = p_data.read();
		for (int i = 0; i < p_data.size(); i++) {
			buf.write[i] = r[i];
		}
	}

	Ref<XMLParser> parser;
	parser.instance();
	Error err = parser->open_buffer(buf);
	if (err != OK) {
		if (r_error) {
			*r_error = err;
		}
		return Ref<BMFontData>();
	}

	while (parser->read() == OK) {
		if (parser->get_node_type() != XMLParser::NODE_ELEMENT) {
			continue;
		}

		const String name = parser->get_node_name();

		if (name == "common") {
			bm_data->line_height = parser->get_attribute_value_safe("lineHeight").to_int();
			bm_data->base = parser->get_attribute_value_safe("base").to_int();
			bm_data->scale_w = parser->get_attribute_value_safe("scaleW").to_int();
			bm_data->scale_h = parser->get_attribute_value_safe("scaleH").to_int();
			bm_data->pages_count = parser->get_attribute_value_safe("pages").to_int();
		} else if (name == "page") {
			String file = parser->get_attribute_value_safe("file");
			if (!file.empty()) {
				bm_data->page_files.push_back(p_base_path + file);
			}
		} else if (name == "char") {
			BMFontChar ch;
			ch.x = parser->get_attribute_value_safe("x").to_int();
			ch.y = parser->get_attribute_value_safe("y").to_int();
			ch.width = parser->get_attribute_value_safe("width").to_int();
			ch.height = parser->get_attribute_value_safe("height").to_int();
			ch.xoffset = parser->get_attribute_value_safe("xoffset").to_int();
			ch.yoffset = parser->get_attribute_value_safe("yoffset").to_int();
			ch.xadvance = parser->get_attribute_value_safe("xadvance").to_int();
			ch.page = parser->get_attribute_value_safe("page").to_int();
			uint32_t glyph_index = (uint32_t)parser->get_attribute_value_safe("id").to_int();
			bm_data->chars[glyph_index] = ch;
		} else if (name == "kerning") {
			int first = parser->get_attribute_value_safe("first").to_int();
			int second = parser->get_attribute_value_safe("second").to_int();
			int amount = parser->get_attribute_value_safe("amount").to_int();
			bm_data->kernings[(uint32_t)first << 16 | (uint32_t)(second & 0xffff)] = amount;
		}
	}

	if (r_error) {
		*r_error = OK;
	}
	return bm_data;
}

static _FORCE_INLINE_ Ref<BMFontData> _load_bmfont_binary(const PoolVector<uint8_t> &p_data, const String &p_base_path, Error *r_error) {
	Ref<BMFontData> bm_data;
	bm_data.instance();

	PoolVector<uint8_t>::Read r = p_data.read();
	const uint8_t *src = r.ptr();
	int len = p_data.size();

	int pos = 3;
	uint8_t version = src[pos++];

	while (pos + 5 <= len) {
		uint8_t block_type = src[pos++];
		if (pos + 4 > len) {
			break;
		}
		uint32_t block_size = (uint32_t)src[pos] | ((uint32_t)src[pos + 1] << 8) | ((uint32_t)src[pos + 2] << 16) | ((uint32_t)src[pos + 3] << 24);
		pos += 4;

		if (pos + (int)block_size > len) {
			break;
		}

		const uint8_t *block = src + pos;

		switch (block_type) {
			case 2: {
				if (block_size < 15) {
					break;
				}
				int bpos = 0;
				uint16_t line_height = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
				bpos += 2;
				uint16_t base = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
				bpos += 2;
				uint16_t scale_w = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
				bpos += 2;
				uint16_t scale_h = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
				bpos += 2;
				uint16_t pages = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));

				bm_data->line_height = line_height;
				bm_data->base = base;
				bm_data->scale_w = scale_w;
				bm_data->scale_h = scale_h;
				bm_data->pages_count = pages;
			} break;
			case 3: {
				int bpos = 0;
				for (int i = 0; i < bm_data->pages_count && bpos < (int)block_size; i++) {
					const char *name = (const char *)(block + bpos);
					int max_len = (int)block_size - bpos;
					int strlen = 0;
					while (strlen < max_len && name[strlen] != '\0') {
						strlen++;
					}
					if (strlen == max_len) {
						break;
					}
					String file = String::utf8(name, strlen);
					if (!file.empty()) {
						bm_data->page_files.push_back(p_base_path + file);
					}
					bpos += strlen + 1;
				}
			} break;
			case 4: {
				const int record_size = 20;
				if (block_size < (unsigned)record_size) {
					break;
				}
				int count = block_size / record_size;
				int bpos = 0;
				for (int i = 0; i < count; i++) {
					if (bpos + record_size > (int)block_size) {
						break;
					}
					uint32_t id = (uint32_t)block[bpos] | ((uint32_t)block[bpos + 1] << 8) | ((uint32_t)block[bpos + 2] << 16) | ((uint32_t)block[bpos + 3] << 24);
					bpos += 4;
					uint16_t x = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					uint16_t y = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					uint16_t w = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					uint16_t h = (uint16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					int16_t xoff = (int16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					int16_t yoff = (int16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					int16_t xadv = (int16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;
					uint8_t page = block[bpos++];
					block[bpos++];

					BMFontChar ch;
					ch.x = x;
					ch.y = y;
					ch.width = w;
					ch.height = h;
					ch.xoffset = xoff;
					ch.yoffset = yoff;
					ch.xadvance = xadv;
					ch.page = page;

					bm_data->chars[id] = ch;
				}
			} break;
			case 5: {
				const int record_size = 10;
				if (block_size < (unsigned)record_size) {
					break;
				}
				int count = block_size / record_size;
				int bpos = 0;
				for (int i = 0; i < count; i++) {
					if (bpos + record_size > (int)block_size) {
						break;
					}
					uint32_t first = (uint32_t)block[bpos] | ((uint32_t)block[bpos + 1] << 8) | ((uint32_t)block[bpos + 2] << 16) | ((uint32_t)block[bpos + 3] << 24);
					bpos += 4;
					uint32_t second = (uint32_t)block[bpos] | ((uint32_t)block[bpos + 1] << 8) | ((uint32_t)block[bpos + 2] << 16) | ((uint32_t)block[bpos + 3] << 24);
					bpos += 4;
					int16_t amount = (int16_t)(block[bpos] | (block[bpos + 1] << 8));
					bpos += 2;

					bm_data->kernings[(uint32_t)first << 16 | (uint32_t)(second & 0xffff)] = amount;
				}
			} break;
			default:
				break;
		}

		pos += block_size;
	}

	if (r_error) {
		*r_error = OK;
	}
	return bm_data;
}

Ref<BMFontData> BMFontData::load_from_buffer(const PoolVector<uint8_t> &p_data, const String &p_base_path, Error *r_error) {
	PoolVector<uint8_t>::Read r = p_data.read();
	const char *src = (const char *)r.ptr();
	int len = p_data.size();

	if (len < 4) {
		if (r_error) {
			*r_error = ERR_FILE_UNRECOGNIZED;
		}
		return Ref<BMFontData>();
	}

	if (src[0] == 'B' && src[1] == 'M' && src[2] == 'F') {
		return _load_bmfont_binary(p_data, p_base_path, r_error);
	} else {
		int start = 0;
		while (start < len && (src[start] == ' ' || src[start] == '\t' || src[start] == '\r' || src[start] == '\n')) {
			start++;
		}

		if (len - start < 4) {
			if (r_error) {
				*r_error = ERR_FILE_UNRECOGNIZED;
			}
			return Ref<BMFontData>();
		}

		if (src[start] == '<' && src[start + 1] == '?' && src[start + 2] == 'x' && src[start + 3] == 'm' && src[start + 4] == 'l') {
			return _load_bmfont_xml(p_data, p_base_path, r_error);
		}

		if (src[start] == '<' && src[start + 1] == 'f' && src[start + 2] == 'o' && src[start + 3] == 'n' && src[start + 4] == 't') {
			return _load_bmfont_xml(p_data, p_base_path, r_error);
		}

		if (src[start] == 'i' && src[start + 1] == 'n' && src[start + 2] == 'f' && src[start + 3] == 'o') {
			return _load_bmfont_text(p_data, p_base_path, r_error);
		}
	}

	if (r_error) {
		*r_error = ERR_FILE_UNRECOGNIZED;
	}
	return Ref<BMFontData>();
}

/*************************************************************************/

FontDriverBMFont::FontDriverBMFont() {
}

FontDriverBMFont::~FontDriverBMFont() {
	const FontID *k = NULL;
	while ((k = font_id_to_info.next(k)) != NULL) {
		FontInfo *info = font_id_to_info[*k];
		info->driver = NULL;
		memdelete(info);
	}

	font_id_to_info.clear();
	fonts.clear();
}

_FORCE_INLINE_ void FontDriverBMFont::_setup_builtin_fonts() {
	{
		PoolVector<uint8_t> NotoSansUI_Regular;
		NotoSansUI_Regular.resize(_font_NotoSansUI_Regular_fnt_size);
		copymem(NotoSansUI_Regular.write().ptr(), _font_NotoSansUI_Regular_fnt, _font_NotoSansUI_Regular_fnt_size);

		Vector<PoolVector<uint8_t>> pages;
		pages.resize(_font_NotoSansUI_Regular_page_count);
		for (int i = 0; i < _font_NotoSansUI_Regular_page_count; i++) {
			PoolVector<uint8_t> page;
			page.resize(_font_NotoSansUI_Regular_page_sizes[i]);
			copymem(page.write().ptr(), _font_NotoSansUI_Regular_page[i], _font_NotoSansUI_Regular_page_sizes[i]);

			pages.write[i] = page;
		}

		FontID font_id;
		if (load_font_data(font_id, NotoSansUI_Regular, pages) == OK) {
			builtin_font_ids.push_back(font_id);
		}
	}
}

Error FontDriverBMFont::init() {
	_setup_builtin_fonts();

	return OK;
}

Error FontDriverBMFont::load_font_data(FontID &r_font_id, const PoolVector<uint8_t> &p_font_data, const Vector<PoolVector<uint8_t>> &p_page_data) {
	r_font_id = FontID();

	r_font_id.font_hash = XXH32(p_font_data.read().ptr(), p_font_data.size(), 0);

	BMFontInstance *font_instance = fonts.getptr(r_font_id);
	if (font_instance) {
		return OK;
	}

	Error err = OK;
	Ref<BMFontData> bm_data = BMFontData::load_from_buffer(p_font_data, String(), &err);
	if (err != OK || !bm_data.is_valid()) {
		return ERR_FILE_UNRECOGNIZED;
	}

	BMFontInstance new_font_instance;
	new_font_instance.data = bm_data;

	for (int i = 0; i < p_page_data.size(); i++) {
		Ref<Image> img = memnew(Image(p_page_data[i].read().ptr(), p_page_data[i].size()));
		ERR_CONTINUE_MSG(img.is_null() || img->empty(), "Error loading image.");

		new_font_instance.page_images.push_back(img);
	}

	if (new_font_instance.page_images.size() != (size_t)bm_data->pages_count) {
		return ERR_FILE_CORRUPT;
	}

	fonts[r_font_id] = new_font_instance;

	return OK;
}

Error FontDriverBMFont::load_font_data(FontID &r_font_id, const PoolVector<uint8_t> &p_font_data) {
	r_font_id = FontID();

	r_font_id.font_hash = XXH32(p_font_data.read().ptr(), p_font_data.size(), 0);

	BMFontInstance *font_instance = fonts.getptr(r_font_id);
	if (font_instance) {
		return OK;
	}

	Error err = OK;
	Ref<BMFontData> bm_data = BMFontData::load_from_buffer(p_font_data, String(), &err);
	if (err != OK || !bm_data.is_valid() || bm_data->page_files.empty()) {
		return ERR_FILE_UNRECOGNIZED;
	}

	BMFontInstance new_font_instance;
	new_font_instance.data = bm_data;

	for (int i = 0; i < bm_data->page_files.size(); i++) {
		Ref<Image> img;
		img.instance();

		const String &file = bm_data->page_files[i];
		err = ImageLoader::load_image(file, img);
		ERR_CONTINUE_MSG(err != OK, "Error loading image '" + file + "'.");

		new_font_instance.page_images.push_back(img);
	}

	fonts[r_font_id] = new_font_instance;

	return OK;
}

Error FontDriverBMFont::load_font_file(FontID &r_font_id, const String &p_font_path) {
	r_font_id = FontID();

	Error err;
	FileAccess *f = FileAccess::open(p_font_path, FileAccess::READ, &err);
	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot open font file '" + p_font_path + "'.");

	int len = f->get_len();
	PoolVector<uint8_t> font_data;
	font_data.resize(len);
	PoolVector<uint8_t>::Write w = font_data.write();
	int r = f->get_buffer(w.ptr(), len);
	f->close();
	memdelete(f);
	ERR_FAIL_COND_V_MSG(r != len, ERR_FILE_CORRUPT, "Failed to read font file '" + p_font_path + "'.");

	r_font_id.font_hash = XXH32(font_data.read().ptr(), font_data.size(), 0);

	BMFontInstance *font_instance = fonts.getptr(r_font_id);
	if (font_instance) {
		return OK;
	}

	String base_path = p_font_path.get_base_dir();
	if (!base_path.empty() && !base_path.ends_with("/")) {
		base_path += "/";
	}

	Ref<BMFontData> bm_data = BMFontData::load_from_buffer(font_data, base_path, &err);
	if (err != OK || !bm_data.is_valid() || bm_data->page_files.empty()) {
		return ERR_FILE_UNRECOGNIZED;
	}

	BMFontInstance new_font_instance;
	new_font_instance.data = bm_data;

	for (int i = 0; i < bm_data->page_files.size(); i++) {
		Ref<Image> img;
		img.instance();

		const String &file = bm_data->page_files[i];
		err = ImageLoader::load_image(file, img);
		ERR_CONTINUE_MSG(err != OK, "Error loading image '" + file + "'.");

		new_font_instance.page_images.push_back(img);
	}

	fonts[r_font_id] = new_font_instance;

	return OK;
}

Vector<FontID> FontDriverBMFont::get_builtin_font_ids() const {
	return builtin_font_ids;
}

Ref<FontDriver::FontInfo> FontDriverBMFont::get_font_info(const FontID &p_font_id) const {
	ERR_FAIL_COND_V(!p_font_id.is_valid(), Ref<FontInfo>());

	const BMFontInstance *font_instance = fonts.getptr(p_font_id);
	if (!font_instance) {
		return Ref<FontInfo>();
	}

	FontInfo **cached = font_id_to_info.getptr(p_font_id);
	if (cached && *cached) {
		return Ref<FontInfo>(*cached);
	}

	FontInfo *info = memnew(FontInfo);
	info->face_count = 1;
	info->path = font_instance->data->page_files.size() ? font_instance->data->page_files[0].get_base_dir().plus_file("font.fnt") : String();
	info->driver = const_cast<FontDriverBMFont *>(this);
	info->font_id = p_font_id;

	font_id_to_info[p_font_id] = info;
	return Ref<FontInfo>(info);
}

void FontDriverBMFont::finalize_font(const FontID &p_font_id) {
	font_id_to_info.erase(p_font_id);
	fonts.erase(p_font_id);
}

bool FontDriverBMFont::owns_font(const FontID &p_font_id) const {
	ERR_FAIL_COND_V(!p_font_id.is_valid(), false);

	return fonts.has(p_font_id);
}

bool FontDriverBMFont::validate_font(const FontID &p_font_id) {
	ERR_FAIL_COND_V(!p_font_id.is_valid(), false);

	const BMFontInstance *font_instance = fonts.getptr(p_font_id);
	return (font_instance && font_instance->data.is_valid() && !font_instance->page_images.empty());
}

uint32_t FontDriverBMFont::get_glyph_index(const FontID &p_font_id, char32_t p_char) const {
	ERR_FAIL_COND_V(!p_font_id.is_valid(), 0);

	const BMFontInstance *font_instance = fonts.getptr(p_font_id);
	if (!font_instance) {
		return 0;
	}

	if (font_instance->data->chars.has(p_char)) {
		return p_char;
	} else if (font_instance->data->chars.has('?')) {
		return '?';
	}

	return 0;
}

bool FontDriverBMFont::get_font_metrics(float &r_ascent, float &r_descent, const FontID &p_font_id, int p_size, int p_oversampling) const {
	ERR_FAIL_COND_V(!p_font_id.is_valid(), false);

	const BMFontInstance *font_instance = fonts.getptr(p_font_id);
	if (!font_instance) {
		return false;
	}

	int fixed_ascent = MAX(font_instance->data->base, 0);
	fixed_ascent = MIN(font_instance->data->line_height, fixed_ascent);

	r_ascent = (float)fixed_ascent;
	r_descent = (float)(font_instance->data->line_height - fixed_ascent);
	return true;
}

Vector2 FontDriverBMFont::get_font_kerning(const FontID &p_font_id, char32_t p_char, char32_t p_next_char, int p_size, int p_oversampling) const {
	const BMFontInstance *font_instance = fonts.getptr(p_font_id);
	if (!font_instance) {
		return Vector2();
	}

	uint32_t kerning_key = ((uint32_t)p_char << 16) | ((uint32_t)p_next_char & 0xffff);
	const int *kerning = font_instance->data->kernings.getptr(kerning_key);
	return kerning ? Vector2(*kerning, 0) : Vector2();
}

void FontDriverBMFont::clear_glyph_cache(const GlyphCacheKey &p_glyph_key) {
	if (glyph_info_map.has(p_glyph_key)) {
		glyph_info_map.erase(p_glyph_key);
	}

	if (texture_map.has(p_glyph_key)) {
		texture_map.erase(p_glyph_key);
	}
}

GlyphInfo FontDriverBMFont::get_glyph_info(const GlyphCacheKey &p_glyph_key, uint32_t p_glyph_index) {
	GlyphInfo glyph_info{};

	BMFontInstance *font_instance = fonts.getptr(p_glyph_key.font_id);
	ERR_FAIL_COND_V(!font_instance, glyph_info);

	const BMFontChar *ch = font_instance->data->chars.getptr(p_glyph_index);
	ERR_FAIL_COND_V(!ch, glyph_info);

	HashMap<uint32_t, GlyphInfo> *glyph_map = glyph_info_map.getptr(p_glyph_key);
	if (glyph_map && glyph_map->has(p_glyph_index)) {
		return (*glyph_map)[p_glyph_index];
	}

	if (!texture_map.has(p_glyph_key)) {
		Vector<RID> &textures = texture_map[p_glyph_key];
		textures.resize(font_instance->page_images.size());
	}

	ERR_FAIL_COND_V(ch->page >= font_instance->page_images.size(), glyph_info);

	glyph_info.found = true;
	glyph_info.texture_offset = Vector2((float)ch->xoffset, (float)(ch->yoffset - font_instance->data->base));
	glyph_info.advance = Vector2((float)ch->xadvance, 0);
	glyph_info.texture_index = ch->page;
	glyph_info.texture_size = Size2((float)ch->width, (float)ch->height);
	glyph_info.texture_rect_uv = Rect2((float)ch->x, (float)ch->y, (float)ch->width, (float)ch->height);
	glyph_info.texture_format = font_instance->page_images[ch->page]->get_format();
	if (p_glyph_key.font_use_mipmaps) {
		glyph_info.texture_flags |= Texture::FLAG_MIPMAPS;
	}
	if (p_glyph_key.font_use_filter) {
		glyph_info.texture_flags |= Texture::FLAG_FILTER;
	}
	glyph_info.cache_key = p_glyph_key;

	if (glyph_info.found) {
		HashMap<uint32_t, GlyphInfo> &glyph_map = glyph_info_map[p_glyph_key];
		glyph_map[p_glyph_index] = glyph_info;
	}

	return glyph_info;
}

RID FontDriverBMFont::get_glyph_texture_rid(const GlyphInfo &p_glyph_info) {
	Vector<RID> *textures = texture_map.getptr(p_glyph_info.cache_key);
	ERR_FAIL_COND_V(!textures, RID());
	ERR_FAIL_INDEX_V(p_glyph_info.texture_index, textures->size(), RID());

	BMFontInstance *font_instance = fonts.getptr(p_glyph_info.cache_key.font_id);
	ERR_FAIL_COND_V(!font_instance, RID());

	int idx = p_glyph_info.texture_index;
	ERR_FAIL_INDEX_V(idx, font_instance->page_images.size(), RID());

	if (!(*textures)[idx].is_valid()) {
		Ref<Image> img = font_instance->page_images[idx];
		if (img.is_valid() && !img->empty()) {
			(*textures).write[idx] = VisualServer::get_singleton()->texture_create_from_image(img, Texture::FLAG_VIDEO_SURFACE | p_glyph_info.texture_flags);
		}
	}
	return (*textures)[idx];
}
