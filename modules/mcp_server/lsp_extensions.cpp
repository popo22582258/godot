/**************************************************************************/
/*  lsp_extensions.cpp                                                      */
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

#include "lsp_extensions.h"

#include "core/io/file_access.h"

#ifdef MODULE_GDSCRIPT_ENABLED
#include "modules/gdscript/gdscript_cache.h"
#include "modules/gdscript/gdscript_parser.h"
#include "modules/gdscript/gdscript_analyzer.h"
#endif

String LSPExtensions::_get_token_type_name(SemanticTokenType type) {
	switch (type) {
		case SemanticTokenType::TYPE:
			return "type";
		case SemanticTokenType::CLASS:
			return "class";
		case SemanticTokenType::ENUM:
			return "enum";
		case SemanticTokenType::INTERFACE:
			return "interface";
		case SemanticTokenType::STRUCT:
			return "struct";
		case SemanticTokenType::TYPE_PARAMETER:
			return "typeParameter";
		case SemanticTokenType::PARAMETER:
			return "parameter";
		case SemanticTokenType::VARIABLE:
			return "variable";
		case SemanticTokenType::PROPERTY:
			return "property";
		case SemanticTokenType::ENUM_MEMBER:
			return "enumMember";
		case SemanticTokenType::FUNCTION:
			return "function";
		case SemanticTokenType::METHOD:
			return "method";
		case SemanticTokenType::MODIFIER:
			return "modifier";
		case SemanticTokenType::COMMENT:
			return "comment";
		case SemanticTokenType::STRING:
			return "string";
		case SemanticTokenType::NUMBER:
			return "number";
		case SemanticTokenType::KEYWORD:
			return "keyword";
		case SemanticTokenType::OPERATOR:
			return "operator";
		case SemanticTokenType::PUNCTUATION:
			return "punctuation";
		case SemanticTokenType::BUILTIN_TYPE:
			return "type";
		case SemanticTokenType::BUILTIN_FUNCTION:
			return "function";
		case SemanticTokenType::EVENT:
			return "event";
		case SemanticTokenType::LABEL:
			return "label";
		case SemanticTokenType::NAMESPACE:
			return "namespace";
		case SemanticTokenType::TAG:
			return "tag";
		default:
			return "variable";
	}
}

String LSPExtensions::_get_token_modifier_name(SemanticTokenModifier modifier) {
	switch (modifier) {
		case SemanticTokenModifier::DEFINITION:
			return "definition";
		case SemanticTokenModifier::DECLARATION:
			return "declaration";
		case SemanticTokenModifier::IMPLEMENTATION:
			return "implementation";
		case SemanticTokenModifier::REFERENCE:
			return "reference";
		case SemanticTokenModifier::STATIC:
			return "static";
		case SemanticTokenModifier::DEPRECATED:
			return "deprecated";
		case SemanticTokenModifier::READ_ONLY:
			return "readonly";
		case SemanticTokenModifier::WRITE_ONLY:
			return "writeonly";
		case SemanticTokenModifier::ASYNC:
			return "async";
		case SemanticTokenModifier::ABSTRACT:
			return "abstract";
		case SemanticTokenModifier::CONST:
			return "constant";
		case SemanticTokenModifier::DEFAULT_LIBRARY:
			return "defaultLibrary";
		default:
			return "";
	}
}

Array LSPExtensions::get_semantic_tokens(const String &script_path) {
	Array tokens;

#ifdef MODULE_GDSCRIPT_ENABLED
	Ref<FileAccess> fa = FileAccess::create_for_path(script_path);
	if (!fa.is_valid()) {
		return tokens;
	}

	String content = fa->get_as_text();
	fa->close();

	GDScriptParser parser;
	Error err = parser.parse(content, script_path, false);
	if (err != OK) {
		return tokens;
	}

	// Analyze for semantic information
	GDScriptAnalyzer analyzer(&parser);
	analyzer.analyze();

	// Process functions
	const Map<String, GDScriptParser::FunctionNode *> &functions = parser.get_functions();
	for (const KeyValue<String, GDScriptParser::FunctionNode *> &E : functions) {
		Dictionary token;
		token["type"] = E.value->is_static ? "static" : "method";
		token["start_line"] = E.value->start_line;
		token["start_column"] = 0;
		token["end_line"] = E.value->end_line;
		token["end_column"] = E.value->identifier.end_column;
		token["name"] = E.key;
		tokens.append(token);
	}

	// Process variables
	const Map<String, GDScriptParser::VariableNode *> &variables = parser.get_variables();
	for (const KeyValue<String, GDScriptParser::VariableNode *> &E : variables) {
		Dictionary token;
		token["type"] = "variable";
		token["start_line"] = E.value->start_line;
		token["start_column"] = E.value->start_column;
		token["end_line"] = E.value->end_line;
		token["end_column"] = E.value->end_column;
		token["name"] = E.key;
		token["is_static"] = E.value->is_static;
		token["is_const"] = E.value->is_constant;
		tokens.append(token);
	}

	// Process signals
	const Map<String, GDScriptParser::SignalNode *> &signals = parser.get_signals();
	for (const KeyValue<String, GDScriptParser::SignalNode *> &E : signals) {
		Dictionary token;
		token["type"] = "event";
		token["start_line"] = E.value->start_line;
		token["start_column"] = E.value->start_column;
		token["end_line"] = E.value->end_line;
		token["end_column"] = E.value->end_column;
		token["name"] = E.key;
		tokens.append(token);
	}

#endif

	return tokens;
}

