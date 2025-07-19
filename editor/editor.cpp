/*************************************************************************/
/*  editor.cpp                                                           */
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

#include "editor.h"

#include "core/crypto/crypto.h"
#include "editor/doc/doc_data.h"
#include "editor/doc/doc_data_class_path.gen.h"
#include "editor_node.h"
#include "editor_settings.h"
#include "progress_dialog.h"
#include "project_manager.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"

namespace Editor {

MainLoop *run_project_manger() {
	SceneTree *sml = memnew(SceneTree);

	Engine::get_singleton()->set_editor_hint(true);
	ProjectManager *pmanager = memnew(ProjectManager);
	ProgressDialog *progress_dialog = memnew(ProgressDialog);
	pmanager->add_child(progress_dialog);
	sml->get_root()->add_child(pmanager);

	return sml;
}

MainLoop *run_editor() {
	SceneTree *sml = memnew(SceneTree);

	EditorNode *editor_node = memnew(EditorNode);
	sml->get_root()->add_child(editor_node);

	return sml;
}

String export_preset;
String export_path;
bool export_debug = false;
bool export_pack_only = false;

bool may_export_preset() {
	if (export_preset != "") {
		Error err = EditorNode::get_singleton()->export_preset(export_preset, export_path, export_debug, export_pack_only);
		if (err != OK)
			ERR_PRINT("Failed to export preset");

		return true; // Do not load anything.
	} else {
		return false;
	}
}

void load_scene(const String &p_path) {
	if (!EditorNode::get_singleton()->has_scenes_in_session()) {
		Error err = EditorNode::get_singleton()->load_scene(p_path);
		if (err != OK)
			ERR_PRINT("Failed to load scene");
	}
}

void handle_cmdline(const List<String> &p_args) {
	bool doc_base = true;
	String doc_tool;

	for (int i = 0; i < p_args.size(); i++) {
		if (p_args[i] == "--no-docbase") {
			doc_base = false;
		} else if (i < (p_args.size() - 1)) {
			bool parsed_pair = true;
			if (p_args[i] == "--doctool") {
				doc_tool = p_args[i + 1];
			} else if (p_args[i] == "--export") {
				export_preset = p_args[i + 1];
			} else if (p_args[i] == "--export-debug") {
				export_preset = p_args[i + 1];
				export_debug = true;
			} else if (p_args[i] == "--export-pack") {
				export_preset = p_args[i + 1];
				export_pack_only = true;
			} else {
				parsed_pair = false;
			}
			if (parsed_pair) {
				i++;
			}
		} else if (p_args[i].length() && p_args[i][0] != '-' && export_path == "") {
			export_path = p_args[i];
		}
	}

	if (doc_tool != "") {
		Engine::get_singleton()->set_editor_hint(true); // Needed to instance editor-only classes for their default values

		{
			DirAccessRef da = DirAccess::open(doc_tool);
			ERR_FAIL_COND_MSG(!da, "Argument supplied to --doctool must be a valid directory path.");
		}
		DocData doc;
		doc.generate(doc_base);

		DocData docsrc;
		Map<String, String> doc_data_classes;
		Set<String> checked_paths;
		print_line("Loading docs...");

		for (int i = 0; i < _doc_data_class_path_count; i++) {
			// Custom modules are always located by absolute path.
			String path = _doc_data_class_paths[i].path;
			if (path.is_rel_path()) {
				path = doc_tool.plus_file(path);
			}
			String name = _doc_data_class_paths[i].name;
			doc_data_classes[name] = path;
			if (!checked_paths.has(path)) {
				checked_paths.insert(path);

				// Create the module documentation directory if it doesn't exist
				DirAccess *da = DirAccess::create_for_path(path);
				da->make_dir_recursive(path);
				memdelete(da);

				docsrc.load_classes(path);
				print_line("Loading docs from: " + path);
			}
		}

		String index_path = doc_tool.plus_file("doc/classes");
		// Create the main documentation directory if it doesn't exist
		DirAccess *da = DirAccess::create_for_path(index_path);
		da->make_dir_recursive(index_path);
		memdelete(da);

		docsrc.load_classes(index_path);
		checked_paths.insert(index_path);
		print_line("Loading docs from: " + index_path);

		print_line("Merging docs...");
		doc.merge_from(docsrc);
		for (Set<String>::Element *E = checked_paths.front(); E; E = E->next()) {
			print_line("Erasing old docs at: " + E->get());
			DocData::erase_classes(E->get());
		}

		print_line("Generating new docs...");
		doc.save_classes(index_path, doc_data_classes);
	}
}
} // namespace Editor

MainLoop *editor_main(String p_name, const List<String> &p_args) {
	MainLoop *main_loop = NULL;

	Editor::handle_cmdline(p_args);

	if (p_name == "project_manager") {
		main_loop = Editor::run_project_manger();
	}

	if (p_name == "editor") {
		main_loop = Editor::run_editor();
	}

	if (main_loop) {
		// Hide console window if requested (Windows-only).
		bool hide_console = EditorSettings::get_singleton()->get_setting("interface/editor/hide_console_window");
		OS::get_singleton()->set_console_visible(!hide_console);

		// Load SSL Certificates from Editor Settings (or builtin)
		Crypto::load_default_certificates(EditorSettings::get_singleton()->get_setting("network/ssl/editor_ssl_certificates").operator String());
	}

	return main_loop;
}
