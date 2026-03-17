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

#include "core/object/object.h"
#include "core/string/string.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"

class HotReloadHelper : public Node {
	GDCLASS(HotReloadHelper, Node);

private:
	// File watchers for auto-reload
	HashMap<String, uint64_t> watched_files;
	float check_interval = 1.0;
	float time_since_last_check = 0.0;
	bool auto_reload_enabled = false;

	// Reload event callbacks
	Array pending_events;

public:
	// Signal emitted when a script is reloaded
	// Parameters: script_path (String), success (bool), error_message (String)
	Dictionary signal_script_reloaded(const String &p_script_path, bool p_success, const String &p_error_message = "");

	// Signal emitted when a scene is updated
	// Parameters: scene_path (String), nodes_changed (Array)
	Dictionary signal_scene_updated(const String &p_scene_path, const Array &p_nodes_changed = Array());

	// Check and reload changed files
	void _process(float delta);

	// Manual reload functions
	Dictionary reload_script(const String &p_script_path);
	Dictionary reload_scene(const String &p_scene_path);
	Dictionary reload_all_scripts();

	// Configuration
	void set_check_interval(float p_interval);
	float get_check_interval() const { return check_interval; }

	void set_auto_reload_enabled(bool p_enabled);
	bool is_auto_reload_enabled() const { return auto_reload_enabled; }

	// File watching
	void watch_file(const String &p_path);
	void unwatch_file(const String &p_path);
	void unwatch_all_files();

	// Get pending events
	Array get_pending_events();
	void clear_pending_events();

protected:
	static void _bind_methods();

private:
	void _check_for_changes();
	bool _has_file_changed(const String &p_path);
	Dictionary _reload_gdscript(const String &p_path, bool p_soft_reload);
};

class ScriptReloader : public RefCounted {
	GDCLASS(ScriptReloader, RefCounted);

public:
	struct ReloadResult {
		bool success = false;
		String error_message;
		Array errors;
		Array warnings;
	};

	static ReloadResult reload_script(const String &script_path, bool keep_state = true);
	static ReloadResult reload_all_scripts(bool keep_state = true);

	static bool has_script_changed(const String &script_path);
	static uint64_t get_script_modified_time(const String &script_path);
};