Dictionary LSPExtensions::get_hover_info(const String &script_path, int line, int character) {
	Dictionary hover;

#ifdef MODULE_GDSCRIPT_ENABLED
	// This would integrate with GDScript analyzer to get type information
	hover["content"] = "Hover information for position";
	hover["range"] = Dictionary();
#endif

	return hover;
}

Array LSPExtensions::get_code_actions(const String &script_path, int line, int start_col, int end_col) {
	Array actions;

#ifdef MODULE_GDSCRIPT_ENABLED
	// Quick Fix: Add type annotation
	Dictionary add_type_action;
	add_type_action["title"] = "Add type annotation";
	add_type_action["kind"] = "quickfix";
	add_type_action["edit"] = Dictionary();
	actions.append(add_type_action);

	// Quick Fix: Remove unused variable
	Dictionary remove_unused_action;
	remove_unused_action["title"] = "Remove unused variable";
	remove_unused_action["kind"] = "quickfix";
	remove_unused_action["edit"] = Dictionary();
	actions.append(remove_unused_action);

	// Refactor: Extract to function
	Dictionary extract_action;
	extract_action["title"] = "Extract to function";
	extract_action["kind"] = "refactor";
	extract_action["edit"] = Dictionary();
	actions.append(extract_action);
#endif

	return actions;
}

Array LSPExtensions::get_inline_hints(const String &script_path, int line) {
	Array hints;

#ifdef MODULE_GDSCRIPT_ENABLED
	// Add parameter name hints
	// Add inferred type hints
#endif

	return hints;
}

Dictionary LSPExtensions::semantic_tokens_to_lsp(const Array &tokens) {
	Dictionary result;

	// Convert to LSP semantic tokens format
	// This is a simplified version - full implementation would need to
	// calculate deltas properly

	Array data;
	for (int i = 0; i < tokens.size(); i++) {
		Dictionary token = tokens[i];
		data.append(token.get("delta_line", 0));
		data.append(token.get("delta_start", 0));
		data.append(token.get("length", 0));
		data.append(token.get("token_type", 0));
		data.append(token.get("token_modifiers", 0));
	}

	result["data"] = data;

	return result;
}

Array LSPExtensions::get_completions(const String &script_path, int line, int character) {
	Array completions;

#ifdef MODULE_GDSCRIPT_ENABLED
	// Keyword completions
	String keywords[] = {
		"func", "class", "extends", "static", "const", "var", "enum",
		"if", "elif", "else", "match", "for", "while", "break", "continue",
		"return", "yield", "signal", "tool", "export", "onready", "set", "get",
		"@export", "@onready", "@tool", "@rpc", "@slave", "@remote", "@master"
	};

	for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
		Dictionary completion;
		completion["label"] = keywords[i];
		completion["kind"] = 14; // Keyword
		completion["detail"] = "keyword";
		completions.append(completion);
	}

	// Built-in types
	String types[] = {
		"int", "float", "String", "bool", "Array", "Dictionary",
		"Vector2", "Vector3", "Vector4", "Color", "Transform2D", "Transform3D",
		"Node", "Node2D", "Node3D", "Control", "Object", "RefCounted"
	};

	for (int i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
		Dictionary completion;
		completion["label"] = types[i];
		completion["kind"] = 7; // Type
		completion["detail"] = "built-in type";
		completions.append(completion);
	}

	// Built-in functions
	String builtins[] = {
		"print", "push_error", "push_warning", "assert", "load",
		"preload", "instantiate", "new", "get_node", "get_tree",
		"connect", "disconnect", "emit_signal", "call", "call_deferred"
	};

	for (int i = 0; i < sizeof(builtins) / sizeof(builtins[0]); i++) {
		Dictionary completion;
		completion["label"] = builtins[i];
		completion["kind"] = 1; // Function
		completion["detail"] = "built-in function";
		completions.append(completion);
	}
#endif

	return completions;
}
