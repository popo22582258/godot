/**************************************************************************/
/*  structured_logger.h                                                    */
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

#include "core/io/logger.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

class StructuredLogger : public Logger {
public:
	enum class LogType {
		INFO,
		WARNING,
		ERROR,
		DEBUG,
		RESULT
	};

private:
	bool json_output_enabled = false;
	bool stdout_plain_enabled = true;

	String get_timestamp();
	String escape_json_string(const String &p_str);

public:
	StructuredLogger();

	void set_json_output(bool p_enabled);
	void set_stdout_plain(bool p_enabled);

	virtual void logv(const char *p_format, va_list p_list, bool p_err) override;
	virtual void log_error(const char *p_function, const char *p_file, int p_line, const char *p_code, const char *p_rationale, bool p_editor_notify = false, ErrorType p_type = ERR_ERROR, const Vector<Ref<ScriptBacktrace>> &p_script_backtraces = {}) override;

	void log_structured(LogType p_type, const String &p_message, const Dictionary &p_data = Dictionary());
	void log_info(const String &p_message, const Dictionary &p_data = Dictionary());
	void log_warning(const String &p_message, const Dictionary &p_data = Dictionary());
	void log_error_msg(const String &p_message, const Dictionary &p_data = Dictionary());
	void log_result(const String &p_command, bool p_success, const Dictionary &p_data = Dictionary());
};

// Global access
StructuredLogger *get_structured_logger();
