/**************************************************************************/
/*  lsp_extensions.h                                                        */
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

#include "core/string/string.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class LSPExtensions {
public:
	// Semantic token types ( LSP 3.16+ )
	enum class SemanticTokenType {
		TYPE, // type, interface, struct, enum
		CLASS,
		ENUM,
		INTERFACE,
		STRUCT,
		TYPE_PARAMETER,
		PARAMETER,
		VARIABLE, // variable (including constants)
		PROPERTY,
		ENUM_MEMBER,
		FUNCTION,
		METHOD,
		MODIFIER, // readonly, static, abstract, async, etc.
		COMMENT,
		STRING,
		NUMBER,
		KEYWORD,
		OPERATOR,
		PUNCTUATION,
		BUILTIN_TYPE,
		BUILTIN_FUNCTION,
		EVENT, // signal
		LABEL,
		NAMESPACE,
		TAG, // decorator
	};

	// Semantic token modifiers
	enum class SemanticTokenModifier {
		DEFINITION,
		DECLARATION,
		IMPLEMENTATION,
		REFERENCE,
		STATIC,
		DEPRECATED,
		READ_ONLY,
		WRITE_ONLY,
		ASYNC,
		ABSTRACT,
		CONST,
		DEFAULT_LIBRARY,
	};

	// Inline hint types
	enum class InlineHintType {
		PARAMETER_NAME, // Parameter name hints
		INFERRED_TYPE,  // Inferred type hints
		CONST_EXPR,     // Constant expressions
	};

	// Code action kinds
	enum class CodeActionKind {
		QUICK_FIX,
		REFACTOR,
		REFACTOR_EXTRACT,
		REFACTOR_INLINE,
		REFACTOR_MOVE,
		ORGANIZE_IMPORTS,
		ADD_IMPORT,
		REMOVE_UNUSED,
		AUTO_FIX,
	};

	// Get semantic tokens for a GDScript file
	static Array get_semantic_tokens(const String &script_path);

	// Get enhanced hover information
	static Dictionary get_hover_info(const String &script_path, int line, int character);

	// Get code actions
	static Array get_code_actions(const String &script_path, int line, int start_col, int end_col);

	// Get inline hints
	static Array get_inline_hints(const String &script_path, int line);

	// Convert semantic token data to LSP format
	static Dictionary semantic_tokens_to_lsp(const Array &tokens);

	// Get enhanced completion items
	static Array get_completions(const String &script_path, int line, int character);

private:
	static String _get_token_type_name(SemanticTokenType type);
	static String _get_token_modifier_name(SemanticTokenModifier modifier);
};
