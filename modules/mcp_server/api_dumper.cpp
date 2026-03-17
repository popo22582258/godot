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
	api["version"] = "4.7";
	api["message"] = "API dump not fully implemented - requires Godot API compatibility updates";

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
	class_dict["message"] = "Not implemented";
	return class_dict;
}

Array APIDumper::get_class_hierarchy(const String &class_name) {
	Array hierarchy;
	hierarchy.append(class_name);
	return hierarchy;
}

Array APIDumper::get_builtin_types() {
	Array types;
	Dictionary type_dict;
	type_dict["name"] = "void";
	types.append(type_dict);
	return types;
}

Array APIDumper::get_global_functions() {
	Array functions;
	return functions;
}
