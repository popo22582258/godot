/**************************************************************************/
/*  mcp_tools.cpp                                                          */
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

#include "mcp_tools.h"

#include "core/io/file_access.h"
#include "core/string/string_builder.h"

#ifdef MODULE_GDSCRIPT_ENABLED
#include "modules/gdscript/gdscript.h"
#include "modules/gdscript/gdscript_cache.h"
#include "modules/gdscript/gdscript_parser.h"
#include "modules/gdscript/gdscript_analyzer.h"
#include "modules/gdscript/gdscript_warning.h"
#endif

MCPTools::ValidationResult MCPTools::validate_gdscript(const String &script_path) {
	ValidationResult result;

#ifdef MODULE_GDSCRIPT_ENABLED
	Ref<FileAccess> fa = FileAccess::create_for_path(script_path);
	if (!fa.is_valid()) {
		result.valid = false;
		result.error_message = "Cannot open file: " + script_path;
		result.error_code = "FILE_NOT_FOUND";
		return result;
	}

	String content = fa->get_as_text();
	fa->close();

	return validate_gdscript_content(content);
#else
	result.valid = false;
	result.error_message = "GDScript module not enabled";
	result.error_code = "GDSCRIPT_DISABLED";
	return result;
#endif
}

MCPTools::ValidationResult MCPTools::validate_gdscript_content(const String &content) {
	ValidationResult result;

#ifdef MODULE_GDSCRIPT_ENABLED
	GDScriptParser parser;
	Error err = parser.parse(content, "", false);

	if (err != OK) {
		result.valid = false;
		result.error_message = "Parse error";
		result.error_code = "PARSE_ERROR";

		// Try to get error location from parser
		const List<GDScriptParser::ParserError> &errors = parser.get_errors();
		if (!errors.is_empty()) {
			const GDScriptParser::ParserError &error = errors.front()->get();
			result.error_message = error.message;
			result.error_line = error.line;
			result.error_column = error.column;
		}

		// Add errors as diagnostics
		for (const GDScriptParser::ParserError &error : errors) {
			Dictionary diag;
			diag["severity"] = "error";
			diag["line"] = error.line;
			diag["column"] = error.column;
			diag["message"] = error.message;
			diag["code"] = "PARSE_ERROR";
			result.diagnostics.append(diag);
		}

		return result;
	}

	// Run analyzer to check for semantic errors
	GDScriptAnalyzer analyzer(&parser);
	err = analyzer.analyze();

	if (err != OK) {
		result.valid = false;
		result.error_message = "Analysis error";
		result.error_code = "ANALYSIS_ERROR";
	}

	// Add warnings as diagnostics
	const List<GDScriptWarning> &warnings = parser.get_warnings();
	for (const GDScriptWarning &warning : warnings) {
		Dictionary diag;
		diag["severity"] = "warning";
		diag["line"] = warning.line;
		diag["column"] = warning.column;
		diag["message"] = warning.get_message();
		diag["code"] = GDScriptWarning::get_code_string(warning.code);
		result.diagnostics.append(diag);
		result.warnings.append(diag);
	}

#else
	result.valid = false;
	result.error_message = "GDScript module not enabled";
	result.error_code = "GDSCRIPT_DISABLED";
#endif

	return result;
}

MCPTools::LintResult MCPTools::lint_gdscript(const String &script_path) {
	LintResult result;
	result.file_path = script_path;

#ifdef MODULE_GDSCRIPT_ENABLED
	Ref<FileAccess> fa = FileAccess::create_for_path(script_path);
	if (!fa.is_valid()) {
		result.success = false;
		Dictionary diag;
		diag["severity"] = "error";
		diag["message"] = "Cannot open file: " + script_path;
		diag["code"] = "FILE_NOT_FOUND";
		result.diagnostics.append(diag);
		result.error_count = 1;
		return result;
	}

	String content = fa->get_as_text();
	fa->close();

	return lint_gdscript_content(content, script_path);
#else
	result.success = false;
	Dictionary diag;
	diag["severity"] = "error";
	diag["message"] = "GDScript module not enabled";
	diag["code"] = "GDSCRIPT_DISABLED";
	result.diagnostics.append(diag);
	result.error_count = 1;
#endif

	return result;
}

