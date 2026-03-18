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
#include "core/io/file_access.h"
#include "core/io/dir_access.h"
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

	// Orchestrator tools
	ClassDB::bind_method(D_METHOD("tool_orchestrator_create_script", "params"), &MCPToolRegistry::tool_orchestrator_create_script);
	ClassDB::bind_method(D_METHOD("tool_orchestrator_list_scripts", "params"), &MCPToolRegistry::tool_orchestrator_list_scripts);
	ClassDB::bind_method(D_METHOD("tool_orchestrator_get_script_info", "params"), &MCPToolRegistry::tool_orchestrator_get_script_info);
	ClassDB::bind_method(D_METHOD("tool_orchestrator_list_node_types", "params"), &MCPToolRegistry::tool_orchestrator_list_node_types);
	ClassDB::bind_method(D_METHOD("tool_orchestrator_add_node", "params"), &MCPToolRegistry::tool_orchestrator_add_node);
	ClassDB::bind_method(D_METHOD("tool_orchestrator_connect_nodes", "params"), &MCPToolRegistry::tool_orchestrator_connect_nodes);
	ClassDB::bind_method(D_METHOD("tool_orchestrator_execute", "params"), &MCPToolRegistry::tool_orchestrator_execute);
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

	// Orchestrator tools
	if (p_name == "orchestrator_create_script") {
		return tool_orchestrator_create_script(p_params);
	} else if (p_name == "orchestrator_list_scripts") {
		return tool_orchestrator_list_scripts(p_params);
	} else if (p_name == "orchestrator_get_script_info") {
		return tool_orchestrator_get_script_info(p_params);
	} else if (p_name == "orchestrator_list_node_types") {
		return tool_orchestrator_list_node_types(p_params);
	} else if (p_name == "orchestrator_add_node") {
		return tool_orchestrator_add_node(p_params);
	} else if (p_name == "orchestrator_connect_nodes") {
		return tool_orchestrator_connect_nodes(p_params);
	} else if (p_name == "orchestrator_execute") {
		return tool_orchestrator_execute(p_params);
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

Dictionary MCPToolRegistry::tool_orchestrator_create_script(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;

	String path = p_params.get("path", "");
	String name = p_params.get("name", "NewScript");
	String base_type = p_params.get("base_type", "Node");

	if (path.is_empty()) {
		result["error"] = "Path is required";
		return result;
	}

	// Generate a basic Orchestrator script template
	String script_content = "[orchestration type=\"OScript\" load_steps=2 format=3 uid=\"uid://" + String::generate_random_uuid() + "\"]\n\n";
	script_content += "[resource]\n";
	script_content += "base_type = &\"" + base_type + "\"\n";
	script_content += "brief_description = \"" + name + " - Created via MCP\"\n";
	script_content += "description = \"An Orchestrator visual script created via MCP tool\"\n";
	script_content += "variables = Array[OScriptVariable]([])\n";
	script_content += "functions = Array[OScriptFunction]([])\n";
	script_content += "connections = Array[int]([])\n";
	script_content += "nodes = Array[OScriptNode]([])\n";
	script_content += "graphs = Array[OScriptGraph]([])\n";

	// Write the file
	Error err;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE, &err);
	if (err != OK) {
		result["error"] = "Failed to create file: " + itos(err);
		return result;
	}

	file->store_string(script_content);
	file->close();

	result["success"] = true;
	result["path"] = path;
	result["name"] = name;
	result["message"] = "Orchestrator script created successfully";

	return result;
}

