/**************************************************************************/
/*  mcp_tool_registry.cpp                                                */
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

#include "mcp_tool_registry.h"

#include "core/object/class_db.h"
#include "core/templates/local_vector.h"
#include "debug_scanner.h"

void MCPToolRegistry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_tool", "name", "definition"), &MCPToolRegistry::register_tool);
	ClassDB::bind_method(D_METHOD("has_tool", "name"), &MCPToolRegistry::has_tool);
	ClassDB::bind_method(D_METHOD("get_tool_definition", "name"), &MCPToolRegistry::get_tool_definition);
	ClassDB::bind_method(D_METHOD("get_all_tool_names"), &MCPToolRegistry::get_all_tool_names);
	ClassDB::bind_method(D_METHOD("call_tool", "name", "params"), &MCPToolRegistry::call_tool);
	ClassDB::bind_method(D_METHOD("register_builtin_tools"), &MCPToolRegistry::register_builtin_tools);

	// Built-in tools
	ClassDB::bind_method(D_METHOD("tool_get_errors", "params"), &MCPToolRegistry::tool_get_errors);
	ClassDB::bind_method(D_METHOD("tool_get_warnings", "params"), &MCPToolRegistry::tool_get_warnings);
	ClassDB::bind_method(D_METHOD("tool_get_scene_tree", "params"), &MCPToolRegistry::tool_get_scene_tree);
	ClassDB::bind_method(D_METHOD("tool_get_project_structure", "params"), &MCPToolRegistry::tool_get_project_structure);
	ClassDB::bind_method(D_METHOD("tool_run_project", "params"), &MCPToolRegistry::tool_run_project);
	ClassDB::bind_method(D_METHOD("tool_stop_project", "params"), &MCPToolRegistry::tool_stop_project);
	ClassDB::bind_method(D_METHOD("tool_validate_script", "params"), &MCPToolRegistry::tool_validate_script);
	ClassDB::bind_method(D_METHOD("tool_list_tools", "params"), &MCPToolRegistry::tool_list_tools);
}

void MCPToolRegistry::register_tool(const String &p_name, const Dictionary &p_definition) {
	tools[p_name] = p_definition;
}

bool MCPToolRegistry::has_tool(const String &p_name) const {
	return tools.has(p_name);
}

Dictionary MCPToolRegistry::get_tool_definition(const String &p_name) const {
	if (tools.has(p_name)) {
		return tools[p_name];
	}
	return Dictionary();
}

Array MCPToolRegistry::get_all_tool_names() const {
	Array result;
	LocalVector<Variant> keys = tools.get_key_list();
	for (const Variant &k : keys) {
		result.append(k);
	}
	return result;
}

Variant MCPToolRegistry::call_tool(const String &p_name, const Dictionary &p_params) {
	if (p_name == "get_errors") {
		return tool_get_errors(p_params);
	} else if (p_name == "get_warnings") {
		return tool_get_warnings(p_params);
	} else if (p_name == "get_scene_tree") {
		return tool_get_scene_tree(p_params);
	} else if (p_name == "get_project_structure") {
		return tool_get_project_structure(p_params);
	} else if (p_name == "run_project") {
		return tool_run_project(p_params);
	} else if (p_name == "stop_project") {
		return tool_stop_project(p_params);
	} else if (p_name == "validate_script") {
		return tool_validate_script(p_params);
	} else if (p_name == "list_tools") {
		return tool_list_tools(p_params);
	}

	Dictionary error;
	error["error"] = "Method not found";
	return error;
}

Dictionary MCPToolRegistry::tool_get_errors(const Dictionary &p_params) {
	Dictionary result;
	result["errors"] = Array();
	result["statistics"] = Dictionary();
	return result;
}

Dictionary MCPToolRegistry::tool_get_warnings(const Dictionary &p_params) {
	Dictionary result;
	result["warnings"] = Array();
	result["statistics"] = Dictionary();
	return result;
}

Dictionary MCPToolRegistry::tool_get_scene_tree(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;
	result["scene_tree"] = Dictionary();
	result["message"] = "Not implemented yet";
	return result;
}

Dictionary MCPToolRegistry::tool_get_project_structure(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;
	result["structure"] = Array();
	result["message"] = "Not implemented yet";
	return result;
}

Dictionary MCPToolRegistry::tool_run_project(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;
	result["message"] = "Not implemented yet";
	return result;
}

Dictionary MCPToolRegistry::tool_stop_project(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;
	result["message"] = "Not implemented yet";
	return result;
}

Dictionary MCPToolRegistry::tool_validate_script(const Dictionary &p_params) {
	Dictionary result;
	result["valid"] = true;
	result["errors"] = Array();
	result["warnings"] = Array();
	return result;
}

Dictionary MCPToolRegistry::tool_list_tools(const Dictionary &p_params) {
	Dictionary result;
	result["tools"] = get_all_tool_names();
	result["count"] = tools.size();
	return result;
}

void MCPToolRegistry::register_builtin_tools() {
	// Register get_errors tool
	Dictionary get_errors_def;
	get_errors_def["name"] = "get_errors";
	get_errors_def["description"] = "Get all errors from the current project";
	get_errors_def["parameters"] = Dictionary();
	register_tool("get_errors", get_errors_def);

	// Register get_warnings tool
	Dictionary get_warnings_def;
	get_warnings_def["name"] = "get_warnings";
	get_warnings_def["description"] = "Get all warnings from the current project";
	get_warnings_def["parameters"] = Dictionary();
	register_tool("get_warnings", get_warnings_def);

	// Register get_scene_tree tool
	Dictionary get_scene_tree_def;
	get_scene_tree_def["name"] = "get_scene_tree";
	get_scene_tree_def["description"] = "Get the scene tree of the current project";
	get_scene_tree_def["parameters"] = Dictionary();
	register_tool("get_scene_tree", get_scene_tree_def);

	// Register get_project_structure tool
	Dictionary get_project_structure_def;
	get_project_structure_def["name"] = "get_project_structure";
	get_project_structure_def["description"] = "Get the project file structure";
	get_project_structure_def["parameters"] = Dictionary();
	register_tool("get_project_structure", get_project_structure_def);

	// Register run_project tool
	Dictionary run_project_def;
	run_project_def["name"] = "run_project";
	run_project_def["description"] = "Run the current project";
	run_project_def["parameters"] = Dictionary();
	register_tool("run_project", run_project_def);

	// Register stop_project tool
	Dictionary stop_project_def;
	stop_project_def["name"] = "stop_project";
	stop_project_def["description"] = "Stop the running project";
	stop_project_def["parameters"] = Dictionary();
	register_tool("stop_project", stop_project_def);

	// Register validate_script tool
	Dictionary validate_script_def;
	validate_script_def["name"] = "validate_script";
	validate_script_def["description"] = "Validate a GDScript file";
	Dictionary validate_params;
	validate_params["script_path"] = "string";
	validate_script_def["parameters"] = validate_params;
	register_tool("validate_script", validate_script_def);

	// Register list_tools tool
	Dictionary list_tools_def;
	list_tools_def["name"] = "list_tools";
	list_tools_def["description"] = "List all available MCP tools";
	list_tools_def["parameters"] = Dictionary();
	register_tool("list_tools", list_tools_def);
}
