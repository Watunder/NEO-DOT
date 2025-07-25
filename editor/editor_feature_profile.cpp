/*************************************************************************/
/*  editor_feature_profile.cpp                                           */
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

#include "editor_feature_profile.h"
#include "core/io/json.h"
#include "core/os/dir_access.h"
#include "editor/editor_settings.h"
#include "editor_node.h"
#include "editor_scale.h"

const char *EditorFeatureProfile::feature_names[FEATURE_MAX] = {
	TTRC("3D Editor"),
	TTRC("Script Editor"),
	TTRC("Scene Tree Editing"),
	TTRC("Node Dock"),
	TTRC("FileSystem Dock"),
	TTRC("Import Dock"),
};

const char *EditorFeatureProfile::feature_identifiers[FEATURE_MAX] = {
	"3d",
	"script",
	"scene_tree",
	"node_dock",
	"filesystem_dock",
	"import_dock",
};

void EditorFeatureProfile::set_disable_class(const StringName &p_class, bool p_disabled) {
	if (p_disabled) {
		disabled_classes.insert(p_class);
	} else {
		disabled_classes.erase(p_class);
	}
}

bool EditorFeatureProfile::is_class_disabled(const StringName &p_class) const {
	if (p_class == StringName()) {
		return false;
	}
	return disabled_classes.has(p_class) || is_class_disabled(ClassDB::get_parent_class_nocheck(p_class));
}

void EditorFeatureProfile::set_disable_class_editor(const StringName &p_class, bool p_disabled) {
	if (p_disabled) {
		disabled_editors.insert(p_class);
	} else {
		disabled_editors.erase(p_class);
	}
}

bool EditorFeatureProfile::is_class_editor_disabled(const StringName &p_class) const {
	if (p_class == StringName()) {
		return false;
	}
	return disabled_editors.has(p_class) || is_class_editor_disabled(ClassDB::get_parent_class_nocheck(p_class));
}

void EditorFeatureProfile::set_disable_class_property(const StringName &p_class, const StringName &p_property, bool p_disabled) {
	if (p_disabled) {
		if (!disabled_properties.has(p_class)) {
			disabled_properties[p_class] = Set<StringName>();
		}

		disabled_properties[p_class].insert(p_property);
	} else {
		ERR_FAIL_COND(!disabled_properties.has(p_class));
		disabled_properties[p_class].erase(p_property);
		if (disabled_properties[p_class].empty()) {
			disabled_properties.erase(p_class);
		}
	}
}
bool EditorFeatureProfile::is_class_property_disabled(const StringName &p_class, const StringName &p_property) const {
	if (!disabled_properties.has(p_class)) {
		return false;
	}

	if (!disabled_properties[p_class].has(p_property)) {
		return false;
	}

	return true;
}

bool EditorFeatureProfile::has_class_properties_disabled(const StringName &p_class) const {
	return disabled_properties.has(p_class);
}

void EditorFeatureProfile::set_disable_feature(Feature p_feature, bool p_disable) {
	ERR_FAIL_INDEX(p_feature, FEATURE_MAX);
	features_disabled[p_feature] = p_disable;
}
bool EditorFeatureProfile::is_feature_disabled(Feature p_feature) const {
	ERR_FAIL_INDEX_V(p_feature, FEATURE_MAX, false);
	return features_disabled[p_feature];
}

String EditorFeatureProfile::get_feature_name(Feature p_feature) {
	ERR_FAIL_INDEX_V(p_feature, FEATURE_MAX, String());
	return feature_names[p_feature];
}

