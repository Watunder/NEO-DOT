/*************************************************************************/
/*  quickjs_debugger.h                                                   */
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

#ifndef QUICKJSDEBUGGER_H
#define QUICKJSDEBUGGER_H

#include "core/io/stream_peer_tcp.h"
#include "core/io/tcp_server.h"
#include "core/reference.h"
#include "quickjs/quickjs-debugger.h"
#define QJS_DEBUGGER_MAX_BUFFER_SIZE 4194304

/* GODOT ADDITTION */
#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif
/* GODOT END */

class QuickJSDebugger : public Reference {
	GDCLASS(QuickJSDebugger, Reference)

	Ref<StreamPeerTCP> peer;
	Ref<TCP_Server> server;
	JSRuntime *runtime;
	JSContext *ctx;
	uint8_t request_buffer[QJS_DEBUGGER_MAX_BUFFER_SIZE];

	struct ConnectionConfig {
		IP_Address address;
		uint16_t port;
	};

	ConnectionConfig parse_address(const String &address);

	static size_t transport_read(void *udata, char *buffer, size_t length);
	static size_t transport_write(void *udata, const char *buffer, size_t length);
	static size_t transport_peek(void *udata);
	static void transport_close(JSRuntime *rt, void *udata);

	Error attach_js_debugger(JSContext *p_ctx, Ref<StreamPeerTCP> p_peer);

public:
	Error connect(JSContext *ctx, const String &address);
	Error listen(JSContext *ctx, const String &address);
	void poll();

	QuickJSDebugger();
	~QuickJSDebugger();
};

#endif // QUICKJSDEBUGGER_H
