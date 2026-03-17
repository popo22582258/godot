/**************************************************************************/
/*  mcp_server.h                                                            */
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

#include "core/io/stream_peer.h"
#include "core/object/ref_counted.h"
#include "core/object/object.h"
#include "core/string/string.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include "mcp_tools.h"
#include "scene_json_converter.h"
#include "hot_reload_helper.h"

#include <atomic>
#include <vector>

class MCPServer : public Node {
	GDCLASS(MCPServer, Node);

private:
	int port = 6550;
	bool running = false;
	bool stdio_mode = false;

	Ref<StreamPeer> connection;
	Vector<uint8_t> read_buffer;

	std::atomic<bool> should_stop{false};

	// MCP Protocol state
	String method;
	Dictionary params;
	int id = 0;

protected:
	static void _bind_methods();

public:
	MCPServer();
	~MCPServer();

	// Configuration
	void set_port(int p_port);
	int get_port() const { return port; }

	void set_stdio_mode(bool p_enabled);
	bool is_stdio_mode() const { return stdio_mode; }

	// Server control
	void start_server();
	void stop_server();
	bool is_running() const { return running; }

	// MCP Protocol handling
	void _process(double delta);
	void handle_jsonrpc(const Dictionary &message);
	Dictionary create_response(const Variant &result, int p_id);
	Dictionary create_error(int code, const String &message, int p_id);

	// MCP Tools - Scene Operations
	Dictionary tool_get_scene_tree();
	Dictionary tool_create_node(const String &parent_path, const String &type, const String &name, const Dictionary &properties);
	Dictionary tool_delete_node(const String &node_path);
	Dictionary tool_modify_node(const String &node_path, const Dictionary &properties);
	Dictionary tool_save_scene(const String &path);
	Dictionary tool_load_scene(const String &path);

	// MCP Tools - Scene Format Conversion
	Dictionary tool_tscn_to_json(const String &tscn_path, const String &json_path);
	Dictionary tool_json_to_tscn(const String &json_path, const String &tscn_path);

	// MCP Tools - Script Operations
	Dictionary tool_create_script(const String &path, const String &content, const String &language);
	Dictionary tool_edit_script(const String &path, const String &content);
	Dictionary tool_validate_script(const String &path);
	Dictionary tool_lint_gdscript(const String &path);
	Dictionary tool_get_script_symbols(const String &path);
	Dictionary tool_attach_script(const String &node_path, const String &script_path);

	// MCP Tools - Project Operations
	Dictionary tool_get_project_structure();
	Dictionary tool_get_project_settings();
	Dictionary tool_set_project_setting(const String &key, const Variant &value);

	// MCP Tools - Execution & Debugging
	Dictionary tool_run_project();
	Dictionary tool_stop_project();
	Dictionary tool_run_scene(const String &path);
	Dictionary tool_get_debug_output();
	Dictionary tool_get_errors();
	Dictionary tool_execute_expression(const String &expression);
	Dictionary tool_set_breakpoint(const String &script_path, int line);

	// MCP Tools - Editor Operations
	Dictionary tool_take_screenshot();
	Dictionary tool_get_editor_state();
	Dictionary tool_execute_menu_item(const String &menu_path);

	// MCP Tools - Hot Reload Operations
	Dictionary tool_reload_script(const String &script_path);
	Dictionary tool_reload_scene(const String &scene_path);
	Dictionary tool_reload_all_scripts();
	Dictionary tool_get_reload_events();
	Dictionary tool_watch_file(const String &path);
	Dictionary tool_set_auto_reload(bool enabled);

	// Internal
	void _notification(int p_what);

private:
	void _handle_connection();
	void _process_stdio();
	Dictionary _parse_json_message(const String &json_str);
	String _format_json_response(const Dictionary &response);
};

class MCPTool : public RefCounted {
	GDCLASS(MCPTool, RefCounted);

public:
	String name;
	String description;
	Dictionary parameters;

	MCPTool() {}
	MCPTool(const String &p_name, const String &p_description, const Dictionary &p_parameters = Dictionary()) :
			name(p_name), description(p_description), parameters(p_parameters) {}

	Dictionary to_json() const;
};