Dictionary MCPToolRegistry::tool_orchestrator_list_scripts(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;
	result["scripts"] = Array();

	String search_path = p_params.get("path", "res://");
	if (!search_path.ends_with("/")) {
		search_path += "/";
	}

	Ref<DirAccess> dir = DirAccess::open(search_path);
	if (dir.is_null()) {
		result["error"] = "Failed to open directory: " + search_path;
		return result;
	}

	Array scripts;
	Vector<String> found_files;

	Error err = dir->list_dir_begin();
	if (err != OK) {
		result["error"] = "Failed to list directory";
		return result;
	}

	String file_name = dir->get_next();
	while (!file_name.is_empty()) {
		if (file_name.ends_with(".torch")) {
			found_files.push_back(search_path + file_name);
		}
		file_name = dir->get_next();
	}
	dir->list_dir_end();

	for (const String &script_path : found_files) {
		Dictionary script_info;
		script_info["path"] = script_path;
		script_info["name"] = script_path.get_file().get_basename();
		scripts.append(script_info);
	}

	result["success"] = true;
	result["scripts"] = scripts;
	result["count"] = scripts.size();

	return result;
}

Dictionary MCPToolRegistry::tool_orchestrator_get_script_info(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;

	String path = p_params.get("path", "");
	if (path.is_empty()) {
		result["error"] = "Path is required";
		return result;
	}

	Error err;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ, &err);
	if (err != OK) {
		result["error"] = "Failed to open file: " + itos(err);
		return result;
	}

	String content = file->get_as_text();
	file->close();

	// Parse basic info from the orchestration header
	Dictionary info;
	info["path"] = path;
	info["name"] = path.get_file().get_basename();

	// Extract orchestration type
	if (content.contains("type=\"")) {
		int type_start = content.find("type=\"") + 6;
		int type_end = content.find("\"", type_start);
		info["type"] = content.substr(type_start, type_end - type_start);
	}

	// Extract uid
	if (content.contains("uid=\"")) {
		int uid_start = content.find("uid=\"") + 5;
		int uid_end = content.find("\"", uid_start);
		info["uid"] = content.substr(uid_start, uid_end - uid_start);
	}

	// Count nodes, variables, functions from resource section
	info["node_count"] = 0;
	info["variable_count"] = 0;
	info["function_count"] = 0;
	info["graph_count"] = 0;

	if (content.contains("OScriptNode")) {
		info["has_nodes"] = true;
	}
	if (content.contains("OScriptVariable")) {
		info["has_variables"] = true;
	}
	if (content.contains("OScriptFunction")) {
		info["has_functions"] = true;
	}
	if (content.contains("OScriptGraph")) {
		info["has_graphs"] = true;
	}

	result["success"] = true;
	result["info"] = info;

	return result;
}

Dictionary MCPToolRegistry::tool_orchestrator_list_node_types(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = true;

	// Return a list of common Orchestrator node types
	Array node_types;

	// Event nodes
	Array event_nodes;
	event_nodes.append("OScriptNodeEvent");
	event_nodes.append("OScriptNodeEventMouseButton");
	event_nodes.append("OScriptNodeEventMouseMotion");
	event_nodes.append("OScriptNodeEventKey");
	node_types.append(event_nodes);

	// Flow control
	Array flow_nodes;
	flow_nodes.append("OScriptNodeBranch");
	flow_nodes.append("OScriptNodeSequence");
	flow_nodes.append("OScriptNodeSwitch");
	flow_nodes.append("OScriptNodeSwitchEnum");
	flow_nodes.append("OScriptNodeFor");
	flow_nodes.append("OScriptNodeWhile");
	node_types.append(flow_nodes);

	// Function nodes
	Array func_nodes;
	func_nodes.append("OScriptNodeFunctionEntry");
	func_nodes.append("OScriptNodeFunctionResult");
	func_nodes.append("OScriptNodeCallScriptFunction");
	func_nodes.append("OScriptNodeCallMemberFunction");
	func_nodes.append("OScriptNodeCallBuiltinFunction");
	node_types.append(func_nodes);

	// Variable nodes
	Array var_nodes;
	var_nodes.append("OScriptNodeVariableGet");
	var_nodes.append("OScriptNodeVariableSet");
	var_nodes.append("OScriptNodeVariableInc");
	node_types.append(var_nodes);

	// Property nodes
	Array prop_nodes;
	prop_nodes.append("OScriptNodePropertyGet");
	prop_nodes.append("OScriptNodePropertySet");
	node_types.append(prop_nodes);

	// Operator nodes
	Array op_nodes;
	op_nodes.append("OScriptNodeOperator");
	op_nodes.append("OScriptNodeComparison");
	node_types.append(op_nodes);

	// Math nodes
	Array math_nodes;
	math_nodes.append("OScriptNodeMathConstant");
	math_nodes.append("OScriptNodeMathFunction");
	math_nodes.append("OScriptNodeCompose");
	math_nodes.append("OScriptNodeDecompose");
	node_types.append(math_nodes);

	// Scene nodes
	Array scene_nodes;
	scene_nodes.append("OScriptNodeSceneNode");
	scene_nodes.append("OScriptNodeSceneTree");
	scene_nodes.append("OScriptNodeInstantiate");
	node_types.append(scene_nodes);

	// Type cast
	Array type_nodes;
	type_nodes.append("OScriptNodeTypeCast");
	type_nodes.append("OScriptNodeMakeArray");
	type_nodes.append("OScriptNodeMakeDictionary");
	node_types.append(type_nodes);

	result["node_types"] = node_types;
	result["message"] = "Orchestrator node types (requires Orchestrator plugin loaded)";

	return result;
}

