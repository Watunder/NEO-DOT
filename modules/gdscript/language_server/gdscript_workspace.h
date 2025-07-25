/*************************************************************************/
/*  gdscript_workspace.h                                                 */
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

#ifndef GDSCRIPT_WORKSPACE_H
#define GDSCRIPT_WORKSPACE_H

#include "../gdscript_parser.h"
#include "core/variant.h"
#include "editor/editor_file_system.h"
#include "gdscript_extend_parser.h"
#include "lsp.hpp"

class GDScriptWorkspace : public Reference {
	GDCLASS(GDScriptWorkspace, Reference);

private:
	void _get_owners(EditorFileSystemDirectory *efsd, String p_path, List<String> &owners);
	Node *_get_owner_scene_node(String p_path);

protected:
	static void _bind_methods();
	void remove_cache_parser(const String &p_path);
	bool initialized = false;
	Map<StringName, lsp::DocumentSymbol> native_symbols;

	const lsp::DocumentSymbol *get_native_symbol(const String &p_class, const String &p_member = "") const;
	const lsp::DocumentSymbol *get_script_symbol(const String &p_path) const;

	void reload_all_workspace_scripts();

	ExtendGDScriptParser *get_parse_successed_script(const String &p_path);
	ExtendGDScriptParser *get_parse_result(const String &p_path);

	void list_script_files(const String &p_root_dir, List<String> &r_files);

public:
	String root;
	String root_uri;

	Map<String, ExtendGDScriptParser *> scripts;
	Map<String, ExtendGDScriptParser *> parse_results;
	HashMap<StringName, ClassMembers> native_members;

public:
	Array symbol(const Dictionary &p_params);

public:
	Error initialize();

	Error parse_script(const String &p_path, const String &p_content);
	Error parse_local_script(const String &p_path);

	String get_file_path(const String &p_uri) const;
	String get_file_uri(const String &p_path) const;

	void publish_diagnostics(const String &p_path);
	void completion(const lsp::CompletionParams &p_params, List<ScriptCodeCompletionOption> *r_options);

	const lsp::DocumentSymbol *resolve_symbol(const lsp::TextDocumentPositionParams &p_doc_pos, const String &p_symbol_name = "", bool p_func_required = false);
	void resolve_related_symbols(const lsp::TextDocumentPositionParams &p_doc_pos, List<const lsp::DocumentSymbol *> &r_list);
	const lsp::DocumentSymbol *resolve_native_symbol(const lsp::NativeSymbolInspectParams &p_params);
	void resolve_document_links(const String &p_uri, List<lsp::DocumentLink> &r_list);
	Dictionary generate_script_api(const String &p_path);
	Error resolve_signature(const lsp::TextDocumentPositionParams &p_doc_pos, lsp::SignatureHelp &r_signature);

	void did_delete_files(const Dictionary &p_params);

	GDScriptWorkspace();
	~GDScriptWorkspace();
};

#endif
