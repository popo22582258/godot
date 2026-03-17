/**************************************************************************/
/*  gdtest.cpp                                                             */
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
/* permit persons to whom the Software is furnished to doso, subject to  */
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

#include "gdtest.h"

#include "core/io/dir_access.h"
#include "core/object/class_db.h"
#include "core/string/string_builder.h"

void GDTest::_bind_methods() {
	// Assertions are bound in subclasses
}

void GDTest::assert_true(bool condition, const String &message) {
	if (!condition) {
		fail(message.is_empty() ? "Assertion failed: expected true" : message);
	}
}

void GDTest::assert_false(bool condition, const String &message) {
	if (condition) {
		fail(message.is_empty() ? "Assertion failed: expected false" : message);
	}
}

void GDTest::assert_eq(const Variant &actual, const Variant &expected, const String &message) {
	if (actual != expected) {
		String msg = message.is_empty() ? "Assertion failed: expected " + expected.stringify() + " but got " + actual.stringify() : message;
		fail(msg);
	}
}

void GDTest::assert_ne(const Variant &actual, const Variant &expected, const String &message) {
	if (actual == expected) {
		String msg = message.is_empty() ? "Assertion failed: expected not " + expected.stringify() : message;
		fail(msg);
	}
}

void GDTest::assert_gt(const Variant &a, const Variant &b, const String &message) {
	if (!(a > b)) {
		String msg = message.is_empty() ? "Assertion failed: expected " + a.stringify() + " > " + b.stringify() : message;
		fail(msg);
	}
}

void GDTest::assert_ge(const Variant &a, const Variant &b, const String &message) {
	if (!(a >= b)) {
		String msg = message.is_empty() ? "Assertion failed: expected " + a.stringify() + " >= " + b.stringify() : message;
		fail(msg);
	}
}

void GDTest::assert_lt(const Variant &a, const Variant &b, const String &message) {
	if (!(a < b)) {
		String msg = message.is_empty() ? "Assertion failed: expected " + a.stringify() + " < " + b.stringify() : message;
		fail(msg);
	}
}

void GDTest::assert_le(const Variant &a, const Variant &b, const String &message) {
	if (!(a <= b)) {
		String msg = message.is_empty() ? "Assertion failed: expected " + a.stringify() + " <= " + b.stringify() : message;
		fail(msg);
	}
}

void GDTest::assert_null(const Variant &value, const String &message) {
	if (!value.is_null()) {
		String msg = message.is_empty() ? "Assertion failed: expected null" : message;
		fail(msg);
	}
}

void GDTest::assert_not_null(const Variant &value, const String &message) {
	if (value.is_null()) {
		String msg = message.is_empty() ? "Assertion failed: expected non-null" : message;
		fail(msg);
	}
}

void GDTest::assertThrows(const Callable &callable, const String &message) {
	// This is a simplified version - in reality you'd need to catch exceptions
	// GDScript doesn't have try-catch in the same way, so this is a placeholder
}

void GDTest::skip(const String &reason) {
	print_line("SKIPPED: " + reason);
}

void GDTest::fail(const String &message) {
	ERR_FAIL_MSG("Test failed: " + message);
}

void GDTestRunner::_bind_methods() {
	ClassDB::bind_method(D_METHOD("run_tests_in_directory", "dir_path"), &GDTestRunner::run_tests_in_directory);
	ClassDB::bind_method(D_METHOD("run_test_file", "test_file_path"), &GDTestRunner::run_test_file);
	ClassDB::bind_method(D_METHOD("run_all_tests"), &GDTestRunner::run_all_tests);
	ClassDB::bind_method(D_METHOD("get_results"), &GDTestRunner::get_results);
	ClassDB::bind_method(D_METHOD("get_failed_tests"), &GDTestRunner::get_failed_tests);
	ClassDB::bind_method(D_METHOD("get_passed_tests"), &GDTestRunner::get_passed_tests);
	ClassDB::bind_method(D_METHOD("set_stop_on_failure", "enabled"), &GDTestRunner::set_stop_on_failure);
	ClassDB::bind_method(D_METHOD("is_stop_on_failure"), &GDTestRunner::is_stop_on_failure);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "stop_on_failure"), "set_stop_on_failure", "is_stop_on_failure");
}