MCPTools::LintResult MCPTools::lint_gdscript_content(const String &content, const String &source_path) {
	LintResult result;
	result.file_path = source_path;

#ifdef MODULE_GDSCRIPT_ENABLED
	GDScriptParser parser;
	Error err = parser.parse(content, source_path, false);

	// Even if parse fails, we still get some warnings
	const List<GDScriptWarning> &warnings = parser.get_warnings();

	for (const GDScriptWarning &warning : warnings) {
		Dictionary diag = _warning_to_diagnostic(
				warning.code,
				warning.get_message(),
				warning.line,
				warning.column);

		String severity = "info";
		if (warning.is_error) {
			severity = "error";
			result.error_count++;
		} else {
			severity = "warning";
			result.warning_count++;
		}
		diag["severity"] = severity;
		result.diagnostics.append(diag);
	}

	// If parse failed, add that as an error
	if (err != OK) {
		const List<GDScriptParser::ParserError> &errors = parser.get_errors();
		for (const GDScriptParser::ParserError &error : errors) {
			Dictionary diag;
			diag["severity"] = "error";
			diag["line"] = error.line;
			diag["column"] = error.column;
			diag["message"] = error.message;
			diag["code"] = "PARSE_ERROR";
			result.diagnostics.append(diag);
			result.error_count++;
		}
	}

	// Run analyzer for deeper analysis
	if (err == OK) {
		GDScriptAnalyzer analyzer(&parser);
		err = analyzer.analyze();

		// Add any analysis warnings
		const List<GDScriptWarning> &analysis_warnings = parser.get_warnings();
		for (const GDScriptWarning &warning : analysis_warnings) {
			bool already_added = false;
			for (const Dictionary &d : result.diagnostics) {
				if (d.has("line") && d.has("code") &&
						(int)d["line"] == warning.line &&
						(String)d["code"] == GDScriptWarning::get_code_string(warning.code)) {
					already_added = true;
					break;
				}
			}
			if (!already_added) {
				Dictionary diag = _warning_to_diagnostic(
						warning.code,
						warning.get_message(),
						warning.line,
						warning.column);
				diag["severity"] = warning.is_error ? "error" : "warning";
				result.diagnostics.append(diag);
				if (warning.is_error) {
					result.error_count++;
				} else {
					result.warning_count++;
				}
			}
		}
	}

#else
	result.success = false;
	Dictionary diag;
	diag["severity"] = "error";
	diag["message"] = "GDScript module not enabled";
	diag["code"] = "GDSCRIPT_DISABLED";
	result.diagnostics.append(diag);
	result.error_count = 1;
#endif

	return result;
}

Array MCPTools::get_script_symbols_internal(const String &script_path) {
	Array symbols;

#ifdef MODULE_GDSCRIPT_ENABLED
	Ref<FileAccess> fa = FileAccess::create_for_path(script_path);
	if (!fa.is_valid()) {
		return symbols;
	}

	String content = fa->get_as_text();
	fa->close();

	GDScriptParser parser;
	Error err = parser.parse(content, script_path, false);
	if (err != OK) {
		return symbols;
	}

	// Get class functions
	const Map<String, GDScriptParser::FunctionNode *> &functions = parser.get_functions();
	for (const KeyValue<String, GDScriptParser::FunctionNode *> &E : functions) {
		Dictionary func_sym;
		func_sym["name"] = E.key;
		func_sym["type"] = "function";
		func_sym["line"] = E.value->start_line;
		func_sym["end_line"] = E.value->end_line;
		func_sym["is_static"] = E.value->is_static;
		func_sym["is_virtual"] = E.value->is_virtual;

		// Return type
		if (E.value->return_type) {
			func_sym["return_type"] = E.value->return_type->to_string();
		}

		// Parameters
		Array params;
		for (const GDScriptParser::ParameterNode *param : E.value->parameters) {
			Dictionary param_info;
			param_info["name"] = param->identifier;
			if (param->type_expression) {
				param_info["type"] = param->type_expression->to_string();
			}
			params.append(param_info);
		}
		func_sym["parameters"] = params;

		symbols.append(func_sym);
	}

	// Get class variables
	const Map<String, GDScriptParser::VariableNode *> &variables = parser.get_variables();
	for (const KeyValue<String, GDScriptParser::VariableNode *> &E : variables) {
		Dictionary var_sym;
		var_sym["name"] = E.key;
		var_sym["type"] = "variable";
		var_sym["line"] = E.value->start_line;
		var_sym["is_private"] = E.key.begins_with("_");
		var_sym["is_static"] = E.value->is_static;

		if (E.value->type_expression) {
			var_sym["var_type"] = E.value->type_expression->to_string();
		}

		symbols.append(var_sym);
	}

	// Get signals
	const Map<String, GDScriptParser::SignalNode *> &signals = parser.get_signals();
	for (const KeyValue<String, GDScriptParser::SignalNode *> &E : signals) {
		Dictionary sig_sym;
		sig_sym["name"] = E.key;
		sig_sym["type"] = "signal";
		sig_sym["line"] = E.value->start_line;

		Array params;
		for (const GDScriptParser::ParameterNode *param : E.value->parameters) {
			Dictionary param_info;
			param_info["name"] = param->identifier;
			if (param->type_expression) {
				param_info["type"] = param->type_expression->to_string();
			}
			params.append(param_info);
		}
		sig_sym["parameters"] = params;

		symbols.append(sig_sym);
	}

#endif

	return symbols;
}