Error EditorFeatureProfile::save_to_file(const String &p_path) {
	Dictionary json;
	json["type"] = "feature_profile";
	Array dis_classes;
	for (Set<StringName>::Element *E = disabled_classes.front(); E; E = E->next()) {
		dis_classes.push_back(String(E->get()));
	}
	dis_classes.sort();
	json["disabled_classes"] = dis_classes;

	Array dis_editors;
	for (Set<StringName>::Element *E = disabled_editors.front(); E; E = E->next()) {
		dis_editors.push_back(String(E->get()));
	}
	dis_editors.sort();
	json["disabled_editors"] = dis_editors;

	Array dis_props;

	for (Map<StringName, Set<StringName>>::Element *E = disabled_properties.front(); E; E = E->next()) {
		for (Set<StringName>::Element *F = E->get().front(); F; F = F->next()) {
			dis_props.push_back(String(E->key()) + ":" + String(F->get()));
		}
	}

	json["disabled_properties"] = dis_props;

	Array dis_features;
	for (int i = 0; i < FEATURE_MAX; i++) {
		if (features_disabled[i]) {
			dis_features.push_back(feature_identifiers[i]);
		}
	}

	json["disabled_features"] = dis_features;

	FileAccessRef f = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(!f, ERR_CANT_CREATE, "Cannot create file '" + p_path + "'.");

	String text = JSON::print(json, "\t");
	f->store_string(text);
	f->close();
	return OK;
}

Error EditorFeatureProfile::load_from_file(const String &p_path) {
	Error err;
	String text = FileAccess::get_file_as_string(p_path, &err);
	if (err != OK) {
		return err;
	}

	String err_str;
	int err_line;
	Variant v;
	err = JSON::parse(text, v, err_str, err_line);
	if (err != OK) {
		ERR_PRINTS("Error parsing '" + p_path + "' on line " + itos(err_line) + ": " + err_str);
		return ERR_PARSE_ERROR;
	}

	Dictionary json = v;

	if (!json.has("type") || String(json["type"]) != "feature_profile") {
		ERR_PRINTS("Error parsing '" + p_path + "', it's not a feature profile.");
		return ERR_PARSE_ERROR;
	}

	disabled_classes.clear();

	if (json.has("disabled_classes")) {
		Array disabled_classes_arr = json["disabled_classes"];
		for (int i = 0; i < disabled_classes_arr.size(); i++) {
			disabled_classes.insert(disabled_classes_arr[i]);
		}
	}

	disabled_editors.clear();

	if (json.has("disabled_editors")) {
		Array disabled_editors_arr = json["disabled_editors"];
		for (int i = 0; i < disabled_editors_arr.size(); i++) {
			disabled_editors.insert(disabled_editors_arr[i]);
		}
	}

	disabled_properties.clear();

	if (json.has("disabled_properties")) {
		Array disabled_properties_arr = json["disabled_properties"];
		for (int i = 0; i < disabled_properties_arr.size(); i++) {
			String s = disabled_properties_arr[i];
			set_disable_class_property(s.get_slice(":", 0), s.get_slice(":", 1), true);
		}
	}

	if (json.has("disabled_features")) {
		Array disabled_features_arr = json["disabled_features"];
		for (int i = 0; i < FEATURE_MAX; i++) {
			bool found = false;
			String f = feature_identifiers[i];
			for (int j = 0; j < disabled_features_arr.size(); j++) {
				String fd = disabled_features_arr[j];
				if (fd == f) {
					found = true;
					break;
				}
			}

			features_disabled[i] = found;
		}
	}

	return OK;
}

void EditorFeatureProfile::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_disable_class", "class_name", "disable"), &EditorFeatureProfile::set_disable_class);
	ClassDB::bind_method(D_METHOD("is_class_disabled", "class_name"), &EditorFeatureProfile::is_class_disabled);

	ClassDB::bind_method(D_METHOD("set_disable_class_editor", "class_name", "disable"), &EditorFeatureProfile::set_disable_class_editor);
	ClassDB::bind_method(D_METHOD("is_class_editor_disabled", "class_name"), &EditorFeatureProfile::is_class_editor_disabled);

	ClassDB::bind_method(D_METHOD("set_disable_class_property", "class_name", "property", "disable"), &EditorFeatureProfile::set_disable_class_property);
	ClassDB::bind_method(D_METHOD("is_class_property_disabled", "class_name", "property"), &EditorFeatureProfile::is_class_property_disabled);

	ClassDB::bind_method(D_METHOD("set_disable_feature", "feature", "disable"), &EditorFeatureProfile::set_disable_feature);
	ClassDB::bind_method(D_METHOD("is_feature_disabled", "feature"), &EditorFeatureProfile::is_feature_disabled);

	ClassDB::bind_method(D_METHOD("get_feature_name", "feature"), &EditorFeatureProfile::_get_feature_name);

	ClassDB::bind_method(D_METHOD("save_to_file", "path"), &EditorFeatureProfile::save_to_file);
	ClassDB::bind_method(D_METHOD("load_from_file", "path"), &EditorFeatureProfile::load_from_file);

	BIND_ENUM_CONSTANT(FEATURE_3D);
	BIND_ENUM_CONSTANT(FEATURE_SCRIPT);
	BIND_ENUM_CONSTANT(FEATURE_SCENE_TREE);
	BIND_ENUM_CONSTANT(FEATURE_NODE_DOCK);
	BIND_ENUM_CONSTANT(FEATURE_FILESYSTEM_DOCK);
	BIND_ENUM_CONSTANT(FEATURE_IMPORT_DOCK);
	BIND_ENUM_CONSTANT(FEATURE_MAX);
}

