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
#include "core/os/os.h"
#include "core/os/thread.h"

#include <cstdio>

void MCPStdioHandler::_stdio_thread_func(void *p_userdata) {
	MCPStdioHandler *handler = static_cast<MCPStdioHandler *>(p_userdata);
	if (handler) {
		handler->read_stdin_loop();
	}
}

void MCPStdioHandler::read_stdin_loop() {
	print_line("MCP Stdio: Starting stdin reader thread");

	// Initialize the server
	server.instantiate();
	server->initialize();
	server->start_server();

	print_line("MCP Stdio: Server initialized, waiting for requests...");

	// Wait for main thread to fully initialize
	OS::get_singleton()->delay_usec(1000000); // 1 second

	while (!thread_should_stop) {
		// Check if thread should stop
		if (thread_should_stop) {
			break;
		}

		// Check if stdin is still valid
		if (stdin == nullptr || ferror(stdin)) {
			print_line("MCP Stdio: stdin error, exiting");
			break;
		}

		// Simple blocking read with fgets
		char buffer[8192];

		// Direct blocking read
		if (fgets(buffer, sizeof(buffer), stdin) == nullptr) {
			// EOF or error
			if (thread_should_stop) {
				break;
			}
			if (feof(stdin)) {
				print_line("MCP Stdio: EOF reached");
				break;
			}
			print_line("MCP Stdio: read error");
			break;
		}

		// Verify we got data
		size_t len = strlen(buffer);
		if (len == 0) {
			continue;
		}

		// Safe string conversion
		String line = String::utf8(buffer, len);
		line = line.strip_edges();

		if (line.is_empty()) {
			continue;
		}

		print_line("MCP Stdio: Received request (" + itos(line.length()) + " chars)");

		// Process the request
		String response = server->process_request(line);

		// Write response to stdout
		if (!response.is_empty()) {
			CharString utf8_response = response.utf8();
			fprintf(stdout, "%s\n", utf8_response.get_data());
			fflush(stdout);
			print_line("MCP Stdio: Sent response (" + itos(response.length()) + " chars)");
		}
	}

	print_line("MCP Stdio: Stdin reader thread exiting");
}

void MCPStdioHandler::start() {
	if (running) {
		return;
	}

	running = true;
	server_status = "starting";

	print_line("=== MCP Server stdio mode ===");
	print_line("MCP Server starting in stdio mode...");
	print_line("Reading JSON-RPC requests from stdin...");
	print_line("Writing responses to stdout...");
	print_line("---");

	// Start the stdin reader thread
	thread_should_stop = false;
	stdio_thread = memnew(Thread);
	Thread::ID thread_id = stdio_thread->start(_stdio_thread_func, this);

	if (thread_id != Thread::UNASSIGNED_ID) {
		server_status = "running";
		print_line("MCP Server stdio thread started");
	} else {
		server_status = "error";
		running = false;
		memdelete(stdio_thread);
		stdio_thread = nullptr;
		print_line("ERROR: Failed to start MCP stdio thread");
	}
}

void MCPStdioHandler::stop() {
	if (!running) {
		return;
	}

	print_line("MCP Stdio: Stopping...");

	// Signal the thread to stop
	thread_should_stop = true;

	// Wait for thread to finish
	if (stdio_thread) {
		stdio_thread->wait_to_finish();
		memdelete(stdio_thread);
		stdio_thread = nullptr;
	}

	running = false;
	server_status = "stopped";
	print_line("MCP Server stopped");
}

String MCPStdioHandler::process_line(const String &p_line) {
	if (p_line.is_empty()) {
		return "";
	}

	// If server doesn't exist, create a new one
	if (!server.is_valid()) {
		server.instantiate();
		server->initialize();
	}

	// Process the request
	return server->process_request(p_line);
}
