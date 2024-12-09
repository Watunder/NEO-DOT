#include "embed_window_editor_plugin.h"

void EmbedWindowEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_embed_window_mode_changed", "enable"), &EmbedWindowEditorPlugin::_embed_window_mode_changed);
	ClassDB::bind_method(D_METHOD("_update_region", "object", "exclude"), &EmbedWindowEditorPlugin::_update_region);
	ClassDB::bind_method(D_METHOD("_clear_region"), &EmbedWindowEditorPlugin::_clear_region);
	ClassDB::bind_method("_draw_focus", &EmbedWindowEditorPlugin::_draw_focus);
}

void EmbedWindowEditorPlugin::_update_region(ObjectID p_object, const Rect2 &p_exclude) {
	if (p_exclude == last_region_rect) {
		return;
	}

	last_region_rect = p_exclude;

	real_t dx = last_region_rect.position.x - last_embed_window_rect.position.x - window_area_ttp->get_border_width(MARGIN_LEFT);
	real_t dy = last_region_rect.position.y - last_embed_window_rect.position.y - window_area_ttp->get_border_width(MARGIN_TOP);

	HRGN region = CreateRectRgn(dx, dy, dx + last_region_rect.size.width, dy + last_region_rect.size.height);

	RegionData data;
	data.region = region;
	data.rect = last_region_rect;
	region_map.insert(p_object, data);
}

void EmbedWindowEditorPlugin::_clear_region() {
	HRGN region = CreateRectRgn(0, 0, 0, 0);
	int ret = GetWindowRgn(embed_window_handle, region);
	if (ret != ERROR && ret != NULLREGION) {
		SetWindowRgn(embed_window_handle, NULL, TRUE);
	}
	DeleteObject(region);

	last_region_rect = Rect2();
	DeleteObject(last_popups_region);
	region_rect_changed = false;
	region_visibility_changed = false;
}

void EmbedWindowEditorPlugin::_notification(int p_option) {
	switch (p_option) {
		case NOTIFICATION_PROCESS: {
			Rect2 rect = window_area->get_global_rect();
			if (rect != last_embed_window_rect) {
				last_embed_window_rect = rect;

				last_embed_window_region = CreateRectRgn(0, 0, last_embed_window_rect.size.width, last_embed_window_rect.size.height);
				region_rect_changed = true;

				rect.position.x += window_area_ttp->get_border_width(MARGIN_LEFT);
				rect.position.y += window_area_ttp->get_border_width(MARGIN_TOP);
				rect.size.width -= window_area_ttp->get_border_width(MARGIN_LEFT) + window_area_ttp->get_border_width(MARGIN_RIGHT);
				rect.size.height -= window_area_ttp->get_border_width(MARGIN_TOP) + window_area_ttp->get_border_width(MARGIN_BOTTOM);
				SetWindowPos(embed_window_handle, NULL, (int)rect.position.x, (int)rect.position.y, (int)rect.size.width, (int)rect.size.height, SWP_NOZORDER | SWP_NOACTIVATE);
			}

			for (Map<ObjectID, RegionData>::Element *E = region_map.front(); E; E = E->next()) {
				Object *obj = ObjectDB::get_instance(E->key());
				Control *control = Object::cast_to<Control>(obj);
				if (control) {
					bool visible = control->is_visible();
					if (!visible) {
						region_map.erase(E);
						region_visibility_changed = true;
						continue;
					}

					Rect2 rect = control->get_global_rect();
					if (Object::cast_to<WindowDialog>(control)) {
						int title_height = control->get_constant("title_height", "WindowDialog");
						rect.position.y -= title_height;
						rect.size.y += title_height;
					}
					if (rect != E->get().rect) {
						E->get().rect = rect;
						_update_region(E->key(), rect);
						region_rect_changed = true;
					}
				} else {
					region_map.erase(E);
					region_visibility_changed = true;
				}
			}

			if (last_region_map_size != region_map.size()) {
				last_region_map_size = region_map.size();
				region_visibility_changed = true;
			}

			if (region_visibility_changed || region_rect_changed) {
				for (Map<ObjectID, RegionData>::Element *E = region_map.front(); E; E = E->next()) {
					if (last_popups_region) {
						HRGN complex_region = CreateRectRgn(0, 0, 0, 0);
						CombineRgn(complex_region, last_popups_region, E->get().region, RGN_OR);
						last_popups_region = complex_region;
					} else {
						last_popups_region = E->get().region;
					}
				}

				if (last_popups_region) {
					HRGN region = CreateRectRgn(0, 0, 0, 0);
					CombineRgn(region, last_embed_window_region, last_popups_region, RGN_XOR);
					SetWindowRgn(embed_window_handle, region, TRUE);
					DeleteObject(region);

					last_region_rect = Rect2();
					last_popups_region = NULL;
					region_rect_changed = false;
					region_visibility_changed = false;
				} else if (region_visibility_changed) {
					_clear_region();
				}
			}
		} break;
	}
}