EditorFeatureProfile::EditorFeatureProfile() {
	for (int i = 0; i < FEATURE_MAX; i++) {
		features_disabled[i] = false;
	}
}

//////////////////////////

void EditorFeatureProfileManager::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		current_profile = EDITOR_GET("_default_feature_profile");
		if (current_profile != String()) {
			current.instance();
			Error err = current->load_from_file(EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(current_profile + ".profile"));
			if (err != OK) {
				ERR_PRINTS("Error loading default feature profile: " + current_profile);
				current_profile = String();
				current.unref();
			}
		}
		_update_profile_list(current_profile);
	}
}

String EditorFeatureProfileManager::_get_selected_profile() {
	int idx = profile_list->get_selected();
	if (idx < 0) {
		return String();
	}

	return profile_list->get_item_metadata(idx);
}

void EditorFeatureProfileManager::_update_profile_list(const String &p_select_profile) {
	String selected_profile;
	if (p_select_profile == String()) { //default, keep
		if (profile_list->get_selected() >= 0) {
			selected_profile = profile_list->get_item_metadata(profile_list->get_selected());
			if (!FileAccess::exists(EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(selected_profile + ".profile"))) {
				selected_profile = String(); //does not exist
			}
		}
	} else {
		selected_profile = p_select_profile;
	}

	Vector<String> profiles;
	DirAccessRef d = DirAccess::open(EditorSettings::get_singleton()->get_feature_profiles_dir());
	ERR_FAIL_COND_MSG(!d, "Cannot open directory '" + EditorSettings::get_singleton()->get_feature_profiles_dir() + "'.");

	d->list_dir_begin();
	while (true) {
		String f = d->get_next();
		if (f == String()) {
			break;
		}

		if (!d->current_is_dir()) {
			int last_pos = f.find_last(".profile");
			if (last_pos != -1) {
				profiles.push_back(f.substr(0, last_pos));
			}
		}
	}

	profiles.sort();

	profile_list->clear();

	for (int i = 0; i < profiles.size(); i++) {
		String name = profiles[i];

		if (i == 0 && selected_profile == String()) {
			selected_profile = name;
		}

		if (name == current_profile) {
			name += " (current)";
		}
		profile_list->add_item(name);
		int index = profile_list->get_item_count() - 1;
		profile_list->set_item_metadata(index, profiles[i]);
		if (profiles[i] == selected_profile) {
			profile_list->select(index);
		}
	}

	profile_actions[PROFILE_CLEAR]->set_disabled(current_profile == String());
	profile_actions[PROFILE_ERASE]->set_disabled(selected_profile == String());
	profile_actions[PROFILE_EXPORT]->set_disabled(selected_profile == String());
	profile_actions[PROFILE_SET]->set_disabled(selected_profile == String());

	current_profile_name->set_text(current_profile);

	_update_selected_profile();
}

