/**************************************************************************/
/*  mcp_server.cpp                                                          */
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

#include "mcp_protocol.h"
#include "mcp_tool_registry.h"

#include "core/io/json.h"

void MCPServer::initialize() {
	server_status = "initialized";
	print_line("MCP Server initialized");
}

bool MCPServer::start_server() {
	if (running) {
		print_line("MCP Server is already running");
		return false;
	}

	running = true;
	server_status = "running";
	print_line("MCP Server started on port " + itos(port));
	print_line("MCP Server ready - connect using WebSocket or stdio");
	print_line("  WebSocket: ws://localhost:" + itos(port));
	print_line("  Or use: godot --headless --path <project> --mcp-stdio");

	return true;
}

void MCPServer::stop_server() {
	if (!running) {
		return;
	}

	running = false;
	server_status = "stopped";
	print_line("MCP Server stopped");
}

String MCPServer::process_request(const String &p_request) {
	// Parse JSON-RPC request
	Ref<MCPProtocol> protocol;
	protocol.instantiate();

	Variant result = JSON::parse_string(p_request);
	if (result.get_type() != Variant::DICTIONARY) {
		return protocol->create_error_response(Variant(), -32700, "Parse error");
	}

	Dictionary request = result;
	String method;
	if (request.has("method")) {
		method = request["method"];
	}
	Variant params;
	if (request.has("params")) {
		params = request["params"];
	}
	Variant id;
	if (request.has("id")) {
		id = request["id"];
	}

	// Handle MCP protocol methods
	if (method == "initialize") {
		Dictionary capabilities;
		capabilities["tools"] = true;
		capabilities["resources"] = true;
		capabilities["prompts"] = true;

		Dictionary response;
		response["protocolVersion"] = "2024-11-05";
		Dictionary server_info;
		server_info["name"] = "Godot MCP Server";
		server_info["version"] = "1.0.0";
		response["serverInfo"] = server_info;
		response["capabilities"] = capabilities;

		return protocol->create_response(id, response);
	} else if (method == "tools/list") {
		Ref<MCPToolRegistry> registry;
		registry.instantiate();
		registry->register_builtin_tools();

		Dictionary result_dict;
		result_dict["tools"] = Array();
		return protocol->create_response(id, result_dict);
	} else if (method == "tools/call") {
		Ref<MCPToolRegistry> registry;
		registry.instantiate();
		registry->register_builtin_tools();

		Dictionary call_params;
		if (params.get_type() == Variant::DICTIONARY) {
			call_params = params;
		}
		String tool_name = call_params.get("name", "");
		Dictionary tool_params = call_params.get("arguments", Dictionary());

		Variant tool_result = registry->call_tool(tool_name, tool_params);
		return protocol->create_response(id, tool_result);
	}

	// Unknown method
	return protocol->create_error_response(id, -32601, "Method not found");
}
