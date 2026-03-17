/**************************************************************************/
/*  mcp_server.cpp                                                        */
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

#include "mcp_server.h"

#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/object/script_language.h"
#include "editor/editor_node.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "scene/resources/packed_scene.h"

MCPServer::MCPServer() {
	set_process(true);
}

MCPServer::~MCPServer() {
	stop_server();
}

void MCPServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_port", "port"), &MCPServer::set_port);
	ClassDB::bind_method(D_METHOD("get_port"), &MCPServer::get_port);
	ClassDB::bind_method(D_METHOD("set_stdio_mode", "enabled"), &MCPServer::set_stdio_mode);
	ClassDB::bind_method(D_METHOD("is_stdio_mode"), &MCPServer::is_stdio_mode);
	ClassDB::bind_method(D_METHOD("start_server"), &MCPServer::start_server);
	ClassDB::bind_method(D_METHOD("stop_server"), &MCPServer::stop_server);
	ClassDB::bind_method(D_METHOD("is_running"), &MCPServer::is_running);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "port"), "set_port", "get_port");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stdio_mode"), "set_stdio_mode", "is_stdio_mode");
}

void MCPServer::set_port(int p_port) {
	port = p_port;
}

void MCPServer::set_stdio_mode(bool p_enabled) {
	stdio_mode = p_enabled;
}

void MCPServer::start_server() {
	if (running) {
		return;
	}

	running = true;
	should_stop = false;

	if (stdio_mode) {
		// Stdio mode will be handled in _process
		print_line("[MCP] Server started in stdio mode");
	} else {
		print_line("[MCP] Server would start on port " + itoa(port) + " (WebSocket server implementation pending)");
	}
}

void MCPServer::stop_server() {
	if (!running) {
		return;
	}

	should_stop = true;
	running = false;
	print_line("[MCP] Server stopped");
}

void MCPServer::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		// Auto-start if configured
		EditorSettings *settings = EditorSettings::get_singleton();
		if (settings) {
			bool auto_start = settings->get_setting("mcp_server/auto_start");
			if (auto_start) {
				start_server();
			}
		}
	} else if (p_what == NOTIFICATION_EXIT_TREE) {
		stop_server();
	}
}

void MCPServer::_process(double delta) {
	if (!running) {
		return;
	}

	if (stdio_mode) {
		_process_stdio();
	}
}

void MCPServer::_process_stdio() {
	// Check if there's data available on stdin
	// In a real implementation, this would use OS::get_singleton()->get_stdin()
	// For now, this is a placeholder that demonstrates the protocol structure
}

Dictionary MCPServer::create_response(const Variant &result, int p_id) {
	Dictionary response;
	response["jsonrpc"] = "2.0";
	response["id"] = p_id;
	response["result"] = result;
	return response;
}

Dictionary MCPServer::create_error(int code, const String &message, int p_id) {
	Dictionary error;
	error["jsonrpc"] = "2.0";
	error["id"] = p_id;

	Dictionary error_obj;
	error_obj["code"] = code;
	error_obj["message"] = message;

	error["error"] = error_obj;
	return error;
}

String MCPServer::_format_json_response(const Dictionary &response) {
	String json_str = JSON::stringify(response, "\t", false);
	return json_str;
}

// MCP Tools Implementation

Dictionary MCPServer::tool_get_scene_tree() {
	Dictionary result;
	result["success"] = true;

	SceneTree *st = get_tree();
	if (!st) {
		return create_error(-32603, "Scene tree not available", 0);
	}

	Node *root = st->get_root_node();
	if (!root) {
		return create_error(-32603, "No root node", 0);
	}

	// Build scene tree as JSON
	Dictionary tree_data;
	tree_data["name"] = root->get_name();
	tree_data["type"] = root->get_class_name();
	tree_data["path"] = root->get_path();

	Array children;
	for (int i = 0; i < root->get_child_count(); i++) {
		Node *child = root->get_child(i);
		Dictionary child_data;
		child_data["name"] = child->get_name();
		child_data["type"] = child->get_class_name();
		child_data["path"] = child->get_path();
		children.append(child_data);
	}
	tree_data["children"] = children;

	result["scene_tree"] = tree_data;
	return create_response(result, id);
}

