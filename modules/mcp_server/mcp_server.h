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

#include "core/object/class_db.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

class MCPServer : public RefCounted {
	GDCLASS(MCPServer, RefCounted);

private:
	int port = 6550;
	bool running = false;

public:
	void set_port(int p_port) { port = p_port; }
	int get_port() const { return port; }

	void start_server() { running = true; }
	void stop_server() { running = false; }
	bool is_running() const { return running; }

	Dictionary get_status() const {
		Dictionary status;
		status["port"] = port;
		status["running"] = running;
		return status;
	}

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_port", "port"), &MCPServer::set_port);
		ClassDB::bind_method(D_METHOD("get_port"), &MCPServer::get_port);
		ClassDB::bind_method(D_METHOD("start_server"), &MCPServer::start_server);
		ClassDB::bind_method(D_METHOD("stop_server"), &MCPServer::stop_server);
		ClassDB::bind_method(D_METHOD("is_running"), &MCPServer::is_running);
		ClassDB::bind_method(D_METHOD("get_status"), &MCPServer::get_status);

		ADD_PROPERTY(PropertyInfo(Variant::INT, "port"), "set_port", "get_port");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "running"), "", "is_running");
	}
};

class MCPTool : public RefCounted {
	GDCLASS(MCPTool, RefCounted);

public:
	String tool_name;
	Dictionary tool_definition;

	void set_name(const String &p_name) { tool_name = p_name; }
	String get_name() const { return tool_name; }

	void set_definition(const Dictionary &p_def) { tool_definition = p_def; }
	Dictionary get_definition() const { return tool_definition; }

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_name", "name"), &MCPTool::set_name);
		ClassDB::bind_method(D_METHOD("get_name"), &MCPTool::get_name);
		ClassDB::bind_method(D_METHOD("set_definition", "definition"), &MCPTool::set_definition);
		ClassDB::bind_method(D_METHOD("get_definition"), &MCPTool::get_definition);
	}
};
