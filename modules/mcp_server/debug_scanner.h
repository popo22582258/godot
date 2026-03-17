/**************************************************************************/
/*  debug_scanner.h                                                        */
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
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class DebugScanner : public RefCounted {
	GDCLASS(DebugScanner, RefCounted);

private:
	bool listening = false;
	Array errors;
	Array warnings;
	Array script_errors;
	Array shader_errors;
	int error_count = 0;
	int warning_count = 0;
	int script_error_count = 0;
	int shader_error_count = 0;

public:
	static void _bind_methods();

	// Start listening for errors
	void start_listening();

	// Stop listening for errors
	void stop_listening();

	// Get all errors
	Array get_errors() const;

	// Get all warnings
	Array get_warnings() const;

	// Get script errors
	Array get_script_errors() const;

	// Get shader errors
	Array get_shader_errors() const;

	// Clear all cached errors
	void clear_errors();

	// Get statistics
	Dictionary get_statistics() const;

	// Check if currently listening
	bool is_listening() const;

	// Add an error programmatically (for testing/manual use)
	void add_error(const String &p_message, const String &p_source, int p_line, const String &p_error_code);

	// Add a warning programmatically
	void add_warning(const String &p_message, const String &p_source, int p_line, const String &p_warning_code);

	// Get error by index
	Dictionary get_error(int p_index) const;

	// Get warning by index
	Dictionary get_warning(int p_index) const;

	// Get total count of all issues
	int get_total_issue_count() const;

	// Export errors to JSON format
	String to_json() const;
};