Dictionary MCPServer::tool_create_node(const String &parent_path, const String &type, const String &name, const Dictionary &properties) {
	Dictionary result;
	result["success"] = false;

	Node *parent = get_node_or_null(parent_path);
	if (!parent) {
		return create_error(-32602, "Parent node not found: " + parent_path, id);
	}

	Object *obj = ClassDB::create_instance(type);
	if (!obj) {
		return create_error(-32602, "Cannot create node of type: " + type, id);
	}

	Node *new_node = Object::cast_to<Node>(obj);
	if (!new_node) {
		memdelete(obj);
		return create_error(-32602, "Created object is not a Node: " + type, id);
	}

	new_node->set_name(name);

	// Apply properties
	Array prop_keys = properties.keys();
	for (int i = 0; i < prop_keys.size(); i++) {
		String key = prop_keys[i];
		Variant value = properties[key];
		new_node->set(key, value);
	}

	parent->add_child(new_node);
	result["success"] = true;
	result["node_path"] = new_node->get_path();

	return create_response(result, id);
}

Dictionary MCPServer::tool_delete_node(const String &node_path) {
	Dictionary result;
	result["success"] = false;

	Node *node = get_node_or_null(node_path);
	if (!node) {
		return create_error(-32602, "Node not found: " + node_path, id);
	}

	node->queue_free();
	result["success"] = true;

	return create_response(result, id);
}

