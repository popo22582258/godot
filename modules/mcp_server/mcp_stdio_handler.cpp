/**************************************************************************/
/*  mcp_stdio_handler.cpp                                                 */
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

#include "mcp_stdio_handler.h"

#include "mcp_server.h"
#include "mcp_protocol.h"
#include "mcp_tool_registry.h"

#include "core/io/json.h"

void MCPStdioHandler::start() {
	if (running) {
		return;
	}

	running = true;
	server_status = "running";

	// Print MCP Server ready message to stdout
	print_line("MCP Server started in stdio mode");
	print_line("Ready for JSON-RPC requests");
	print_line("---");
}

void MCPStdioHandler::stop() {
	if (!running) {
		return;
	}

	running = false;
	server_status = "stopped";
	print_line("MCP Server stopped");
}

String MCPStdioHandler::process_line(const String &p_line) {
	if (p_line.is_empty()) {
		return "";
	}

	// Create MCP Server to handle the request
	Ref<MCPServer> server;
	server.instantiate();
	server->initialize();

	// Process the request
	return server->process_request(p_line);
}
