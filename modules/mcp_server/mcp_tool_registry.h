/**************************************************************************/
/*  mcp_tool_registry.h                                                  */
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

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/ustring.h"

class MCPToolRegistry : public RefCounted {
	GDCLASS(MCPToolRegistry, RefCounted);

private:
	Dictionary tools;

protected:
	static void _bind_methods();

public:
	void register_tool(const String &p_name, const Dictionary &p_definition);
	bool has_tool(const String &p_name) const;
	Dictionary get_tool_definition(const String &p_name) const;
	Array get_all_tool_names() const;
	Variant call_tool(const String &p_name, const Dictionary &p_params);

	// Built-in tools
	Dictionary tool_get_errors(const Dictionary &p_params);
	Dictionary tool_get_warnings(const Dictionary &p_params);
	Dictionary tool_get_scene_tree(const Dictionary &p_params);
	Dictionary tool_get_project_structure(const Dictionary &p_params);
	Dictionary tool_run_project(const Dictionary &p_params);
	Dictionary tool_stop_project(const Dictionary &p_params);
	Dictionary tool_validate_script(const Dictionary &p_params);
	Dictionary tool_list_tools(const Dictionary &p_params);

	// Orchestrator tools
	Dictionary tool_orchestrator_create_script(const Dictionary &p_params);
	Dictionary tool_orchestrator_list_scripts(const Dictionary &p_params);
	Dictionary tool_orchestrator_get_script_info(const Dictionary &p_params);
	Dictionary tool_orchestrator_list_node_types(const Dictionary &p_params);
	Dictionary tool_orchestrator_add_node(const Dictionary &p_params);
	Dictionary tool_orchestrator_connect_nodes(const Dictionary &p_params);
	Dictionary tool_orchestrator_execute(const Dictionary &p_params);

	// Initialize built-in tools
	void register_builtin_tools();
	void register_orchestrator_tools();
};