Dictionary MCPTools::get_scene_tree_internal(Node *p_root, bool include_properties, int max_depth) {
	Dictionary result;

	if (!p_root) {
		return result;
	}

	result["name"] = p_root->get_name();
	result["type"] = p_root->get_class_name();
	result["path"] = p_root->get_path();

	if (include_properties) {
		// Get exported properties
		Array properties;
		List<PropertyInfo> props;
		p_root->get_property_list(&props);
		for (const PropertyInfo &pi : props) {
			if (pi.usage & PROPERTY_USAGE_SCRIPT_VARIABLE) {
				Dictionary prop;
				prop["name"] = pi.name;
				prop["type"] = Variant::get_type_name(pi.type);
				prop["value"] = p_root->get(pi.name);
				properties.append(prop);
			}
		}
		result["properties"] = properties;
	}

	// Get script
	Ref<Script> script = p_root->get_script();
	if (script.is_valid()) {
		result["script"] = script->get_path();
	}

	// Get children
	if (max_depth != 0) {
		Array children;
		int child_max_depth = max_depth > 0 ? max_depth - 1 : -1;
		for (int i = 0; i < p_root->get_child_count(); i++) {
			Node *child = p_root->get_child(i);
			Dictionary child_data = get_scene_tree_internal(child, include_properties, child_max_depth);
			children.append(child_data);
		}
		result["children"] = children;
	}

	return result;
}

Dictionary MCPTools::_warning_to_diagnostic(int p_code, const String &p_message, int p_line, int p_column) {
	Dictionary diag;

	String code_str = GDScriptWarning::get_code_string((GDScriptWarning::Code)p_code);
	diag["code"] = code_str;
	diag["line"] = p_line;
	diag["column"] = p_column;
	diag["message"] = p_message;

	// Add suggestions based on warning code
	String suggestion;
	bool auto_fixable = false;

	switch (p_code) {
		case GDScriptWarning::UNTYPED_DECLARATION:
			suggestion = "Add explicit type annotation";
			auto_fixable = false;
			break;
		case GDScriptWarning::UNUSED_VARIABLE:
			suggestion = "Remove unused variable or prefix with underscore";
			auto_fixable = false;
			break;
		case GDScriptWarning::UNUSED_PRIVATE_CLASS_VARIABLE:
			suggestion = "Remove unused private variable";
			auto_fixable = true;
			break;
		case GDScriptWarning::UNUSED_SIGNAL:
			suggestion = "Remove unused signal or connect it";
			auto_fixable = false;
			break;
		case GDScriptWarning::SHADOWED_VARIABLE:
			suggestion = "Rename variable to avoid shadowing";
			auto_fixable = false;
			break;
		case GDScriptWarning::UNSAFE_CAST:
			suggestion = "Use @onready or add null check";
			auto_fixable = false;
			break;
		default:
			break;
	}

	diag["suggestion"] = suggestion;
	diag["auto_fixable"] = auto_fixable;

	return diag;
}
