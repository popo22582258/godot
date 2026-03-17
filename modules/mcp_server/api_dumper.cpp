/**************************************************************************/
/*  api_dumper.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "api_dumper.h"

#include "core/io/file_access.h"
#include "core/io/json.h"

Error APIDumper::dump_api_json(const String &output_path, bool include_docs) {
	Dictionary api;

	// Header
	api["version"] = "4.7"; // This would come from actual Godot version
	api["generated_at"] = Time::get_singleton()->get_datetime_string_from_system();

	// Built-in types
	Array builtin_types = get_builtin_types();
	api["builtin_types"] = builtin_types;

	// Global functions
	Array global_functions = get_global_functions();
	api["global_functions"] = global_functions;

	// All classes
	Array classes;
	List<StringName> class_list;
	ClassDB::get_class_list(&class_list);

	class_list.sort();

	for (const StringName &class_name : class_list) {
		Dictionary class_dict = dump_class_api(class_name, include_docs);
		classes.append(class_dict);
	}

	api["classes"] = classes;

	// Write to file
	String json_str = JSON::stringify(api, "\t", false);

	Ref<FileAccess> fa = FileAccess::create_for_path(output_path);
	if (!fa.is_valid()) {
		return ERR_CANT_CREATE;
	}

	fa->store_string(json_str);
	fa->close();

	return OK;
}

Dictionary APIDumper::dump_class_api(const String &class_name, bool include_docs) {
	Dictionary class_dict;

	class_dict["name"] = class_name;

	// Get base class
	StringName base_class = ClassDB::get_parent_class(class_name);
	if (base_class != StringName()) {
		class_dict["inherits"] = base_class;
	}

	// Check if it's core type
	class_dict["is_core_type"] = ClassDB::is_class_exposed(class_name);

	// Get methods
	Array methods;
	List<MethodInfo> method_list;
	ClassDB::get_method_list(class_name, &method_list);

	for (const MethodInfo &method : method_list) {
		if (method.name.begins_with("_")) {
			continue; // Skip internal methods
		}
		methods.append(_get_method_signature_info(method));
	}
	class_dict["methods"] = methods;

	// Get properties
	Array properties;
	List<PropertyInfo> prop_list;
	ClassDB::get_property_list(class_name, &prop_list);

	for (const PropertyInfo &prop : prop_list) {
		if (prop.usage & PROPERTY_USAGE_GROUP) {
			continue;
		}
		if (prop.name.begins_with("_")) {
			continue;
		}
		properties.append(_get_property_signature_info(prop));
	}
	class_dict["properties"] = properties;

	// Get signals
	Array signals;
	List<MethodInfo> signal_list;
	ClassDB::get_signal_list(class_name, &signal_list);

	for (const MethodInfo &signal : signal_list) {
		Dictionary signal_dict;
		signal_dict["name"] = signal.name;

		Array params;
		for (const PropertyInfo &param : signal.arguments) {
			Dictionary param_dict;
			param_dict["name"] = param.name;
			param_dict["type"] = _get_type_name(param.type);
			params.append(param_dict);
		}
		signal_dict["parameters"] = params;

		signals.append(signal_dict);
	}
	class_dict["signals"] = signals;

	// Get constants
	_dump_constants(class_dict, class_name);

	// Get enums
	Array enums;
	HashMap<String, HashMap<String, int>> enums_map = ClassDB::get_enum_list(class_name);
	for (const KeyValue<String, HashMap<String, int>> &E : enums_map) {
		Dictionary enum_dict;
		enum_dict["name"] = E.key;
		_dump_enum_values(enum_dict, E.key, E.value);
		enums.append(enum_dict);
	}
	class_dict["enums"] = enums;

	return class_dict;
}

Array APIDumper::get_class_hierarchy(const String &class_name) {
	Array hierarchy;
	StringName current = class_name;

	while (current != StringName()) {
		hierarchy.append(current);
		current = ClassDB::get_parent_class(current);
	}

	return hierarchy;
}

Array APIDumper::get_builtin_types() {
	Array types;

	for (int i = 0; i < Variant::VARIANT_MAX; i++) {
		Dictionary type_dict;
		type_dict["name"] = Variant::get_type_name((Variant::Type)i);
		type_dict["type_id"] = i;

		// Check if it's a numeric type
		switch (i) {
			case Variant::INT:
			case Variant::FLOAT:
			case Variant::RID:
			case Variant::CALLABLE:
			case Variant::SIGNAL:
			case Variant::STRING:
			case Variant::STRING_NAME:
				type_dict["category"] = "primitive";
				break;
			default:
				type_dict["category"] = "complex";
				break;
		}

		types.append(type_dict);
	}

	return types;
}

Array APIDumper::get_global_functions() {
	Array functions;

	List<StringName> global_classes;
	ClassDB::get_class_list(&global_classes);

	// Get global scope methods (from @GlobalScope)
	List<MethodInfo> method_list;
	ClassDB::get_method_list("@GlobalScope", &method_list);

	for (const MethodInfo &method : method_list) {
		if (method.name.begins_with("_")) {
			continue;
		}
		functions.append(_get_method_signature_info(method));
	}

	return functions;
}

void APIDumper::_dump_method_info(Dictionary &class_dict, const MethodInfo &p_method, const String &p_name) {
	// This is handled by _get_method_signature_info
}

void APIDumper::_dump_property_info(Dictionary &class_dict, const PropertyInfo &p_property, const String &p_name) {
	// This is handled by _get_property_signature_info
}

void APIDumper::_dump_signal_info(Dictionary &class_dict, const MethodInfo &p_signal, const String &p_name) {
	// This is handled in dump_class_api
}

void APIDumper::_dump_constants(Dictionary &class_dict, const String &class_name) {
	Array constants;

	List<String> constants_list;
	ClassDB::get_integer_constant_list(class_name, &constants_list);

	for (const String &constant : constants_list) {
		Dictionary const_dict;
		const_dict["name"] = constant;
		constants.append(const_dict);
	}

	class_dict["constants"] = constants;
}

void APIDumper::_dump_enum_values(Dictionary &class_dict, const String &enum_name, const HashMap<String, int> &values) {
	Array enum_values;

	for (const KeyValue<String, int> &E : values) {
		Dictionary value_dict;
		value_dict["name"] = E.key;
		value_dict["value"] = E.value;
		enum_values.append(value_dict);
	}

	class_dict["values"] = enum_values;
}

Dictionary APIDumper::_get_method_signature_info(const MethodInfo &p_method) {
	Dictionary method_dict;

	method_dict["name"] = p_method.name;
	method_dict["return_type"] = _get_type_name(p_method.return_type);

	// Return type hints
	if (p_method.return_val.flags & PROPERTY_FLAG_NONE) {
		// No special flags
	} else if (p_method.return_val.hint == PROPERTY_HINT_METHOD_OF_TYPE) {
		method_dict["return_hint"] = p_method.return_val.hint_string;
	}

	// Parameters
	Array parameters;
	for (const PropertyInfo &param : p_method.arguments) {
		Dictionary param_dict;
		param_dict["name"] = param.name;
		param_dict["type"] = _get_type_name(param.type);

		if (!param.hint_string.is_empty()) {
			param_dict["hint"] = param.hint_string;
		}

		parameters.append(param_dict);
	}
	method_dict["parameters"] = parameters;

	// Flags
	method_dict["is_virtual"] = p_method.flags & METHOD_FLAG_VIRTUAL;
	method_dict["is_static"] = p_method.flags & METHOD_FLAG_STATIC;
	method_dict["is_const"] = p_method.flags & METHOD_FLAG_CONST;
	method_dict["is_vararg"] = p_method.flags & METHOD_FLAG_VARARG;

	return method_dict;
}

Dictionary APIDumper::_get_property_signature_info(const PropertyInfo &p_property) {
	Dictionary prop_dict;

	prop_dict["name"] = p_property.name;
	prop_dict["type"] = _get_type_name(p_property.type);

	if (!p_property.hint_string.is_empty()) {
		prop_dict["hint"] = p_property.hint_string;
	}

	if (p_property.usage & PROPERTY_USAGE_EXPORT) {
		prop_dict["is_exported"] = true;
	}

	if (p_property.usage & PROPERTY_USAGE_STORAGE) {
		prop_dict["is_storage"] = true;
	}

	return prop_dict;
}

String APIDumper::_get_type_name(Variant::Type p_type) {
	return Variant::get_type_name(p_type);
}

String APIDumper::_get_method_hint_string(const PropertyInfo &p_prop) {
	return p_prop.hint_string;
}