void EditorFeatureProfileManager::_profile_action(int p_action) {
	switch (p_action) {
		case PROFILE_CLEAR: {
			EditorSettings::get_singleton()->set("_default_feature_profile", "");
			EditorSettings::get_singleton()->save();
			current_profile = "";
			current.unref();

			_update_profile_list();
			_emit_current_profile_changed();
		} break;
		case PROFILE_SET: {
			String selected = _get_selected_profile();
			ERR_FAIL_COND(selected == String());
			if (selected == current_profile) {
				return; // Nothing to do here.
			}
			EditorSettings::get_singleton()->set("_default_feature_profile", selected);
			EditorSettings::get_singleton()->save();
			current_profile = selected;
			current = edited;

			_update_profile_list();
			_emit_current_profile_changed();
		} break;
		case PROFILE_IMPORT: {
			import_profiles->popup_centered_ratio();
		} break;
		case PROFILE_EXPORT: {
			export_profile->popup_centered_ratio();
			export_profile->set_current_file(_get_selected_profile() + ".profile");
		} break;
		case PROFILE_NEW: {
			new_profile_dialog->popup_centered_minsize();
			new_profile_name->clear();
			new_profile_name->grab_focus();
		} break;
		case PROFILE_ERASE: {
			String selected = _get_selected_profile();
			ERR_FAIL_COND(selected == String());

			erase_profile_dialog->set_text(vformat(TTR("Erase profile '%s'? (no undo)"), selected));
			erase_profile_dialog->popup_centered_minsize();
		} break;
	}
}

void EditorFeatureProfileManager::_erase_selected_profile() {
	String selected = _get_selected_profile();
	ERR_FAIL_COND(selected == String());
	DirAccessRef da = DirAccess::open(EditorSettings::get_singleton()->get_feature_profiles_dir());
	ERR_FAIL_COND_MSG(!da, "Cannot open directory '" + EditorSettings::get_singleton()->get_feature_profiles_dir() + "'.");

	da->remove(selected + ".profile");
	if (selected == current_profile) {
		_profile_action(PROFILE_CLEAR);
	} else {
		_update_profile_list();
	}
}

void EditorFeatureProfileManager::_create_new_profile() {
	String name = new_profile_name->get_text().strip_edges();
	if (!name.is_valid_filename() || name.find(".") != -1) {
		EditorNode::get_singleton()->show_warning(TTR("Profile must be a valid filename and must not contain '.'"));
		return;
	}
	String file = EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(name + ".profile");
	if (FileAccess::exists(file)) {
		EditorNode::get_singleton()->show_warning(TTR("Profile with this name already exists."));
		return;
	}

	Ref<EditorFeatureProfile> new_profile;
	new_profile.instance();
	new_profile->save_to_file(file);

	_update_profile_list(name);
}

void EditorFeatureProfileManager::_profile_selected(int p_what) {
	_update_selected_profile();
}

void EditorFeatureProfileManager::_fill_classes_from(TreeItem *p_parent, const String &p_class, const String &p_selected) {
	TreeItem *class_item = class_list->create_item(p_parent);
	class_item->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
	class_item->set_icon(0, EditorNode::get_singleton()->get_class_icon(p_class, "Node"));
	String text = p_class;

	bool disabled = edited->is_class_disabled(p_class);
	bool disabled_editor = edited->is_class_editor_disabled(p_class);
	bool disabled_properties = edited->has_class_properties_disabled(p_class);
	if (disabled) {
		class_item->set_custom_color(0, get_color("disabled_font_color", "Editor"));
	} else if (disabled_editor && disabled_properties) {
		text += " " + TTR("(Editor Disabled, Properties Disabled)");
	} else if (disabled_properties) {
		text += " " + TTR("(Properties Disabled)");
	} else if (disabled_editor) {
		text += " " + TTR("(Editor Disabled)");
	}
	class_item->set_text(0, text);
	class_item->set_editable(0, true);
	class_item->set_selectable(0, true);
	class_item->set_metadata(0, p_class);

	if (p_class == p_selected) {
		class_item->select(0);
	}
	if (disabled) {
		//class disabled, do nothing else (do not show further)
		return;
	}

	class_item->set_checked(0, true); // if its not disabled, its checked

	List<StringName> child_classes;
	ClassDB::get_direct_inheriters_from_class(p_class, &child_classes);
	child_classes.sort_custom<StringName::AlphCompare>();

	for (List<StringName>::Element *E = child_classes.front(); E; E = E->next()) {
		String name = E->get();
		if (name.begins_with("Editor") || ClassDB::get_api_type(name) != ClassDB::API_CORE) {
			continue;
		}
		_fill_classes_from(class_item, name, p_selected);
	}
}

