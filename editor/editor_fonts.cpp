/*************************************************************************/
/*  editor_fonts.cpp                                                     */
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

#include "editor_fonts.h"

#include "editor_scale.h"
#include "editor_settings.h"

#include "core/os/dir_access.h"
#include "core/project_settings.h"
#include "modules/freetype/builtin_vector_fonts.gen.h"
#include "scene/resources/default_theme/default_theme.h"
#include "scene/resources/freetype_font.h"
#include "servers/font_server.h"

// Enable filtering and mipmaps on the editor fonts to improve text appearance
// in editors that are zoomed in/out without having dedicated fonts to generate.
// This is the case in GraphEdit-based editors such as the visual script and
// visual shader editors.

// the custom spacings might only work with Noto Sans
#define MAKE_DEFAULT_FONT(m_name, m_size)                   \
	Ref<FreeTypeFont> m_name;                               \
	m_name.instance();                                      \
	if (CustomFont.is_valid()) {                            \
		m_name->set_data(CustomFont->get_data());           \
	}                                                       \
	m_name->set_face_size(m_size);                          \
	m_name->set_use_filter(true);                           \
	m_name->set_use_mipmaps(true);                          \
	m_name->set_spacing(FontServer::SPACING_TOP, -EDSCALE); \
	m_name->set_spacing(FontServer::SPACING_BOTTOM, -EDSCALE);

#define MAKE_BOLD_FONT(m_name, m_size)                      \
	Ref<FreeTypeFont> m_name;                               \
	m_name.instance();                                      \
	if (CustomFontBold.is_valid()) {                        \
		m_name->set_data(CustomFontBold->get_data());       \
	} else {                                                \
		m_name->set_data(NotoSansUI_Bold);                  \
	}                                                       \
	m_name->set_face_size(m_size);                          \
	m_name->set_use_filter(true);                           \
	m_name->set_use_mipmaps(true);                          \
	m_name->set_spacing(FontServer::SPACING_TOP, -EDSCALE); \
	m_name->set_spacing(FontServer::SPACING_BOTTOM, -EDSCALE);

#define MAKE_SOURCE_FONT(m_name, m_size)                    \
	Ref<FreeTypeFont> m_name;                               \
	m_name.instance();                                      \
	if (CustomFontSource.is_valid()) {                      \
		m_name->set_data(CustomFontSource->get_data());     \
	} else {                                                \
		m_name->set_data(Hack_Regular);                     \
	}                                                       \
	m_name->set_face_size(m_size);                          \
	m_name->set_use_filter(true);                           \
	m_name->set_use_mipmaps(true);                          \
	m_name->set_spacing(FontServer::SPACING_TOP, -EDSCALE); \
	m_name->set_spacing(FontServer::SPACING_BOTTOM, -EDSCALE);

void editor_register_fonts(Ref<Theme> p_theme) {
	DirAccess *dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

	/* Custom font */

	String custom_font_path = EditorSettings::get_singleton()->get("interface/editor/main_font");
	Ref<FreeTypeFont> CustomFont;
	if (custom_font_path.length() > 0 && dir->file_exists(custom_font_path)) {
		CustomFont.instance();
		CustomFont->load(custom_font_path);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/main_font", "");
	}

	/* Custom Bold font */

	String custom_font_path_bold = EditorSettings::get_singleton()->get("interface/editor/main_font_bold");
	Ref<FreeTypeFont> CustomFontBold;
	if (custom_font_path_bold.length() > 0 && dir->file_exists(custom_font_path_bold)) {
		CustomFontBold.instance();
		CustomFontBold->load(custom_font_path_bold);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/main_font_bold", "");
	}

	/* Custom source code font */

	String custom_font_path_source = EditorSettings::get_singleton()->get("interface/editor/code_font");
	Ref<FreeTypeFont> CustomFontSource;
	if (custom_font_path_source.length() > 0 && dir->file_exists(custom_font_path_source)) {
		CustomFontSource.instance();
		CustomFontSource->load(custom_font_path_source);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/code_font", "");
	}

	memdelete(dir);

	PoolVector<uint8_t> NotoSansUI_Bold;
	NotoSansUI_Bold.resize(_font_NotoSansUI_Bold_size);
	copymem(NotoSansUI_Bold.write().ptr(), _font_NotoSansUI_Bold, _font_NotoSansUI_Bold_size);

	PoolVector<uint8_t> Hack_Regular;
	Hack_Regular.resize(_font_Hack_Regular_size);
	copymem(Hack_Regular.write().ptr(), _font_Hack_Regular, _font_Hack_Regular_size);

	int default_font_size = int(EDITOR_GET("interface/editor/main_font_size")) * EDSCALE;

	// Default font
	MAKE_DEFAULT_FONT(df, default_font_size);
	p_theme->set_default_theme_font(df);
	p_theme->set_font("main", "EditorFonts", df);

	// Bold font
	MAKE_BOLD_FONT(df_bold, default_font_size);
	p_theme->set_font("bold", "EditorFonts", df_bold);

	// Title font
	MAKE_BOLD_FONT(df_title, default_font_size + 2 * EDSCALE);
	p_theme->set_font("title", "EditorFonts", df_title);

	// Documentation fonts
	MAKE_DEFAULT_FONT(df_doc, int(EDITOR_GET("text_editor/help/help_font_size")) * EDSCALE);
	MAKE_BOLD_FONT(df_doc_bold, int(EDITOR_GET("text_editor/help/help_font_size")) * EDSCALE);
	MAKE_BOLD_FONT(df_doc_title, int(EDITOR_GET("text_editor/help/help_title_font_size")) * EDSCALE);
	MAKE_SOURCE_FONT(df_doc_code, int(EDITOR_GET("text_editor/help/help_source_font_size")) * EDSCALE);
	p_theme->set_font("doc", "EditorFonts", df_doc);
	p_theme->set_font("doc_bold", "EditorFonts", df_doc_bold);
	p_theme->set_font("doc_title", "EditorFonts", df_doc_title);
	p_theme->set_font("doc_source", "EditorFonts", df_doc_code);

	// Ruler font
	MAKE_DEFAULT_FONT(df_rulers, 8 * EDSCALE);
	p_theme->set_font("rulers", "EditorFonts", df_rulers);

	// Rotation widget font
	MAKE_DEFAULT_FONT(df_rotation_control, 14 * EDSCALE);
	p_theme->set_font("rotation_control", "EditorFonts", df_rotation_control);

	// Code font
	MAKE_SOURCE_FONT(df_code, int(EDITOR_GET("interface/editor/code_font_size")) * EDSCALE);
	p_theme->set_font("source", "EditorFonts", df_code);

	MAKE_SOURCE_FONT(df_expression, (int(EDITOR_GET("interface/editor/code_font_size")) - 1) * EDSCALE);
	p_theme->set_font("expression", "EditorFonts", df_expression);

	MAKE_SOURCE_FONT(df_output_code, int(EDITOR_GET("run/output/font_size")) * EDSCALE);
	p_theme->set_font("output_source", "EditorFonts", df_output_code);

	MAKE_SOURCE_FONT(df_text_editor_status_code, default_font_size);
	p_theme->set_font("status_source", "EditorFonts", df_text_editor_status_code);
}
