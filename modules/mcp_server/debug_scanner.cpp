/**************************************************************************/
/*  debug_scanner.cpp                                                      */
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

#include "debug_scanner.h"

#include "core/object/class_db.h"
#include "core/variant/variant.h"
#include "core/io/json.h"
#include "core/os/time.h"

void DebugScanner::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_listening"), &DebugScanner::start_listening);
	ClassDB::bind_method(D_METHOD("stop_listening"), &DebugScanner::stop_listening);
	ClassDB::bind_method(D_METHOD("get_errors"), &DebugScanner::get_errors);
	ClassDB::bind_method(D_METHOD("get_warnings"), &DebugScanner::get_warnings);
	ClassDB::bind_method(D_METHOD("get_script_errors"), &DebugScanner::get_script_errors);
	ClassDB::bind_method(D_METHOD("get_shader_errors"), &DebugScanner::get_shader_errors);
	ClassDB::bind_method(D_METHOD("clear_errors"), &DebugScanner::clear_errors);
	ClassDB::bind_method(D_METHOD("get_statistics"), &DebugScanner::get_statistics);
	ClassDB::bind_method(D_METHOD("is_listening"), &DebugScanner::is_listening);
	ClassDB::bind_method(D_METHOD("add_error", "message", "source", "line", "error_code"), &DebugScanner::add_error);
	ClassDB::bind_method(D_METHOD("add_warning", "message", "source", "line", "warning_code"), &DebugScanner::add_warning);
	ClassDB::bind_method(D_METHOD("get_error", "index"), &DebugScanner::get_error);
	ClassDB::bind_method(D_METHOD("get_warning", "index"), &DebugScanner::get_warning);
	ClassDB::bind_method(D_METHOD("get_total_issue_count"), &DebugScanner::get_total_issue_count);
	ClassDB::bind_method(D_METHOD("to_json"), &DebugScanner::to_json);
}

void DebugScanner::start_listening() {
	listening = true;
}

void DebugScanner::stop_listening() {
	listening = false;
}

Array DebugScanner::get_errors() const {
	return errors;
}

Array DebugScanner::get_warnings() const {
	return warnings;
}

Array DebugScanner::get_script_errors() const {
	return script_errors;
}

Array DebugScanner::get_shader_errors() const {
	return shader_errors;
}

void DebugScanner::clear_errors() {
	errors.clear();
	warnings.clear();
	script_errors.clear();
	shader_errors.clear();
	error_count = 0;
	warning_count = 0;
	script_error_count = 0;
	shader_error_count = 0;
}

Dictionary DebugScanner::get_statistics() const {
	Dictionary stats;
	stats["total_errors"] = error_count;
	stats["total_warnings"] = warning_count;
	stats["script_errors"] = script_error_count;
	stats["shader_errors"] = shader_error_count;
	stats["total_issues"] = get_total_issue_count();
	stats["is_listening"] = listening;
	return stats;
}

bool DebugScanner::is_listening() const {
	return listening;
}

void DebugScanner::add_error(const String &p_message, const String &p_source, int p_line, const String &p_error_code) {
	Dictionary error;
	error["type"] = "error";
	error["message"] = p_message;
	error["source"] = p_source;
	error["line"] = p_line;
	error["error_code"] = p_error_code;
	error["timestamp"] = Time::get_singleton()->get_unix_time_from_system();

	errors.append(error);
	error_count++;

	// Also add to script_errors if source is a .gd file
	if (p_source.ends_with(".gd")) {
		script_errors.append(error);
		script_error_count++;
	}

	// Also add to shader_errors if source is a .gdshader file
	if (p_source.ends_with(".gdshader")) {
		shader_errors.append(error);
		shader_error_count++;
	}
}

void DebugScanner::add_warning(const String &p_message, const String &p_source, int p_line, const String &p_warning_code) {
	Dictionary warning;
	warning["type"] = "warning";
	warning["message"] = p_message;
	warning["source"] = p_source;
	warning["line"] = p_line;
	warning["warning_code"] = p_warning_code;
	warning["timestamp"] = Time::get_singleton()->get_unix_time_from_system();

	warnings.append(warning);
	warning_count++;
}

Dictionary DebugScanner::get_error(int p_index) const {
	if (p_index >= 0 && p_index < errors.size()) {
		return errors[p_index];
	}
	return Dictionary();
}

Dictionary DebugScanner::get_warning(int p_index) const {
	if (p_index >= 0 && p_index < warnings.size()) {
		return warnings[p_index];
	}
	return Dictionary();
}

int DebugScanner::get_total_issue_count() const {
	return error_count + warning_count;
}

String DebugScanner::to_json() const {
	String json = "{\n";
	json += "\"errors\": " + JSON::stringify(errors) + ",\n";
	json += "\"warnings\": " + JSON::stringify(warnings) + ",\n";
	json += "\"statistics\": " + JSON::stringify(get_statistics());
	json += "\n}";
	return json;
}