void EditorFeatureProfileManager::_class_list_item_selected() {
	if (updating_features)
		return;

	property_list->clear();

	TreeItem *item = class_list->get_selected();
	if (!item) {
		return;
	}

	Variant md = item->get_metadata(0);
	if (md.get_type() != Variant::STRING) {
		return;
	}

	String class_name = md;

	if (edited->is_class_disabled(class_name)) {
		return;
	}

	updating_features = true;
	TreeItem *root = property_list->create_item();
	TreeItem *options = property_list->create_item(root);
	options->set_text(0, TTR("Class Options:"));

	{
		TreeItem *option = property_list->create_item(options);
		option->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
		option->set_editable(0, true);
		option->set_selectable(0, true);
		option->set_checked(0, !edited->is_class_editor_disabled(class_name));
		option->set_text(0, TTR("Enable Contextual Editor"));
		option->set_metadata(0, CLASS_OPTION_DISABLE_EDITOR);
	}

	TreeItem *properties = property_list->create_item(root);
	properties->set_text(0, TTR("Enabled Properties:"));

	List<PropertyInfo> props;

	ClassDB::get_property_list(class_name, &props, true);

	for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {
		String name = E->get().name;
		if (!(E->get().usage & PROPERTY_USAGE_EDITOR))
			continue;
		TreeItem *property = property_list->create_item(properties);
		property->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
		property->set_editable(0, true);
		property->set_selectable(0, true);
		property->set_checked(0, !edited->is_class_property_disabled(class_name, name));
		property->set_text(0, name.capitalize());
		property->set_metadata(0, name);
		String icon_type = Variant::get_type_name(E->get().type);
		property->set_icon(0, EditorNode::get_singleton()->get_class_icon(icon_type));
	}

	updating_features = false;
}

void EditorFeatureProfileManager::_class_list_item_edited() {
	if (updating_features)
		return;

	TreeItem *item = class_list->get_edited();
	if (!item) {
		return;
	}

	bool checked = item->is_checked(0);

	Variant md = item->get_metadata(0);
	if (md.get_type() == Variant::STRING) {
		String class_selected = md;
		edited->set_disable_class(class_selected, !checked);
		_save_and_update();
		_update_selected_profile();
	} else if (md.get_type() == Variant::INT) {
		int feature_selected = md;
		edited->set_disable_feature(EditorFeatureProfile::Feature(feature_selected), !checked);
		_save_and_update();
	}
}

void EditorFeatureProfileManager::_property_item_edited() {
	if (updating_features)
		return;

	TreeItem *class_item = class_list->get_selected();
	if (!class_item) {
		return;
	}

	Variant md = class_item->get_metadata(0);
	if (md.get_type() != Variant::STRING) {
		return;
	}

	String class_name = md;

	TreeItem *item = property_list->get_edited();
	if (!item) {
		return;
	}
	bool checked = item->is_checked(0);

	md = item->get_metadata(0);
	if (md.get_type() == Variant::STRING) {
		String property_selected = md;
		edited->set_disable_class_property(class_name, property_selected, !checked);
		_save_and_update();
		_update_selected_profile();
	} else if (md.get_type() == Variant::INT) {
		int feature_selected = md;
		switch (feature_selected) {
			case CLASS_OPTION_DISABLE_EDITOR: {
				edited->set_disable_class_editor(class_name, !checked);
				_save_and_update();
				_update_selected_profile();
			} break;
		}
	}
}

