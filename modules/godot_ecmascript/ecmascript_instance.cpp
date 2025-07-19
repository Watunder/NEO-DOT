/*************************************************************************/
/*  ecmascript_instance.cpp                                              */
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

#include "ecmascript_instance.h"
#include "ecmascript.h"
#include "ecmascript_language.h"

Ref<Script> ECMAScriptInstance::get_script() const {
	return script;
}

void ECMAScriptInstance::get_method_list(List<MethodInfo> *p_list) const {
	if (!ecma_class)
		return;
	const StringName *key = ecma_class->methods.next(NULL);
	while (key) {
		p_list->push_back(ecma_class->methods.get(*key));
		key = ecma_class->methods.next(key);
	}
}

bool ECMAScriptInstance::has_method(const StringName &p_method) const {
	if (!binder || !ecma_object.ecma_object)
		return false;
	return binder->has_method(ecma_object, p_method);
}

bool ECMAScriptInstance::set(const StringName &p_name, const Variant &p_value) {
	if (!binder || !ecma_object.ecma_object)
		return false;
	return binder->set_instance_property(ecma_object, p_name, p_value);
}

bool ECMAScriptInstance::get(const StringName &p_name, Variant &r_ret) const {
	if (!binder || !ecma_object.ecma_object)
		return false;
	return binder->get_instance_property(this->ecma_object, p_name, r_ret);
}

void ECMAScriptInstance::get_property_list(List<PropertyInfo> *p_properties) const {
	if (!ecma_class)
		return;
	for (const StringName *name = ecma_class->properties.next(NULL); name; name = ecma_class->properties.next(name)) {
		const ECMAProperyInfo &pi = ecma_class->properties.get(*name);
		p_properties->push_back(pi);
	}
}

Variant::Type ECMAScriptInstance::get_property_type(const StringName &p_name, bool *r_is_valid) const {
	*r_is_valid = false;
	if (ecma_class) {
		if (const ECMAProperyInfo *pi = ecma_class->properties.getptr(p_name)) {
			*r_is_valid = true;
			return pi->type;
		}
	}
	return Variant::NIL;
}

Variant ECMAScriptInstance::call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
	if (binder == NULL || ecma_object.ecma_object == NULL) {
		r_error.error = Variant::CallError::CALL_ERROR_INSTANCE_IS_NULL;
		ERR_FAIL_V(Variant());
	}
	return binder->call_method(ecma_object, p_method, p_args, p_argcount, r_error);
}

ScriptLanguage *ECMAScriptInstance::get_language() {
	return ECMAScriptLanguage::get_singleton();
}

ECMAScriptInstance::ECMAScriptInstance() {
	owner = NULL;
	binder = NULL;
	ecma_class = NULL;
}

ECMAScriptInstance::~ECMAScriptInstance() {
	if (script.is_valid() && owner) {
		script->instances.erase(owner);
	}
}
