/*************************************************************************/
/*  virtual_node_manager.h                                               */
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

#ifndef VIRTUAL_NODE_MANAGER_H
#define VIRTUAL_NODE_MANAGER_H

#include "core/object.h"
#include "core/os/semaphore.h"
#include "core/os/thread.h"
#include "core/safe_refcount.h"
#include "scene/main/node.h"

class VirtualNodeManager : public Object {
	GDCLASS(VirtualNodeManager, Object)

	static VirtualNodeManager *singleton;

private:
	struct QueueItem {
		enum Type {
			TYPE_CALL,
			TYPE_NOTIFICATION,
			TYPE_SET,
		};

		Type type;
		ObjectID instance_id;
		StringName target;
		Variant args[VARIANT_ARG_MAX];
	};

	List<QueueItem> queue;

	Mutex preview_mutex;
	Semaphore preview_sem;
	Thread thread;
	SafeFlag exit;
	SafeFlag exited;

	static void _thread_func(void *ud);
	void _thread();

protected:
	static void _bind_methods();

public:
	static VirtualNodeManager *get_singleton();

	void node_add_child(Node *p_node, Node *p_child);
	void node_queue_free(Node *p_node);

	void node_notification(Node *p_node, int p_what);
	void node_set_property(Node *p_node, const StringName &p_prop, const Variant &p_value);
	void node_call_function(Node *p_node, const StringName &p_method, const Variant **p_args, int p_argcount);
	Variant node_call_function_bind(const Variant **p_args, int p_argcount, Variant::CallError &r_error);

	void start();
	void stop();

	VirtualNodeManager();
	~VirtualNodeManager();
};

#endif