void EmbedWindowEditorPlugin::_embed_window_mode_changed(bool p_enable) {
	if (p_enable) {
		editor->get_viewport()->hide();
		editor->get_embed_viewport()->show();
		editor->get_bottom_panel_raise_button()->hide();
		editor->get_top_split()->show();

		HWND child_handle = (HWND)debugger->get_remote_window_handle();
		HWND parent_handle = editor_window_handle;

		WINDOWPLACEMENT placement = { 0 };
		placement.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement(child_handle, &placement)) {
			stored_window_info.placement = placement;
			stored_window_info.style = GetWindowLongPtr(child_handle, GWL_STYLE);
			stored_window_info.ex_style = GetWindowLongPtr(child_handle, GWL_EXSTYLE);
		}

		LONG style = stored_window_info.style & ~(WS_BORDER | WS_DLGFRAME | WS_CAPTION | WS_THICKFRAME);
		LONG ex_style = stored_window_info.ex_style & ~(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE);
		SetWindowLongPtr(child_handle, GWL_STYLE, style);
		SetWindowLongPtr(child_handle, GWL_EXSTYLE, ex_style);
		SetWindowPos(child_handle, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

		SetParent(child_handle, parent_handle);

		embed_window_handle = child_handle;

		set_process(true);
	} else {
		editor->get_viewport()->show();
		editor->get_embed_viewport()->hide();
		editor->get_bottom_panel_raise_button()->show();

		HWND child_handle = (HWND)debugger->get_remote_window_handle();

		SetParent(child_handle, NULL);
		if (SetWindowPlacement(child_handle, &stored_window_info.placement)) {
			SetWindowLongPtr(child_handle, GWL_STYLE, stored_window_info.style);
			SetWindowLongPtr(child_handle, GWL_EXSTYLE, stored_window_info.ex_style);
		}

		SetWindowRgn(embed_window_handle, NULL, TRUE);

		embed_window_handle = NULL;
		last_embed_window_rect = Rect2();
		DeleteObject(last_embed_window_region);
		region_map.clear();
		_clear_region();

		set_process(false);
	}
}

void EmbedWindowEditorPlugin::_draw_focus() {
	window_area_ttp->draw(window_area->get_canvas_item(), Rect2(Point2(), window_area->get_size()));
}

EmbedWindowEditorPlugin::EmbedWindowEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	editor->set_embed_window_editor_plugin(editor->get_editor_data().get_editor_plugin_count());
	editor->call_deferred("connect", "embed_window_mode_changed", this, "_embed_window_mode_changed");

	vbox = memnew(VBoxContainer);
	vbox->set_anchors_and_margins_preset(Control::PRESET_WIDE);
	vbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	editor->get_embed_viewport()->add_child(vbox);

	hb = memnew(HBoxContainer);
	hb->set_anchors_and_margins_preset(Control::PRESET_WIDE);
	vbox->add_child(hb);

	settings_menu = memnew(MenuButton);
	settings_menu->set_text(TTR("Settings"));
	hb->add_child(settings_menu);
	hb->add_child(memnew(VSeparator));

	window_area = memnew(Control);
	window_area->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	window_area->connect("draw", this, "_draw_focus");
	vbox->add_child(window_area);

	window_area_ttp = window_area->get_stylebox("Focus", "EditorStyles");

	editor_window_handle = (HWND)OS::get_singleton()->get_native_handle(OS::HandleType::WINDOW_HANDLE);

	debugger = ScriptEditor::get_singleton()->get_debugger();
}

EmbedWindowEditorPlugin::~EmbedWindowEditorPlugin() {
	DeleteObject(last_popups_region);
	DeleteObject(last_embed_window_region);
}