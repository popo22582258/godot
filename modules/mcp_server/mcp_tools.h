/**************************************************************************/
/*  mcp_tools.h                                                            */
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
#include "core/string/string.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class MCPTools {
public:
	// Script validation result
	struct ValidationResult {
		bool valid = true;
		String error_message;
		int error_line = -1;
		int error_column = -1;
		String error_code;
		Array warnings;
		Array diagnostics;
	};

	// Script symbol (function, variable, signal)
	struct Symbol {
		String name;
		String type;
		int line = -1;
		int end_line = -1;
		String documentation;
		Array parameters;
		bool is_private = false;
		bool is_static = false;
		bool is_virtual = false;
	};

	// Lint rule
	struct LintRule {
		String code;
		String name;
		String description;
		String severity; // error, warning, info
		String suggestion;
		bool auto_fixable = false;
	};

	// Lint result
	struct LintResult {
		String file_path;
		bool success = true;
		Array diagnostics; // Array of Dictionary with rule, line, column, message, suggestion, auto_fixable
		int error_count = 0;
		int warning_count = 0;
		int info_count = 0;
	};

	static ValidationResult validate_gdscript(const String &script_path);
	static ValidationResult validate_gdscript_content(const String &content);
	static LintResult lint_gdscript(const String &script_path);
	static LintResult lint_gdscript_content(const String &content, const String &source_path = "");

	static Array get_script_symbols_internal(const String &script_path);
	static Dictionary get_scene_tree_internal(Node *p_root, bool include_properties = true, int max_depth = -1);

private:
	static Dictionary _warning_to_diagnostic(int p_code, const String &p_message, int p_line, int p_column);
};