void EditorFeatureProfileManager::_update_selected_profile() {
	String class_selected;
	int feature_selected = -1;

	if (class_list->get_selected()) {
		Variant md = class_list->get_selected()->get_metadata(0);
		if (md.get_type() == Variant::STRING) {
			class_selected = md;
		} else if (md.get_type() == Variant::INT) {
			feature_selected = md;
		}
	}

	class_list->clear();

	String profile = _get_selected_profile();
	if (profile == String()) { //nothing selected, nothing edited
		property_list->clear();
		edited.unref();
		return;
	}

	if (profile == current_profile) {
		edited = current; //reuse current profile (which is what editor uses)
		ERR_FAIL_COND(current.is_null()); //nothing selected, current should never be null
	} else {
		//reload edited, if different from current
		edited.instance();
		Error err = edited->load_from_file(EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(profile + ".profile"));
		ERR_FAIL_COND_MSG(err != OK, "Error when loading EditorSettings from file '" + EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(profile + ".profile") + "'.");
	}

	updating_features = true;

	TreeItem *root = class_list->create_item();

	TreeItem *features = class_list->create_item(root);
	TreeItem *last_feature;
	features->set_text(0, TTR("Enabled Features:"));
	for (int i = 0; i < EditorFeatureProfile::FEATURE_MAX; i++) {
		TreeItem *feature;
		if (i == EditorFeatureProfile::FEATURE_IMPORT_DOCK) {
			feature = class_list->create_item(last_feature);
		} else {
			feature = class_list->create_item(features);
			last_feature = feature;
		}
		feature->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
		feature->set_text(0, TTRGET(EditorFeatureProfile::get_feature_name(EditorFeatureProfile::Feature(i))));
		feature->set_selectable(0, true);
		feature->set_editable(0, true);
		feature->set_metadata(0, i);
		if (!edited->is_feature_disabled(EditorFeatureProfile::Feature(i))) {
			feature->set_checked(0, true);
		}

		if (i == feature_selected) {
			feature->select(0);
		}
	}

	TreeItem *classes = class_list->create_item(root);
	classes->set_text(0, TTR("Enabled Classes:"));

	_fill_classes_from(classes, "Node", class_selected);
	_fill_classes_from(classes, "Resource", class_selected);

	updating_features = false;

	_class_list_item_selected();
}

void EditorFeatureProfileManager::_import_profiles(const Vector<String> &p_paths) {
	//test it first
	for (int i = 0; i < p_paths.size(); i++) {
		Ref<EditorFeatureProfile> profile;
		profile.instance();
		Error err = profile->load_from_file(p_paths[i]);
		String basefile = p_paths[i].get_file();
		if (err != OK) {
			EditorNode::get_singleton()->show_warning(vformat(TTR("File '%s' format is invalid, import aborted."), basefile));
			return;
		}

		String dst_file = EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(basefile);

		if (FileAccess::exists(dst_file)) {
			EditorNode::get_singleton()->show_warning(vformat(TTR("Profile '%s' already exists. Remove it first before importing, import aborted."), basefile.get_basename()));
			return;
		}
	}

	//do it second
	for (int i = 0; i < p_paths.size(); i++) {
		Ref<EditorFeatureProfile> profile;
		profile.instance();
		Error err = profile->load_from_file(p_paths[i]);
		ERR_CONTINUE(err != OK);
		String basefile = p_paths[i].get_file();
		String dst_file = EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(basefile);
		profile->save_to_file(dst_file);
	}

	_update_profile_list();
}

void EditorFeatureProfileManager::_export_profile(const String &p_path) {
	ERR_FAIL_COND(edited.is_null());
	Error err = edited->save_to_file(p_path);
	if (err != OK) {
		EditorNode::get_singleton()->show_warning(vformat(TTR("Error saving profile to path: '%s'."), p_path));
	}
}

void EditorFeatureProfileManager::_save_and_update() {
	String edited_path = _get_selected_profile();
	ERR_FAIL_COND(edited_path == String());
	ERR_FAIL_COND(edited.is_null());

	edited->save_to_file(EditorSettings::get_singleton()->get_feature_profiles_dir().plus_file(edited_path + ".profile"));

	if (edited == current) {
		update_timer->start();
	}
}

void EditorFeatureProfileManager::_emit_current_profile_changed() {
	emit_signal("current_feature_profile_changed");
}

void EditorFeatureProfileManager::notify_changed() {
	_emit_current_profile_changed();
}