GDTestRunner::SuiteResult GDTestRunner::run_tests_in_directory(const String &dir_path) {
	SuiteResult suite_result;
	suite_result.suite_path = dir_path;

	_discover_tests_in_directory(dir_path, test_suites);

	// Run each discovered test
	for (int i = 0; i < test_suites.size(); i++) {
		Dictionary test_info = test_suites[i];
		// In a real implementation, this would load and run the test
		suite_result.total++;
	}

	return suite_result;
}

GDTestRunner::SuiteResult GDTestRunner::run_test_file(const String &test_file_path) {
	SuiteResult suite_result;
	suite_result.suite_path = test_file_path;

	// Load and run the test file
	Ref<Script> test_script = ResourceLoader::get_singleton()->load(test_file_path);
	if (!test_script.is_valid()) {
		suite_result.failed = 1;
		Dictionary error_result;
		error_result["test_name"] = "load_error";
		error_result["passed"] = false;
		error_result["error_message"] = "Failed to load test file: " + test_file_path;
		suite_result.test_results.append(error_result);
		return suite_result;
	}

	suite_result.total = 1;
	// In a real implementation, this would instantiate and run the test

	return suite_result;
}

Array GDTestRunner::run_all_tests() {
	Array all_results;

	// Run all discovered test suites
	for (int i = 0; i < test_suites.size(); i++) {
		Dictionary test_info = test_suites[i];
		String test_path = test_info["path"];

		SuiteResult suite_result = run_test_file(test_path);

		Dictionary suite_dict;
		suite_dict["path"] = test_path;
		suite_dict["total"] = suite_result.total;
		suite_dict["passed"] = suite_result.passed;
		suite_dict["failed"] = suite_result.failed;
		suite_dict["skipped"] = suite_result.skipped;
		suite_dict["results"] = suite_result.test_results;

		all_results.append(suite_dict);
	}

	return all_results;
}

Array GDTestRunner::get_failed_tests() const {
	Array failed;

	for (const KeyValue<String, Dictionary> &E : results) {
		const Dictionary &test_result = E.value;
		if (!test_result["passed"]) {
			failed.append(test_result);
		}
	}

	return failed;
}

Array GDTestRunner::get_passed_tests() const {
	Array passed;

	for (const KeyValue<String, Dictionary> &E : results) {
		const Dictionary &test_result = E.value;
		if (test_result["passed"]) {
			passed.append(test_result);
		}
	}

	return passed;
}

void GDTestRunner::_discover_tests_in_directory(const String &dir_path, Array &discovered) {
	DirAccess *dir = DirAccess::open(dir_path);
	if (!dir) {
		return;
	}

	dir->list_dir_begin();
	String file_name = dir->get_next();
	while (!file_name.is_empty()) {
		if (file_name == "." || file_name == "..") {
			file_name = dir->get_next();
			continue;
		}

		String full_path = dir_path.path_join(file_name);

		if (dir->current_is_dir()) {
			// Recursively search subdirectories
			_discover_tests_in_directory(full_path, discovered);
		} else if (file_name.ends_with(".gd") && file_name.begins_with("test_")) {
			// Found a test file
			Dictionary test_info;
			test_info["name"] = file_name;
			test_info["path"] = full_path;
			discovered.append(test_info);
		}

		file_name = dir->get_next();
	}

	dir->list_dir_end();
	memdelete(dir);
}

Dictionary GDTestCase::to_dict() const {
	Dictionary dict;
	dict["test_name"] = test_name;
	dict["test_class"] = test_class;
	dict["file_path"] = file_path;
	dict["line_number"] = line_number;
	return dict;
}