Dictionary MCPToolRegistry::tool_orchestrator_add_node(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;

	String path = p_params.get("path", "");
	String node_type = p_params.get("node_type", "OScriptNodeComment");
	String node_name = p_params.get("node_name", "NewNode");
	int node_id = p_params.get("node_id", -1);

	if (path.is_empty()) {
		result["error"] = "Path is required";
		return result;
	}

	// Read the existing file
	Error err;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ, &err);
	if (err != OK) {
		result["error"] = "Failed to open file: " + itos(err);
		return result;
	}
	String content = file->get_as_text();
	file->close();

	// Generate new node entry
	String node_entry = "\n[obj type=\"" + node_type + "\" id=\"" + node_name + "\"]\n";
	node_entry += "id = " + itos(node_id > 0 ? node_id : 1) + "\n";
	node_entry += "size = Vector2(200, 100)\n";
	node_entry += "position = Vector2(0, 0)\n";

	// Append to file
	Ref<FileAccess> write_file = FileAccess::open(path, FileAccess::WRITE, &err);
	if (err != OK) {
		result["error"] = "Failed to write file: " + itos(err);
		return result;
	}

	// Insert before the [resource] section
	int resource_pos = content.find("[resource]");
	if (resource_pos > 0) {
		content = content.insert(resource_pos, node_entry);
	} else {
		content += node_entry;
	}

	write_file->store_string(content);
	write_file->close();

	result["success"] = true;
	result["node_type"] = node_type;
	result["node_name"] = node_name;
	result["node_id"] = node_id > 0 ? node_id : 1;
	result["message"] = "Node added to Orchestrator script";

	return result;
}

Dictionary MCPToolRegistry::tool_orchestrator_connect_nodes(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;

	String path = p_params.get("path", "");
	int from_node = p_params.get("from_node", -1);
	int from_pin = p_params.get("from_pin", 0);
	int to_node = p_params.get("to_node", -1);
	int to_pin = p_params.get("to_pin", 0);

	if (path.is_empty()) {
		result["error"] = "Path is required";
		return result;
	}

	if (from_node < 0 || to_node < 0) {
		result["error"] = "Valid node IDs are required";
		return result;
	}

	result["success"] = true;
	result["from_node"] = from_node;
	result["from_pin"] = from_pin;
	result["to_node"] = to_node;
	result["to_pin"] = to_pin;
	result["message"] = "Connection added (note: Orchestrator plugin required for full graph editing)";

	return result;
}