Ref<EditorFeatureProfile> EditorFeatureProfileManager::get_current_profile() {
	return current;
}

EditorFeatureProfileManager *EditorFeatureProfileManager::singleton = NULL;

void EditorFeatureProfileManager::_bind_methods() {
	ClassDB::bind_method("_update_selected_profile", &EditorFeatureProfileManager::_update_selected_profile);
	ClassDB::bind_method("_profile_action", &EditorFeatureProfileManager::_profile_action);
	ClassDB::bind_method("_create_new_profile", &EditorFeatureProfileManager::_create_new_profile);
	ClassDB::bind_method("_profile_selected", &EditorFeatureProfileManager::_profile_selected);
	ClassDB::bind_method("_erase_selected_profile", &EditorFeatureProfileManager::_erase_selected_profile);
	ClassDB::bind_method("_import_profiles", &EditorFeatureProfileManager::_import_profiles);
	ClassDB::bind_method("_export_profile", &EditorFeatureProfileManager::_export_profile);
	ClassDB::bind_method("_class_list_item_selected", &EditorFeatureProfileManager::_class_list_item_selected);
	ClassDB::bind_method("_class_list_item_edited", &EditorFeatureProfileManager::_class_list_item_edited);
	ClassDB::bind_method("_property_item_edited", &EditorFeatureProfileManager::_property_item_edited);
	ClassDB::bind_method("_emit_current_profile_changed", &EditorFeatureProfileManager::_emit_current_profile_changed);

	ADD_SIGNAL(MethodInfo("current_feature_profile_changed"));
}

