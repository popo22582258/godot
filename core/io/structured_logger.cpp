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

#include "core/os/os.h"
#include "core/string/string_builder.h"

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

String StructuredLogger::get_timestamp() {
	OS::Time time = OS::get_singleton()->get_time();
	OS::Date date = OS::get_singleton()->get_date();

	String timestamp;
	timestamp.sprintf("%04d-%02d-%02dT%02d:%02d:%02dZ",
			(int)date.year,
			(int)date.month,
			(int)date.day,
			(int)time.hour,
			(int)time.minute,
			(int)time.second);
	return timestamp;
}

String StructuredLogger::escape_json_string(const String &p_str) {
	String escaped = p_str;
	escaped = escaped.replace("\\", "\\\\");
	escaped = escaped.replace("\"", "\\\"");
	escaped = escaped.replace("\n", "\\n");
	escaped = escaped.replace("\r", "\\r");
	escaped = escaped.replace("\t", "\\t");
	return escaped;
}

void StructuredLogger::logv(const char *p_format, va_list p_list, bool p_err) {
	String msg;
	msg.vprintf(p_format, p_list);

	if (json_output_enabled) {
		log_structured(p_err ? LogType::ERROR : LogType::INFO, msg);
	} else if (stdout_plain_enabled) {
		// Print normally
		if (p_err) {
			logf_error("%s\n", msg.utf8().get_data());
		} else {
			logf("%s\n", msg.utf8().get_data());
		}
	}
}

void StructuredLogger::log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, bool p_editor_notify, ErrorType p_type, const Vector<Ref<ScriptBacktrace>> &p_script_backtraces) {
	String msg;
	if (p_code && p_code[0]) {
		msg = "Parser Error: ";
		msg += p_code;
	}
	if (p_rationale && p_rationale[0]) {
		if (!msg.is_empty()) {
			msg += " ";
		}
		msg += p_rationale;
	}

	Dictionary data;
	data["function"] = p_function;
	data["file"] = p_file;
	data["line"] = p_line;
	if (p_code && p_code[0]) {
		data["code"] = p_code;
	}
	if (p_rationale && p_rationale[0]) {
		data["rationale"] = p_rationale;
	}

	switch (p_type) {
		case ERR_WARNING:
			log_structured(LogType::WARNING, msg, data);
			break;
		case ERR_SCRIPT:
		case ERR_SHADER:
		case ERR_ERROR:
		default:
			log_structured(LogType::ERROR, msg, data);
			break;
	}
}

void StructuredLogger::log_structured(LogType p_type, const String &p_message, const Dictionary &p_data) {
	String type_str;
	switch (p_type) {
		case LogType::INFO:
			type_str = "info";
			break;
		case LogType::WARNING:
			type_str = "warning";
			break;
		case LogType::ERROR:
			type_str = "error";
			break;
		case LogType::DEBUG:
			type_str = "debug";
			break;
		case LogType::RESULT:
			type_str = "result";
			break;
	}

	Dictionary log_obj;
	log_obj["type"] = type_str;
	log_obj["timestamp"] = get_timestamp();
	log_obj["message"] = p_message;

	if (!p_data.is_empty()) {
		log_obj["data"] = p_data;
	}

	// Convert to JSON string
	String json_str = JSON::stringify(log_obj, "\t", false);

	// Output as JSON Lines format (one JSON per line)
	if (stdout_plain_enabled) {
		// Print to stdout/stderr
		ERR_PRINT("%s\n", json_str.utf8().get_data());
	} else {
		// JSON only
		printf("%s\n", json_str.utf8().get_data());
	}
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
	Dictionary data = p_data;
	data["command"] = p_command;
	data["success"] = p_success;

	log_structured(LogType::RESULT, p_success ? "Command succeeded" : "Command failed", data);
}
