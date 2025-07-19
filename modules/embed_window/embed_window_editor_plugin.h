/*************************************************************************/
/*  embed_window_editor_plugin.h                                         */
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

#ifndef EMBED_WINDOW_EDITOR_PLUGIN_H
#define EMBED_WINDOW_EDITOR_PLUGIN_H

#define EMBED_WINDOW_ENABLED
#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "editor/script_editor_debugger.h"

#include <windows.h>

class EmbedWindowEditorPlugin : public EditorPlugin {
	GDCLASS(EmbedWindowEditorPlugin, EditorPlugin);

	EditorNode *editor;

	VBoxContainer *vbox;

	HBoxContainer *hb;
	MenuButton *settings_menu;

	Control *window_area;
	Ref<StyleBoxFlat> window_area_ttp;

	ScriptEditorDebugger *debugger;

	struct StoredWindowInfo {
		LONG style = 0;
		LONG ex_style = 0;
		WINDOWPLACEMENT placement = { 0 };
	} stored_window_info;

	HWND embed_window_handle = NULL;
	HWND editor_window_handle = NULL;

	Rect2 last_region_rect;
	Rect2 last_embed_window_rect;

	HRGN last_popups_region = NULL;
	HRGN last_embed_window_region = NULL;

	bool region_rect_changed = false;
	bool region_visibility_changed = false;

	struct RegionData {
		HRGN region = NULL;
		Rect2 rect;
	};
	int last_region_map_size = 0;
	Map<ObjectID, RegionData> region_map;

	void _update_region(ObjectID p_object, const Rect2 &p_exclude);
	void _clear_region();

	void _embed_window_mode_changed(bool p_enable);
	void _draw_focus();

protected:
	static void _bind_methods();
	void _notification(int p_option);

public:
	virtual String get_name() const { return "EmbedWindow"; }
	bool has_main_screen() const { return false; }

	EmbedWindowEditorPlugin(EditorNode *p_node);
	~EmbedWindowEditorPlugin();
};

#endif // EMBED_WINDOW_EDITOR_PLUGIN_H
