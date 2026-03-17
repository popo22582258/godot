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

#include "core/object/class_db.h"
#include "core/string/string_builder.h"

void GDTest::_bind_methods() {
	// Bind assertion methods
	ClassDB::bind_method(D_METHOD("assert_true", "condition", "message"), &GDTest::assert_true, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_false", "condition", "message"), &GDTest::assert_false, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_eq", "actual", "expected", "message"), &GDTest::assert_eq, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("assert_ne", "actual", "expected", "message"), &GDTest::assert_ne, DEFVAL(""));

	ClassDB::bind_method(D_METHOD("skip", "reason"), &GDTest::skip);
	ClassDB::bind_method(D_METHOD("fail", "message"), &GDTest::fail);
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

void GDTest::skip(const String &reason) {
	print_line("SKIPPED: " + reason);
}

void GDTest::fail(const String &message) {
	ERR_FAIL_MSG("Test failed: " + message);
}
