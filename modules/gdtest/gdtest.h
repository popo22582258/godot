/**************************************************************************/
/*  gdtest.h                                                               */
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
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "scene/main/node.h"

class GDTest : public Node {
	GDCLASS(GDTest, Node);

protected:
	static void _bind_methods();

public:
	// Virtual test methods to override
	virtual void set_up() {}
	virtual void tear_down() {}
	virtual void run_test() {}

	// Assertion methods
	void assert_true(bool condition, const String &message = "");
	void assert_false(bool condition, const String &message = "");
	void assert_eq(const Variant &actual, const Variant &expected, const String &message = "");
	void assert_ne(const Variant &actual, const Variant &expected, const String &message = "");

	// Test utilities
	void skip(const String &reason = "");
	void fail(const String &message = "");
};

class GDTestCase : public RefCounted {
	GDCLASS(GDTestCase, RefCounted);

public:
	String test_name;
	String test_class;
	String file_path;
	int line_number = 0;

	GDTestCase() {}
	GDTestCase(const String &p_name, const String &p_class, const String &p_file, int p_line = 0) :
			test_name(p_name), test_class(p_class), file_path(p_file), line_number(p_line) {}
};
