/**************************************************************************/
/*  hot_reload_helper.h                                                     */
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

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "scene/main/node.h"

class HotReloadHelper : public Node {
	GDCLASS(HotReloadHelper, Node);

private:
	float check_interval = 1.0;
	bool auto_reload_enabled = false;

public:
	void set_check_interval(float p_interval) { check_interval = p_interval; }
	float get_check_interval() const { return check_interval; }

	void set_auto_reload_enabled(bool p_enabled) { auto_reload_enabled = p_enabled; }
	bool is_auto_reload_enabled() const { return auto_reload_enabled; }

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("set_check_interval", "interval"), &HotReloadHelper::set_check_interval);
		ClassDB::bind_method(D_METHOD("get_check_interval"), &HotReloadHelper::get_check_interval);
		ClassDB::bind_method(D_METHOD("set_auto_reload_enabled", "enabled"), &HotReloadHelper::set_auto_reload_enabled);
		ClassDB::bind_method(D_METHOD("is_auto_reload_enabled"), &HotReloadHelper::is_auto_reload_enabled);
	}
};

class ScriptReloader : public RefCounted {
	GDCLASS(ScriptReloader, RefCounted);

public:
	static bool has_script_changed(const String &script_path) { return false; }
	static uint64_t get_script_modified_time(const String &script_path) { return 0; }
};
