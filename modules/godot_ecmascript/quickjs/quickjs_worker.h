/*************************************************************************/
/*  quickjs_worker.h                                                     */
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

#ifndef QUICKJS_WORKER_H
#define QUICKJS_WORKER_H

#include "core/os/thread.h"
#include "quickjs_binder.h"

class QuickJSWorker : public QuickJSBinder {
	Thread thread;
	bool running = false;
	static void thread_main(void *p_self);
	String entry_script;

	const QuickJSBinder *host_context;
	List<Variant> input_message_queue;
	List<Variant> output_message_queue;

	static JSValue global_worker_close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
	static JSValue global_worker_post_message(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
	static JSValue global_import_scripts(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);

public:
	QuickJSWorker(const QuickJSBinder *p_host_context);
	virtual ~QuickJSWorker();

	virtual void initialize();
	virtual void uninitialize();

	bool frame_of_host(QuickJSBinder *host, const JSValueConst &value);
	void post_message_from_host(const Variant &p_message);
	void start(const String &p_path);
	void stop();
};

#endif // QUICKJS_WORKER_H
