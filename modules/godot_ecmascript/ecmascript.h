#ifndef ECMASCRIPT_H
#define ECMASCRIPT_H

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/script_language.h"
#include "ecmascript_binder.h"
#include "scene/resources/text_file.h"

#define EXT_JSCLASS "mjs"
#define EXT_JSCLASS_BYTECODE EXT_JSCLASS "b"
#define EXT_JSCLASS_ENCRYPTED EXT_JSCLASS "e"
#define EXT_JSMODULE "js"
#define EXT_JSMODULE_BYTECODE EXT_JSMODULE "b"
#define EXT_JSMODULE_ENCRYPTED EXT_JSMODULE "e"
#define EXT_JSON "json"

class ECMAScript : public Script {

	GDCLASS(ECMAScript, Script);

private:
	friend class ECMAScriptInstance;
	friend class QuickJSBinder;
	friend class ResourceFormatLoaderECMAScript;

	Set<Object *> instances;
	StringName class_name;
	String code;
	String script_path;
	Vector<uint8_t> bytecode;

	const BasicECMAClassInfo *ecma_class;

#ifdef TOOLS_ENABLED
	Set<PlaceHolderScriptInstance *> placeholders;
	virtual void _placeholder_erased(PlaceHolderScriptInstance *p_placeholder);
#endif

protected:
	void _notification(int p_what) {}
	static void _bind_methods();

public:
	virtual bool can_instance() const;

	virtual bool inherits_script(const Ref<Script> &p_script) const { return false; }
	virtual Ref<Script> get_base_script() const { return NULL; } //for script inheritance
	virtual StringName get_instance_base_type() const;

	virtual ScriptInstance *instance_create(Object *p_this);
	virtual bool instance_has(const Object *p_this) const;

	virtual PlaceHolderScriptInstance *placeholder_instance_create(Object *p_this);
	virtual bool is_placeholder_fallback_enabled() const;

	virtual bool has_source_code() const { return true; }
	virtual String get_source_code() const { return code; }

	virtual void set_source_code(const String &p_code) { code = p_code; }
	virtual Error reload(bool p_keep_state = true);

	virtual bool has_method(const StringName &p_method) const;
	virtual MethodInfo get_method_info(const StringName &p_method) const;

	virtual bool is_tool() const;
	virtual bool is_valid() const;

	virtual void get_script_method_list(List<MethodInfo> *p_list) const;
	virtual void get_script_property_list(List<PropertyInfo> *p_list) const;
	virtual bool get_property_default_value(const StringName &p_property, Variant &r_value) const;

	virtual bool has_script_signal(const StringName &p_signal) const;
	virtual void get_script_signal_list(List<MethodInfo> *r_signals) const;

	virtual void update_exports(); //editor tool

	/* TODO */ virtual int get_member_line(const StringName &p_member) const { return -1; }
	/* TODO */ virtual void get_constants(Map<StringName, Variant> *p_constants) {}
	/* TODO */ virtual void get_members(Set<StringName> *p_constants) {}

	virtual ScriptLanguage *get_language() const;

	_FORCE_INLINE_ String get_script_path() const { return script_path; }
	_FORCE_INLINE_ void set_script_path(const String &p_path) { script_path = p_path; }

	ECMAScript();
	virtual ~ECMAScript();
};

class ECMAScriptModule : public TextFile {

	GDCLASS(ECMAScriptModule, Resource);

protected:
	static void _bind_methods();
	String script_path;
	Vector<uint8_t> bytecode;

public:
	_FORCE_INLINE_ void set_source_code(String p_source_code) { TextFile::set_text(p_source_code); }
	_FORCE_INLINE_ String get_source_code() const { return TextFile::get_text(); }
	_FORCE_INLINE_ void set_bytecode(Vector<uint8_t> p_bytecode) { bytecode = p_bytecode; }
	_FORCE_INLINE_ Vector<uint8_t> get_bytecode() const { return bytecode; }
	_FORCE_INLINE_ void set_script_path(String p_script_path) { script_path = p_script_path; }
	_FORCE_INLINE_ String get_script_path() const { return script_path; }
	ECMAScriptModule();
};

class ResourceFormatLoaderECMAScript : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual void get_recognized_extensions_for_type(const String &p_type, List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};

class ResourceFormatSaverECMAScript : public ResourceFormatSaver {
public:
	virtual Error save(const String &p_path, const RES &p_resource, uint32_t p_flags = 0);
	virtual void get_recognized_extensions(const RES &p_resource, List<String> *p_extensions) const;
	virtual bool recognize(const RES &p_resource) const;
};

class ResourceFormatLoaderECMAScriptModule : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual void get_recognized_extensions_for_type(const String &p_type, List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;

	static RES load_static(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
};

class ResourceFormatSaverECMAScriptModule : public ResourceFormatSaver {
public:
	virtual Error save(const String &p_path, const RES &p_resource, uint32_t p_flags = 0);
	virtual void get_recognized_extensions(const RES &p_resource, List<String> *p_extensions) const;
	virtual bool recognize(const RES &p_resource) const;
};

#endif // ECMASCRIPT_H