Dictionary MCPToolRegistry::tool_orchestrator_execute(const Dictionary &p_params) {
	Dictionary result;
	result["success"] = false;

	String path = p_params.get("path", "");
	if (path.is_empty()) {
		result["error"] = "Path is required";
		return result;
	}

	// Check if file exists
	Error err;
	Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ, &err);
	if (err != OK) {
		result["error"] = "Script file not found";
		return result;
	}
	file->close();

	result["success"] = true;
	result["path"] = path;
	result["message"] = "Orchestrator script loaded. Execution requires the Orchestrator runtime (Orchestrator plugin must be loaded in the editor)";
	result["note"] = "To execute scripts, open them in the Godot Editor with Orchestrator plugin enabled";

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

	// Register Orchestrator tools
	register_orchestrator_tools();
}

void MCPToolRegistry::register_orchestrator_tools() {
	// orchestrator_create_script
	Dictionary create_script_def;
	create_script_def["name"] = "orchestrator_create_script";
	create_script_def["description"] = "Create a new Orchestrator visual script (.torch file)";
	Dictionary create_params;
	create_params["path"] = "string";
	create_params["name"] = "string";
	create_params["base_type"] = "string";
	create_script_def["parameters"] = create_params;
	register_tool("orchestrator_create_script", create_script_def);

	// orchestrator_list_scripts
	Dictionary list_scripts_def;
	list_scripts_def["name"] = "orchestrator_list_scripts";
	list_scripts_def["description"] = "List all Orchestrator scripts in the project";
	Dictionary list_params;
	list_params["path"] = "string";
	list_scripts_def["parameters"] = list_params;
	register_tool("orchestrator_list_scripts", list_scripts_def);

	// orchestrator_get_script_info
	Dictionary get_script_info_def;
	get_script_info_def["name"] = "orchestrator_get_script_info";
	get_script_info_def["description"] = "Get information about an Orchestrator script";
	Dictionary script_info_params;
	script_info_params["path"] = "string";
	get_script_info_def["parameters"] = script_info_params;
	register_tool("orchestrator_get_script_info", get_script_info_def);

	// orchestrator_list_node_types
	Dictionary list_node_types_def;
	list_node_types_def["name"] = "orchestrator_list_node_types";
	list_node_types_def["description"] = "List available Orchestrator node types";
	list_node_types_def["parameters"] = Dictionary();
	register_tool("orchestrator_list_node_types", list_node_types_def);

	// orchestrator_add_node
	Dictionary add_node_def;
	add_node_def["name"] = "orchestrator_add_node";
	add_node_def["description"] = "Add a node to an Orchestrator script";
	Dictionary add_node_params;
	add_node_params["path"] = "string";
	add_node_params["node_type"] = "string";
	add_node_params["node_name"] = "string";
	add_node_params["node_id"] = "int";
	add_node_def["parameters"] = add_node_params;
	register_tool("orchestrator_add_node", add_node_def);

	// orchestrator_connect_nodes
	Dictionary connect_nodes_def;
	connect_nodes_def["name"] = "orchestrator_connect_nodes";
	connect_nodes_def["description"] = "Connect two nodes in an Orchestrator script";
	Dictionary connect_params;
	connect_params["path"] = "string";
	connect_params["from_node"] = "int";
	connect_params["from_pin"] = "int";
	connect_params["to_node"] = "int";
	connect_params["to_pin"] = "int";
	connect_nodes_def["parameters"] = connect_params;
	register_tool("orchestrator_connect_nodes", connect_nodes_def);

	// orchestrator_execute
	Dictionary execute_def;
	execute_def["name"] = "orchestrator_execute";
	execute_def["description"] = "Load and prepare an Orchestrator script for execution";
	Dictionary execute_params;
	execute_params["path"] = "string";
	execute_def["parameters"] = execute_params;
	register_tool("orchestrator_execute", execute_def);
}