Dictionary MCPServer::tool_modify_node(const String &node_path, const Dictionary &properties) {
	Dictionary result;
	result["success"] = false;

	Node *node = get_node_or_null(node_path);
	if (!node) {
		return create_error(-32602, "Node not found: " + node_path, id);
	}

	Array prop_keys = properties.keys();
	for (int i = 0; i < prop_keys.size(); i++) {
		String key = prop_keys[i];
		Variant value = properties[key];
		node->set(key, value);
	}

	result["success"] = true;
	result["node_path"] = node_path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_save_scene(const String &path) {
	Dictionary result;
	result["success"] = false;

	Node *root = get_tree()->get_root_node();
	if (!root) {
		return create_error(-32603, "No root node", id);
	}

	Ref<PackedScene> scene;
	scene.instantiate();
	Error err = scene->pack(root);
	if (err != OK) {
		return create_error(-32603, "Failed to pack scene", id);
	}

	err = ResourceSaver::get_singleton()->save(scene, path);
	if (err != OK) {
		return create_error(-32603, "Failed to save scene to: " + path, id);
	}

	result["success"] = true;
	result["path"] = path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_load_scene(const String &path) {
	Dictionary result;
	result["success"] = false;

	Error err = get_tree()->change_scene(path);
	if (err != OK) {
		return create_error(-32603, "Failed to load scene: " + path, id);
	}

	result["success"] = true;
	result["path"] = path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_tscn_to_json(const String &tscn_path, const String &json_path) {
	Dictionary result;
	result["success"] = false;

	Error err = SceneJSONConverter::tscn_to_json(tscn_path, json_path);
	if (err != OK) {
		return create_error(-32603, "Failed to convert TSCN to JSON: " + tscn_path, id);
	}

	result["success"] = true;
	result["input_path"] = tscn_path;
	result["output_path"] = json_path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_json_to_tscn(const String &json_path, const String &tscn_path) {
	Dictionary result;
	result["success"] = false;

	Error err = SceneJSONConverter::json_to_tscn(json_path, tscn_path);
	if (err != OK) {
		return create_error(-32603, "Failed to convert JSON to TSCN: " + json_path, id);
	}

	result["success"] = true;
	result["input_path"] = json_path;
	result["output_path"] = tscn_path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_create_script(const String &path, const String &content, const String &language) {
	Dictionary result;
	result["success"] = false;

	Ref<FileAccess> fa = FileAccess::create_for_path(path);
	if (!fa.is_valid()) {
		return create_error(-32603, "Cannot create file: " + path, id);
	}

	fa->store_string(content);
	fa->close();

	result["success"] = true;
	result["path"] = path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_edit_script(const String &path, const String &content) {
	Dictionary result;
	result["success"] = false;

	Ref<FileAccess> fa = FileAccess::create_for_path(path);
	if (!fa.is_valid()) {
		return create_error(-32603, "Cannot open file: " + path, id);
	}

	fa->store_string(content);
	fa->close();

	result["success"] = true;
	result["path"] = path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_validate_script(const String &path) {
	MCPTools::ValidationResult validation = MCPTools::validate_gdscript(path);

	Dictionary result;
	result["success"] = !validation.error_message.is_empty() || validation.diagnostics.size() == 0;
	result["valid"] = validation.valid;
	result["error_message"] = validation.error_message;
	result["error_line"] = validation.error_line;
	result["error_column"] = validation.error_column;
	result["error_code"] = validation.error_code;
	result["diagnostics"] = validation.diagnostics;
	result["warnings"] = validation.warnings;

	return create_response(result, id);
}

Dictionary MCPServer::tool_get_script_symbols(const String &path) {
	Array symbols = MCPTools::get_script_symbols_internal(path);

	Dictionary result;
	result["success"] = true;
	result["symbols"] = symbols;

	// Group by type for convenience
	Array functions;
	Array variables;
	Array signals;

	for (int i = 0; i < symbols.size(); i++) {
		Dictionary sym = symbols[i];
		String type = sym["type"];
		if (type == "function") {
			functions.append(sym);
		} else if (type == "variable") {
			variables.append(sym);
		} else if (type == "signal") {
			signals.append(sym);
		}
	}

	result["functions"] = functions;
	result["variables"] = variables;
	result["signals"] = signals;

	return create_response(result, id);
}

Dictionary MCPServer::tool_attach_script(const String &node_path, const String &script_path) {
	Dictionary result;
	result["success"] = false;

	Node *node = get_node_or_null(node_path);
	if (!node) {
		return create_error(-32602, "Node not found: " + node_path, id);
	}

	Ref<Script> script = ResourceLoader::get_singleton()->load(script_path);
	if (!script.is_valid()) {
		return create_error(-32603, "Failed to load script: " + script_path, id);
	}

	node->set_script(script);
	result["success"] = true;
	result["node_path"] = node_path;
	result["script_path"] = script_path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_get_project_structure() {
	Dictionary result;
	result["success"] = true;

	// Return project file structure
	Array files;

	String project_path = ProjectSettings::get_singleton()->get_project_path();
	DirAccess *dir = DirAccess::open(project_path);
	if (dir) {
		// List files recursively (simplified)
		dir->list_dir_end();
		memdelete(dir);
	}

	result["files"] = files;
	result["project_path"] = project_path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_get_project_settings() {
	Dictionary result;
	result["success"] = true;

	Dictionary settings;
	// Return project settings (simplified)
	result["settings"] = settings;

	return create_response(result, id);
}

Dictionary MCPServer::tool_set_project_setting(const String &key, const Variant &value) {
	Dictionary result;
	result["success"] = false;

	ProjectSettings::get_singleton()->set_setting(key, value);
	ProjectSettings::get_singleton()->set_initial_value(key, value);

	result["success"] = true;
	result["key"] = key;

	return create_response(result, id);
}

Dictionary MCPServer::tool_run_project() {
	Dictionary result;
	result["success"] = false;

	// This would start the project in play mode
	result["success"] = true;
	result["message"] = "Project run requested";

	return create_response(result, id);
}

Dictionary MCPServer::tool_stop_project() {
	Dictionary result;
	result["success"] = true;

	// This would stop the running project
	get_tree()->quit();

	return create_response(result, id);
}

Dictionary MCPServer::tool_run_scene(const String &path) {
	Dictionary result;
	result["success"] = false;

	Error err = get_tree()->change_scene(path);
	if (err != OK) {
		return create_error(-32603, "Failed to run scene: " + path, id);
	}

	result["success"] = true;
	result["path"] = path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_get_debug_output() {
	Dictionary result;
	result["success"] = true;
	result["output"] = Array();

	// This would retrieve debug output
	return create_response(result, id);
}

Dictionary MCPServer::tool_get_errors() {
	Dictionary result;
	result["success"] = true;
	result["errors"] = Array();

	// This would retrieve current script errors
	return create_response(result, id);
}

Dictionary MCPServer::tool_lint_gdscript(const String &path) {
	MCPTools::LintResult lint_result = MCPTools::lint_gdscript(path);

	Dictionary result;
	result["success"] = lint_result.success;
	result["file_path"] = lint_result.file_path;
	result["diagnostics"] = lint_result.diagnostics;
	result["error_count"] = lint_result.error_count;
	result["warning_count"] = lint_result.warning_count;
	result["info_count"] = lint_result.info_count;
	result["summary"] = itos(lint_result.error_count) + " errors, " +
			itos(lint_result.warning_count) + " warnings, " +
			itos(lint_result.info_count) + " info";

	return create_response(result, id);
}

Dictionary MCPServer::tool_execute_expression(const String &expression) {
	Dictionary result;
	result["success"] = false;

	// This would execute an expression in the running game
	result["success"] = true;
	result["expression"] = expression;

	return create_response(result, id);
}

Dictionary MCPServer::tool_set_breakpoint(const String &script_path, int line) {
	Dictionary result;
	result["success"] = true;
	result["script_path"] = script_path;
	result["line"] = line;

	// This would set a breakpoint in the debugger
	return create_response(result, id);
}

Dictionary MCPServer::tool_take_screenshot() {
	Dictionary result;
	result["success"] = false;

	// This would capture the editor/game window
	result["success"] = true;
	result["message"] = "Screenshot capture requested";

	return create_response(result, id);
}

Dictionary MCPServer::tool_get_editor_state() {
	Dictionary result;
	result["success"] = true;

	EditorNode *editor = EditorNode::get_singleton();
	if (editor) {
		result["selected_node"] = editor->get_inspector()->get_selected_path();
	}

	result["open_scenes"] = Array();

	return create_response(result, id);
}

Dictionary MCPServer::tool_execute_menu_item(const String &menu_path) {
	Dictionary result;
	result["success"] = false;

	// This would execute a menu item by path
	result["success"] = true;
	result["menu_path"] = menu_path;

	return create_response(result, id);
}

// Hot Reload Tools Implementation

Dictionary MCPServer::tool_reload_script(const String &script_path) {
	ScriptReloader::ReloadResult reload_result = ScriptReloader::reload_script(script_path, true);

	Dictionary result;
	result["success"] = reload_result.success;
	result["path"] = script_path;
	result["error_message"] = reload_result.error_message;
	result["errors"] = reload_result.errors;
	result["warnings"] = reload_result.warnings;

	return create_response(result, id);
}

Dictionary MCPServer::tool_reload_scene(const String &scene_path) {
	Dictionary result;
	result["success"] = false;

	SceneTree *st = get_tree();
	if (!st) {
		return create_error(-32603, "Scene tree not available", id);
	}

	Error err = st->change_scene(scene_path);
	if (err != OK) {
		result["error_message"] = "Failed to reload scene: " + scene_path;
		return create_error(-32603, result["error_message"], id);
	}

	result["success"] = true;
	result["path"] = scene_path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_reload_all_scripts() {
	ScriptReloader::ReloadResult reload_result = ScriptReloader::reload_all_scripts(true);

	Dictionary result;
	result["success"] = reload_result.success;
	result["error_message"] = reload_result.error_message;

	return create_response(result, id);
}

Dictionary MCPServer::tool_get_reload_events() {
	Dictionary result;
	result["events"] = Array();
	result["success"] = true;

	// This would get pending reload events
	// For now, return empty

	return create_response(result, id);
}

Dictionary MCPServer::tool_watch_file(const String &path) {
	Dictionary result;
	result["success"] = true;
	result["path"] = path;

	// This would add file to watch list
	result["message"] = "File watching requested for: " + path;

	return create_response(result, id);
}

Dictionary MCPServer::tool_set_auto_reload(bool enabled) {
	Dictionary result;
	result["success"] = true;
	result["auto_reload_enabled"] = enabled;

	result["message"] = enabled ? "Auto-reload enabled" : "Auto-reload disabled";

	return create_response(result, id);
}

void MCPServer::handle_jsonrpc(const Dictionary &message) {
	if (!message.has("jsonrpc") || message["jsonrpc"] != "2.0") {
		Dictionary err = create_error(-32600, "Invalid JSON-RPC version", message.has("id") ? (int)message["id"] : 0);
		print_line("[MCP] Error: " + JSON::stringify(err));
		return;
	}

	if (message.has("id")) {
		id = message["id"];
	}

	if (!message.has("method")) {
		Dictionary err = create_error(-32600, "Missing method", id);
		print_line("[MCP] Error: " + JSON::stringify(err));
		return;
	}

	method = message["method"];
	params = message.has("params") ? message["params"] : Dictionary();

	Dictionary response;

	// Route to appropriate tool
	if (method == "get_scene_tree") {
		response = tool_get_scene_tree();
	} else if (method == "create_node") {
		String parent_path = params.has("parent_path") ? params["parent_path"] : "";
		String type = params.has("type") ? params["type"] : "Node";
		String name = params.has("name") ? params["name"] : "NewNode";
		Dictionary properties = params.has("properties") ? params["properties"] : Dictionary();
		response = tool_create_node(parent_path, type, name, properties);
	} else if (method == "delete_node") {
		response = tool_delete_node(params.has("node_path") ? params["node_path"] : "");
	} else if (method == "modify_node") {
		String node_path = params.has("node_path") ? params["node_path"] : "";
		Dictionary properties = params.has("properties") ? params["properties"] : Dictionary();
		response = tool_modify_node(node_path, properties);
	} else if (method == "save_scene") {
		response = tool_save_scene(params.has("path") ? params["path"] : "");
	} else if (method == "load_scene") {
		response = tool_load_scene(params.has("path") ? params["path"] : "");
	} else if (method == "tscn_to_json") {
		response = tool_tscn_to_json(
				params.has("tscn_path") ? params["tscn_path"] : "",
				params.has("json_path") ? params["json_path"] : "");
	} else if (method == "json_to_tscn") {
		response = tool_json_to_tscn(
				params.has("json_path") ? params["json_path"] : "",
				params.has("tscn_path") ? params["tscn_path"] : "");
	} else if (method == "create_script") {
		String path = params.has("path") ? params["path"] : "";
		String content = params.has("content") ? params["content"] : "";
		String language = params.has("language") ? params["language"] : "GDScript";
		response = tool_create_script(path, content, language);
	} else if (method == "edit_script") {
		response = tool_edit_script(params.has("path") ? params["path"] : "", params.has("content") ? params["content"] : "");
	} else if (method == "validate_script") {
		response = tool_validate_script(params.has("path") ? params["path"] : "");
	} else if (method == "lint_gdscript") {
		response = tool_lint_gdscript(params.has("path") ? params["path"] : "");
	} else if (method == "get_script_symbols") {
		response = tool_get_script_symbols(params.has("path") ? params["path"] : "");
	} else if (method == "attach_script") {
		response = tool_attach_script(params.has("node_path") ? params["node_path"] : "", params.has("script_path") ? params["script_path"] : "");
	} else if (method == "get_project_structure") {
		response = tool_get_project_structure();
	} else if (method == "get_project_settings") {
		response = tool_get_project_settings();
	} else if (method == "set_project_setting") {
		response = tool_set_project_setting(params.has("key") ? params["key"] : "", params.has("value") ? params["value"] : Variant());
	} else if (method == "run_project") {
		response = tool_run_project();
	} else if (method == "stop_project") {
		response = tool_stop_project();
	} else if (method == "run_scene") {
		response = tool_run_scene(params.has("path") ? params["path"] : "");
	} else if (method == "get_debug_output") {
		response = tool_get_debug_output();
	} else if (method == "get_errors") {
		response = tool_get_errors();
	} else if (method == "execute_expression") {
		response = tool_execute_expression(params.has("expression") ? params["expression"] : "");
	} else if (method == "set_breakpoint") {
		response = tool_set_breakpoint(params.has("script_path") ? params["script_path"] : "", params.has("line") ? params["line"] : 0);
	} else if (method == "take_screenshot") {
		response = tool_take_screenshot();
	} else if (method == "get_editor_state") {
		response = tool_get_editor_state();
	} else if (method == "execute_menu_item") {
		response = tool_execute_menu_item(params.has("menu_path") ? params["menu_path"] : "");
	} else if (method == "reload_script") {
		response = tool_reload_script(params.has("script_path") ? params["script_path"] : "");
	} else if (method == "reload_scene") {
		response = tool_reload_scene(params.has("scene_path") ? params["scene_path"] : "");
	} else if (method == "reload_all_scripts") {
		response = tool_reload_all_scripts();
	} else if (method == "get_reload_events") {
		response = tool_get_reload_events();
	} else if (method == "watch_file") {
		response = tool_watch_file(params.has("path") ? params["path"] : "");
	} else if (method == "set_auto_reload") {
		response = tool_set_auto_reload(params.has("enabled") ? params["enabled"] : false);
	} else {
		response = create_error(-32601, "Method not found: " + method, id);
	}

	print_line("[MCP] Response: " + _format_json_response(response));
}

Dictionary MCPServer::_parse_json_message(const String &json_str) {
	JSON json;
	Error err = json.parse(json_str);
	if (err != OK) {
		return Dictionary();
	}
	return json.get_data();
}

// MCPTool implementation
Dictionary MCPTool::to_json() const {
	Dictionary tool_def;
	tool_def["name"] = name;
	tool_def["description"] = description;
	tool_def["parameters"] = parameters;
	return tool_def;
}
