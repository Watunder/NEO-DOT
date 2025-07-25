/*************************************************************************/
/*  quickjs_builtin_binder.h                                             */
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

#ifndef QUICKJS_BUILTIN_BINDER_H
#define QUICKJS_BUILTIN_BINDER_H

#include "quickjs/quickjs.h"
#include <core/variant.h>
struct ECMAScriptGCHandler;
class QuickJSBinder;

typedef JSValue (*JSConstructorFunc)(JSContext *ctx, JSValueConst new_target, int argc, JSValueConst *argv);
typedef void (*JSFinalizerFunc)(JSRuntime *rt, JSValue val);

class QuickJSBuiltinBinder {
public:
	struct BuiltinClass {
		JSClassID id;
		JSValue class_function;
		JSValue class_prototype;
		JSClassDef js_class;
	};

private:
	QuickJSBinder *binder;
	JSContext *ctx;
	BuiltinClass *builtin_class_map;
	JSValue to_string_function;
	JSAtom js_key_to_string;

public:
	void builtin_finalizer(ECMAScriptGCHandler *p_bind);

	void register_builtin_class(Variant::Type p_type, const char *p_name, JSConstructorFunc p_constructor, int argc);
	void register_property(Variant::Type p_type, const char *p_name, JSCFunctionMagic *p_getter, JSCFunctionMagic *p_setter, int magic);
	void register_operators(Variant::Type p_type, Vector<JSValue> &p_operators);
	void register_method(Variant::Type p_type, const char *p_name, JSCFunction *p_func, int p_length);
	void register_constant(Variant::Type p_type, const char *p_name, const Variant &p_value);
	void get_cross_type_operators(Variant::Type p_type, Vector<JSValue> &r_operators);

public:
	QuickJSBuiltinBinder();
	~QuickJSBuiltinBinder();

	void initialize(JSContext *p_context, QuickJSBinder *p_binder);
	void uninitialize();

	void bind_builtin_classes_gen();
	void bind_builtin_propties_manually();

	_FORCE_INLINE_ void set_classid(Variant::Type p_type, JSClassID p_id) { builtin_class_map[p_type].id = p_id; }
	_FORCE_INLINE_ JSClassID get_classid(Variant::Type p_type) { return builtin_class_map[p_type].id; }
	_FORCE_INLINE_ BuiltinClass &get_class(Variant::Type p_type) { return *(builtin_class_map + p_type); }

	static void bind_builtin_object(JSContext *ctx, JSValue target, Variant::Type p_type, const void *p_object);
	static JSValue create_builtin_value(JSContext *ctx, Variant::Type p_type, const void *p_val);
	static JSValue new_object_from(JSContext *ctx, const Variant &p_val);
	static JSValue new_object_from(JSContext *ctx, const Vector2 &p_val);
	static JSValue new_object_from(JSContext *ctx, const Vector3 &p_val);
	static JSValue new_object_from(JSContext *ctx, const Rect2 &p_val);
	static JSValue new_object_from(JSContext *ctx, const Color &p_val);
	static JSValue new_object_from(JSContext *ctx, const Transform2D &p_val);
	static JSValue new_object_from(JSContext *ctx, const Transform &p_val);
	static JSValue new_object_from(JSContext *ctx, const Quat &p_val);
	static JSValue new_object_from(JSContext *ctx, const Plane &p_val);
	static JSValue new_object_from(JSContext *ctx, const RID &p_val);
	static JSValue new_object_from(JSContext *ctx, const AABB &p_val);
	static JSValue new_object_from(JSContext *ctx, const Basis &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolIntArray &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolByteArray &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolRealArray &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolColorArray &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolStringArray &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolVector2Array &p_val);
	static JSValue new_object_from(JSContext *ctx, const PoolVector3Array &p_val);
};

#endif // QUICKJS_BUILTIN_BINDER_H
