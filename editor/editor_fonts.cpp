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
#include "scene/resources/default_theme/default_theme.h"
#include "scene/resources/freetype_font.h"
#include "servers/font/builtin_fonts.gen.h"

// Enable filtering and mipmaps on the editor fonts to improve text appearance
// in editors that are zoomed in/out without having dedicated fonts to generate.
// This is the case in GraphEdit-based editors such as the visual script and
// visual shader editors.

// the custom spacings might only work with Noto Sans
#define MAKE_DEFAULT_FONT(m_name, m_size)                       \
	Ref<FreeTypeFont> m_name;                                   \
	m_name.instance();                                          \
	if (CustomFont.is_valid()) {                                \
		m_name->set_data(CustomFont->duplicate());              \
	} else {                                                    \
		m_name->set_data(DefaultFont->get_data()->duplicate()); \
	}                                                           \
	m_name->set_size(m_size);                                   \
	m_name->set_use_filter(true);                               \
	m_name->set_use_mipmaps(true);                              \
	m_name->set_spacing(FreeTypeFont::SPACING_TOP, -EDSCALE);   \
	m_name->set_spacing(FreeTypeFont::SPACING_BOTTOM, -EDSCALE);

#define MAKE_BOLD_FONT(m_name, m_size)                        \
	Ref<FreeTypeFont> m_name;                                 \
	m_name.instance();                                        \
	if (CustomFontBold.is_valid()) {                          \
		m_name->set_data(CustomFontBold->duplicate());        \
	} else {                                                  \
		m_name->set_data(NotoSansUI_Bold->duplicate());       \
	}                                                         \
	m_name->set_size(m_size);                                 \
	m_name->set_use_filter(true);                             \
	m_name->set_use_mipmaps(true);                            \
	m_name->set_spacing(FreeTypeFont::SPACING_TOP, -EDSCALE); \
	m_name->set_spacing(FreeTypeFont::SPACING_BOTTOM, -EDSCALE);

#define MAKE_SOURCE_FONT(m_name, m_size)                      \
	Ref<FreeTypeFont> m_name;                                 \
	m_name.instance();                                        \
	if (CustomFontSource.is_valid()) {                        \
		m_name->set_data(CustomFontSource->duplicate());      \
	} else {                                                  \
		m_name->set_data(Hack_Regular->duplicate());          \
	}                                                         \
	m_name->set_size(m_size);                                 \
	m_name->set_use_filter(true);                             \
	m_name->set_use_mipmaps(true);                            \
	m_name->set_spacing(FreeTypeFont::SPACING_TOP, -EDSCALE); \
	m_name->set_spacing(FreeTypeFont::SPACING_BOTTOM, -EDSCALE);

void editor_register_fonts(Ref<Theme> p_theme) {
	DirAccess *dir = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

	/* Custom font */

	int font_hinting_setting = GLOBAL_GET("rendering/font/freetype_fonts/hinting");
	int font_antialiasing_setting = GLOBAL_GET("rendering/font/freetype_fonts/antialiasing");

	FreeTypeFont::Hinting font_hinting;
	switch (font_hinting_setting) {
		case 0:
			// The "Auto" setting uses the setting that best matches the OS' font rendering:
			// - macOS doesn't use font hinting.
			// - Windows uses ClearType, which is in between "Light" and "Normal" hinting.
			// - Linux has configurable font hinting, but most distributions including Ubuntu default to "Light".
#if defined(PLATFORM_APPLE) && TARGET_OSX
			font_hinting = FreeTypeFont::HINTING_NONE;
#else
			font_hinting = FreeTypeFont::HINTING_LIGHT;
#endif
			break;
		case 1:
			font_hinting = FreeTypeFont::HINTING_NONE;
			break;
		case 2:
			font_hinting = FreeTypeFont::HINTING_LIGHT;
			break;
		default:
			font_hinting = FreeTypeFont::HINTING_NORMAL;
			break;
	}

	FreeTypeFont::Antialiasing font_antialiasing;
	switch (font_antialiasing_setting) {
		case 0:
			font_antialiasing = FreeTypeFont::ANTIALIASING_NONE;
			break;
		default:
			font_antialiasing = FreeTypeFont::ANTIALIASING_NORMAL;
			break;
	}

	String custom_font_path = EditorSettings::get_singleton()->get("interface/editor/main_font");
	Ref<FreeTypeFontData> CustomFont;
	if (custom_font_path.length() > 0 && dir->file_exists(custom_font_path)) {
		CustomFont.instance();
		CustomFont->load_from_file(custom_font_path);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/main_font", "");
	}

	/* Custom Bold font */

	String custom_font_path_bold = EditorSettings::get_singleton()->get("interface/editor/main_font_bold");
	Ref<FreeTypeFontData> CustomFontBold;
	if (custom_font_path_bold.length() > 0 && dir->file_exists(custom_font_path_bold)) {
		CustomFontBold.instance();
		CustomFontBold->load_from_file(custom_font_path_bold);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/main_font_bold", "");
	}

	/* Custom source code font */

	String custom_font_path_source = EditorSettings::get_singleton()->get("interface/editor/code_font");
	Ref<FreeTypeFontData> CustomFontSource;
	if (custom_font_path_source.length() > 0 && dir->file_exists(custom_font_path_source)) {
		CustomFontSource.instance();
		CustomFontSource->load_from_file(custom_font_path_source);
	} else {
		EditorSettings::get_singleton()->set_manually("interface/editor/code_font", "");
	}

	memdelete(dir);

	Ref<FreeTypeFontData> NotoSansUI_Bold;
	NotoSansUI_Bold.instance();
	NotoSansUI_Bold->load_from_memory(_font_NotoSansUI_Bold, _font_NotoSansUI_Bold_size);

	Ref<FreeTypeFontData> Hack_Regular;
	Hack_Regular.instance();
	Hack_Regular->load_from_memory(_font_Hack_Regular, _font_Hack_Regular_size);

	Ref<FreeTypeFont> DefaultFont = p_theme->get_font("", "");

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
