/**************************************************************************/
/*  api_dumper.h                                                            */
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

#pragma once

#include "core/object/class_db.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class APIDumper {
public:
	// Dump all Godot API to JSON
	static Error dump_api_json(const String &output_path, bool include_docs = true);

	// Dump specific class API
	static Dictionary dump_class_api(const String &class_name, bool include_docs = true);

	// Get class hierarchy
	static Array get_class_hierarchy(const String &class_name);

	// Get all built-in types
	static Array get_builtin_types();

	// Get global scope functions
	static Array get_global_functions();

private:
	static void _dump_method_info(Dictionary &class_dict, const MethodInfo &p_method, const String &p_name);
	static void _dump_property_info(Dictionary &class_dict, const PropertyInfo &p_property, const String &p_name);
	static void _dump_signal_info(Dictionary &class_dict, const MethodInfo &p_signal, const String &p_name);
	static void _dump_constants(Dictionary &class_dict, const String &class_name);
	static void _dump_enum_values(Dictionary &class_dict, const String &enum_name, const HashMap<String, int> &values);

	static Dictionary _get_method_signature_info(const MethodInfo &p_method);
	static Dictionary _get_property_signature_info(const PropertyInfo &p_property);

	static String _get_type_name(Variant::Type p_type);
	static String _get_method_hint_string(const PropertyInfo &p_prop);
};
