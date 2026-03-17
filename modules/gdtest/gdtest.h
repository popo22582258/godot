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
#include "core/variant/variant.h"

class GDTest : public Node {
	GDCLASS(GDTest, Node);

public:
	// Test result structure
	struct TestResult {
		String test_name;
		bool passed = false;
		String error_message;
		String assert_type;
		String expected;
		String actual;
		int64_t duration_ms = 0;
	};

	// Test suite result
	struct SuiteResult {
		String suite_path;
		int total = 0;
		int passed = 0;
		int failed = 0;
		int skipped = 0;
		Array test_results;
	};

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
	void assert_gt(const Variant &a, const Variant &b, const String &message = "");
	void assert_ge(const Variant &a, const Variant &b, const String &message = "");
	void assert_lt(const Variant &a, const Variant &b, const String &message = "");
	void assert_le(const Variant &a, const Variant &b, const String &message = "");
	void assert_null(const Variant &value, const String &message = "");
	void assert_not_null(const Variant &value, const String &message = "");
	void assertThrows(const Callable &callable, const String &message = "");

	// Test utilities
	void skip(const String &reason = "");
	void fail(const String &message = "");
};

class GDTestRunner : public Node {
	GDCLASS(GDTestRunner, Node);

private:
	Array test_suites;
	Dictionary results;
	bool stop_on_failure = false;

public:
	// Run all tests in a directory
	SuiteResult run_tests_in_directory(const String &dir_path);

	// Run a specific test file
	SuiteResult run_test_file(const String &test_file_path);

	// Run all discovered tests
	Array run_all_tests();

	// Get results
	Dictionary get_results() const { return results; }
	Array get_failed_tests() const;
	Array get_passed_tests() const;

	// Configuration
	void set_stop_on_failure(bool p_enabled) { stop_on_failure = p_enabled; }
	bool is_stop_on_failure() const { return stop_on_failure; }

protected:
	static void _bind_methods();

private:
	SuiteResult _run_test_class(const String &test_class_path);
	void _discover_tests_in_directory(const String &dir_path, Array &discovered);
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

	Dictionary to_dict() const;
};
