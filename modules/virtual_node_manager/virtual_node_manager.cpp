/*************************************************************************/
/*  virtual_node_manager.cpp                                             */
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

#include "virtual_node_manager.h"
#include "core/message_queue.h"
#include "core/os/os.h"

VirtualNodeManager *VirtualNodeManager::singleton = NULL;

VirtualNodeManager *VirtualNodeManager::get_singleton() {
	return singleton;
}

void VirtualNodeManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("node_add_child", "node", "child"), &VirtualNodeManager::node_add_child);
	ClassDB::bind_method(D_METHOD("node_queue_free", "node"), &VirtualNodeManager::node_queue_free);

	ClassDB::bind_method(D_METHOD("node_notification", "node", "what"), &VirtualNodeManager::node_notification);
	ClassDB::bind_method(D_METHOD("node_set_property", "node", "prop", "value"), &VirtualNodeManager::node_set_property);

	{
		MethodInfo mi;
		mi.name = "node_call_function";
		mi.arguments.push_back(PropertyInfo(Variant::OBJECT, "node", PROPERTY_HINT_NODE_TYPE, "Node"));
		mi.arguments.push_back(PropertyInfo(Variant::STRING, "method"));

		ClassDB::bind_vararg_method(METHOD_FLAGS_DEFAULT, "node_call_function", &VirtualNodeManager::node_call_function_bind, mi, varray(), false);
	}
}

void VirtualNodeManager::node_add_child(Node *p_node, Node *p_child) {
	ERR_FAIL_NULL(p_node);
	ERR_FAIL_NULL(p_child);

	const Variant args[1] = { p_child };
	const Variant *argptrs[1] = { &args[0] };
	node_call_function(p_node, "add_child", (const Variant **)argptrs, 1);
}

void VirtualNodeManager::node_queue_free(Node *p_node) {
	ERR_FAIL_NULL(p_node);

	node_call_function(p_node, "queue_free", NULL, 0);
}

void VirtualNodeManager::node_notification(Node *p_node, int p_what) {
	ERR_FAIL_NULL(p_node);

	preview_mutex.lock();

	QueueItem item;
	item.type = QueueItem::TYPE_NOTIFICATION;
	item.instance_id = p_node->get_instance_id();
	item.args[0] = p_what;

	queue.push_back(item);
	preview_mutex.unlock();
	preview_sem.post();
}

void VirtualNodeManager::node_set_property(Node *p_node, const StringName &p_prop, const Variant &p_value) {
	ERR_FAIL_NULL(p_node);

	preview_mutex.lock();

	QueueItem item;
	item.type = QueueItem::TYPE_SET;
	item.instance_id = p_node->get_instance_id();
	item.target = p_prop;
	item.args[0] = p_value;

	queue.push_back(item);
	preview_mutex.unlock();
	preview_sem.post();
}

void VirtualNodeManager::node_call_function(Node *p_node, const StringName &p_method, const Variant **p_args, int p_argcount) {
	ERR_FAIL_NULL(p_node);

	preview_mutex.lock();

	QueueItem item;
	item.type = QueueItem::TYPE_CALL;
	item.instance_id = p_node->get_instance_id();
	item.target = p_method;
	for (int i = 0; i < MIN(p_argcount, VARIANT_ARG_MAX); i++) {
		item.args[i] = *p_args[i];
	}

	queue.push_back(item);
	preview_mutex.unlock();
	preview_sem.post();
}

Variant VirtualNodeManager::node_call_function_bind(const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	if (p_argcount < 2) {
		r_error.error = Variant::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS;
		r_error.argument = 0;
		return Variant();
	}

	if (p_args[0]->get_type() != Variant::OBJECT) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = 0;
		r_error.expected = Variant::OBJECT;
		return Variant();
	}

	if (p_args[1]->get_type() != Variant::STRING) {
		r_error.error = Variant::CallError::CALL_ERROR_INVALID_ARGUMENT;
		r_error.argument = 1;
		r_error.expected = Variant::STRING;
		return Variant();
	}

	r_error.error = Variant::CallError::CALL_OK;

	Object *object = *p_args[0];
	Node *node = Object::cast_to<Node>(object);
	StringName method = *p_args[1];
	const Variant **args = &p_args[2];
	int argcount = p_argcount - 2;

	node_call_function(node, method, args, argcount);
	return Variant();
}

void VirtualNodeManager::_thread_func(void *ud) {
	VirtualNodeManager *erp = (VirtualNodeManager *)ud;
	erp->_thread();
}

void VirtualNodeManager::_thread() {
	exited.clear();
	while (!exit.is_set()) {
		preview_sem.wait();
		preview_mutex.lock();

		if (queue.size()) {
			QueueItem item = queue.front()->get();
			queue.pop_front();

			preview_mutex.unlock();

			switch (item.type) {
				case QueueItem::TYPE_CALL: {
					MessageQueue::get_singleton()->push_call(item.instance_id, item.target,
							item.args[0], item.args[1], item.args[2], item.args[3], item.args[4]);
				} break;
				case QueueItem::TYPE_NOTIFICATION: {
					MessageQueue::get_singleton()->push_notification(item.instance_id, item.args[0].operator int());
				} break;
				case QueueItem::TYPE_SET: {
					MessageQueue::get_singleton()->push_set(item.instance_id, item.target, item.args[0]);
				} break;
			}
		} else {
			preview_mutex.unlock();
		}
	}
	exited.set();
}

void VirtualNodeManager::start() {
	ERR_FAIL_COND_MSG(thread.is_started(), "Thread already started.");
	thread.start(_thread_func, this);
}

void VirtualNodeManager::stop() {
	if (thread.is_started()) {
		exit.set();
		preview_sem.post();
		while (!exited.is_set()) {
			OS::get_singleton()->delay_usec(10000);
			VisualServer::get_singleton()->sync(); //sync pending stuff, as thread may be blocked on visual server
		}
		thread.wait_to_finish();
	}
}

VirtualNodeManager::VirtualNodeManager() {
	singleton = this;
}

VirtualNodeManager::~VirtualNodeManager() {
	stop();
}
