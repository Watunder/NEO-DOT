/*************************************************************************/
/*  editor_undo_redo_manager.h                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef EDITOR_UNDO_REDO_MANAGER_H
#define EDITOR_UNDO_REDO_MANAGER_H

#include "core/class_db.h"
#include "core/reference.h"
#include "core/undo_redo.h"

class EditorUndoRedoManager : public Reference {
	GDCLASS(EditorUndoRedoManager, Reference);

public:
	enum SpecialHistory {
		GLOBAL_HISTORY = 0,
		INVALID_HISTORY = -99,
	};

private:
	struct Action {
		int history_id = INVALID_HISTORY;
		double timestamp = 0;
		String action_name;
		UndoRedo::MergeMode merge_mode = UndoRedo::MERGE_DISABLE;
	};

	struct History {
		int id = INVALID_HISTORY;
		UndoRedo *undo_redo = nullptr;
		uint64_t saved_version = 1;
		List<Action> undo_stack;
		List<Action> redo_stack;
	};

	HashMap<int, History> history_map;
	Action pending_action;

	bool is_committing = false;

protected:
	static void _bind_methods();

public:
	History &get_or_create_history(int p_idx);
	UndoRedo *get_history_undo_redo(int p_idx) const;
	int get_history_id_for_object(Object *p_object) const;
	History &get_history_for_object(Object *p_object);

	void create_action_for_history(const String &p_name, int p_history_id, UndoRedo::MergeMode p_mode = UndoRedo::MERGE_DISABLE);
	void create_action(const String &p_name = "", UndoRedo::MergeMode p_mode = UndoRedo::MERGE_DISABLE, Object *p_custom_context = nullptr);

	void add_do_method(Object *p_object, const String &p_method, VARIANT_ARG_LIST);
	void add_undo_method(Object *p_object, const String &p_method, VARIANT_ARG_LIST);

	Variant _add_do_method(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	Variant _add_undo_method(const Variant **p_args, int p_argcount, Variant::CallError &r_error);

	void add_do_property(Object *p_object, const String &p_property, const Variant &p_value);
	void add_undo_property(Object *p_object, const String &p_property, const Variant &p_value);
	void add_do_reference(Object *p_object);
	void add_undo_reference(Object *p_object);

	void commit_action();
	bool is_committing_action() const;

	bool undo();
	bool redo();
	void clear_history(bool p_increase_version = true, int p_idx = INVALID_HISTORY);

	void set_history_as_saved(int p_idx);
	void set_history_as_unsaved(int p_idx);
	bool is_history_unsaved(int p_idx);
	bool has_undo();
	bool has_redo();

	String get_current_action_name();

	void discard_history(int p_idx, bool p_erase_from_map = true);
	~EditorUndoRedoManager();
};

VARIANT_ENUM_CAST(EditorUndoRedoManager::SpecialHistory);

#endif // EDITOR_UNDO_REDO_MANAGER_H
