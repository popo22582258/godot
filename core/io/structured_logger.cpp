/**************************************************************************/
/*  structured_logger.cpp                                                    */
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

#include "structured_logger.h"

static StructuredLogger *g_structured_logger = nullptr;

StructuredLogger::StructuredLogger() {
	g_structured_logger = this;
}

StructuredLogger *get_structured_logger() {
	return g_structured_logger;
}

void StructuredLogger::set_json_output(bool p_enabled) {
	json_output_enabled = p_enabled;
}

void StructuredLogger::set_stdout_plain(bool p_enabled) {
	stdout_plain_enabled = p_enabled;
}

void StructuredLogger::logv(const char *p_format, va_list p_list, bool p_err) {
	// Simple implementation - just print to stdout
	vprintf(p_format, p_list);
}

void StructuredLogger::log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, bool p_editor_notify, ErrorType p_type, const Vector<Ref<ScriptBacktrace>> &p_script_backtraces) {
	// Simple error logging
	printf("ERROR: %s at %s:%d\n", p_code ? p_code : "unknown", p_file, p_line);
}

void StructuredLogger::log_structured(LogType p_type, const String &p_message, const Dictionary &p_data) {
	// Simple implementation
	printf("%s\n", p_message.utf8().get_data());
}

void StructuredLogger::log_info(const String &p_message, const Dictionary &p_data) {
	log_structured(LogType::INFO, p_message, p_data);
}

void StructuredLogger::log_warning(const String &p_message, const Dictionary &p_data) {
	log_structured(LogType::WARNING, p_message, p_data);
}

void StructuredLogger::log_error_msg(const String &p_message, const Dictionary &p_data) {
	log_structured(LogType::ERROR, p_message, p_data);
}

void StructuredLogger::log_result(const String &p_command, bool p_success, const Dictionary &p_data) {
	log_structured(LogType::RESULT, p_success ? "Command succeeded" : "Command failed", p_data);
}
