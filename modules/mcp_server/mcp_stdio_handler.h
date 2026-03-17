/**************************************************************************/
/*  mcp_stdio_handler.h                                                   */
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

class MCPStdioHandler : public RefCounted {
	GDCLASS(MCPStdioHandler, RefCounted);

private:
	bool running = false;
	String server_status;

public:
	void start();
	void stop();
	bool is_running() const { return running; }
	String get_status() const { return server_status; }

	// Process a single line from stdin
	String process_line(const String &p_line);

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("start"), &MCPStdioHandler::start);
		ClassDB::bind_method(D_METHOD("stop"), &MCPStdioHandler::stop);
		ClassDB::bind_method(D_METHOD("is_running"), &MCPStdioHandler::is_running);
		ClassDB::bind_method(D_METHOD("get_status"), &MCPStdioHandler::get_status);
		ClassDB::bind_method(D_METHOD("process_line", "line"), &MCPStdioHandler::process_line);
	}
};