EditorFeatureProfileManager::EditorFeatureProfileManager() {
	VBoxContainer *main_vbc = memnew(VBoxContainer);
	add_child(main_vbc);

	HBoxContainer *name_hbc = memnew(HBoxContainer);
	current_profile_name = memnew(LineEdit);
	name_hbc->add_child(current_profile_name);
	current_profile_name->set_editable(false);
	current_profile_name->set_h_size_flags(SIZE_EXPAND_FILL);
	profile_actions[PROFILE_CLEAR] = memnew(Button(TTR("Unset")));
	name_hbc->add_child(profile_actions[PROFILE_CLEAR]);
	profile_actions[PROFILE_CLEAR]->set_disabled(true);
	profile_actions[PROFILE_CLEAR]->connect("pressed", this, "_profile_action", varray(PROFILE_CLEAR));

	main_vbc->add_margin_child(TTR("Current Profile:"), name_hbc);

	HBoxContainer *profiles_hbc = memnew(HBoxContainer);
	profile_list = memnew(OptionButton);
	profile_list->set_h_size_flags(SIZE_EXPAND_FILL);
	profiles_hbc->add_child(profile_list);
	profile_list->connect("item_selected", this, "_profile_selected");

	profile_actions[PROFILE_SET] = memnew(Button(TTR("Make Current")));
	profiles_hbc->add_child(profile_actions[PROFILE_SET]);
	profile_actions[PROFILE_SET]->set_disabled(true);
	profile_actions[PROFILE_SET]->connect("pressed", this, "_profile_action", varray(PROFILE_SET));

	profile_actions[PROFILE_ERASE] = memnew(Button(TTR("Remove")));
	profiles_hbc->add_child(profile_actions[PROFILE_ERASE]);
	profile_actions[PROFILE_ERASE]->set_disabled(true);
	profile_actions[PROFILE_ERASE]->connect("pressed", this, "_profile_action", varray(PROFILE_ERASE));

	profiles_hbc->add_child(memnew(VSeparator));

	profile_actions[PROFILE_NEW] = memnew(Button(TTR("New")));
	profiles_hbc->add_child(profile_actions[PROFILE_NEW]);
	profile_actions[PROFILE_NEW]->connect("pressed", this, "_profile_action", varray(PROFILE_NEW));

	profiles_hbc->add_child(memnew(VSeparator));

	profile_actions[PROFILE_IMPORT] = memnew(Button(TTR("Import")));
	profiles_hbc->add_child(profile_actions[PROFILE_IMPORT]);
	profile_actions[PROFILE_IMPORT]->connect("pressed", this, "_profile_action", varray(PROFILE_IMPORT));

	profile_actions[PROFILE_EXPORT] = memnew(Button(TTR("Export")));
	profiles_hbc->add_child(profile_actions[PROFILE_EXPORT]);
	profile_actions[PROFILE_EXPORT]->set_disabled(true);
	profile_actions[PROFILE_EXPORT]->connect("pressed", this, "_profile_action", varray(PROFILE_EXPORT));

	main_vbc->add_margin_child(TTR("Available Profiles:"), profiles_hbc);

	h_split = memnew(HSplitContainer);
	h_split->set_v_size_flags(SIZE_EXPAND_FILL);
	main_vbc->add_child(h_split);

	VBoxContainer *class_list_vbc = memnew(VBoxContainer);
	h_split->add_child(class_list_vbc);
	class_list_vbc->set_h_size_flags(SIZE_EXPAND_FILL);

	class_list = memnew(Tree);
	class_list_vbc->add_margin_child(TTR("Enabled Classes:"), class_list, true);
	class_list->set_hide_root(true);
	class_list->set_edit_checkbox_cell_only_when_checkbox_is_pressed(true);
	class_list->connect("cell_selected", this, "_class_list_item_selected");
	class_list->connect("item_edited", this, "_class_list_item_edited", varray(), CONNECT_DEFERRED);

	VBoxContainer *property_list_vbc = memnew(VBoxContainer);
	h_split->add_child(property_list_vbc);
	property_list_vbc->set_h_size_flags(SIZE_EXPAND_FILL);

	property_list = memnew(Tree);
	property_list_vbc->add_margin_child(TTR("Class Options"), property_list, true);
	property_list->set_hide_root(true);
	property_list->set_hide_folding(true);
	property_list->set_edit_checkbox_cell_only_when_checkbox_is_pressed(true);
	property_list->connect("item_edited", this, "_property_item_edited", varray(), CONNECT_DEFERRED);

	new_profile_dialog = memnew(ConfirmationDialog);
	new_profile_dialog->set_title(TTR("New profile name:"));
	new_profile_name = memnew(LineEdit);
	new_profile_dialog->add_child(new_profile_name);
	new_profile_name->set_custom_minimum_size(Size2(300 * EDSCALE, 1));
	add_child(new_profile_dialog);
	new_profile_dialog->connect("confirmed", this, "_create_new_profile");
	new_profile_dialog->register_text_enter(new_profile_name);
	new_profile_dialog->get_ok()->set_text(TTR("Create"));

	erase_profile_dialog = memnew(ConfirmationDialog);
	add_child(erase_profile_dialog);
	erase_profile_dialog->set_title(TTR("Erase Profile"));
	erase_profile_dialog->connect("confirmed", this, "_erase_selected_profile");

	import_profiles = memnew(EditorFileDialog);
	add_child(import_profiles);
	import_profiles->set_mode(EditorFileDialog::MODE_OPEN_FILES);
	import_profiles->add_filter("*.profile; " + TTR("Godot Feature Profile"));
	import_profiles->connect("files_selected", this, "_import_profiles");
	import_profiles->set_title(TTR("Import Profile(s)"));
	import_profiles->set_access(EditorFileDialog::ACCESS_FILESYSTEM);

	export_profile = memnew(EditorFileDialog);
	add_child(export_profile);
	export_profile->set_mode(EditorFileDialog::MODE_SAVE_FILE);
	export_profile->add_filter("*.profile; " + TTR("Godot Feature Profile"));
	export_profile->connect("file_selected", this, "_export_profile");
	export_profile->set_title(TTR("Export Profile"));
	export_profile->set_access(EditorFileDialog::ACCESS_FILESYSTEM);

	set_title(TTR("Manage Editor Feature Profiles"));
	EDITOR_DEF("_default_feature_profile", "");

	update_timer = memnew(Timer);
	update_timer->set_wait_time(1); //wait a second before updating editor
	add_child(update_timer);
	update_timer->connect("timeout", this, "_emit_current_profile_changed");
	update_timer->set_one_shot(true);

	updating_features = false;

	singleton = this;
}
