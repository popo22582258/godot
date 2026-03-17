/**************************************************************************/
/*  hot_reload_helper.cpp                                                   */
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

#include "hot_reload_helper.h"

#include "core/io/file_access.h"
#include "core/object/class_db.h"
#include "scene/main/scene_tree.h"

#ifdef MODULE_GDSCRIPT_ENABLED
#include "modules/gdscript/gdscript.h"
#include "modules/gdscript/gdscript_cache.h"
#endif

void HotReloadHelper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("reload_script", "script_path"), &HotReloadHelper::reload_script);
	ClassDB::bind_method(D_METHOD("reload_scene", "scene_path"), &HotReloadHelper::reload_scene);
	ClassDB::bind_method(D_METHOD("reload_all_scripts"), &HotReloadHelper::reload_all_scripts);

	ClassDB::bind_method(D_METHOD("set_check_interval", "interval"), &HotReloadHelper::set_check_interval);
	ClassDB::bind_method(D_METHOD("get_check_interval"), &HotReloadHelper::get_check_interval);

	ClassDB::bind_method(D_METHOD("set_auto_reload_enabled", "enabled"), &HotReloadHelper::set_auto_reload_enabled);
	ClassDB::bind_method(D_METHOD("is_auto_reload_enabled"), &HotReloadHelper::is_auto_reload_enabled);

	ClassDB::bind_method(D_METHOD("watch_file", "path"), &HotReloadHelper::watch_file);
	ClassDB::bind_method(D_METHOD("unwatch_file", "path"), &HotReloadHelper::unwatch_file);
	ClassDB::bind_method(D_METHOD("unwatch_all_files"), &HotReloadHelper::unwatch_all_files);

	ClassDB::bind_method(D_METHOD("get_pending_events"), &HotReloadHelper::get_pending_events);
	ClassDB::bind_method(D_METHOD("clear_pending_events"), &HotReloadHelper::clear_pending_events);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "check_interval"), "set_check_interval", "get_check_interval");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_reload_enabled"), "set_auto_reload_enabled", "is_auto_reload_enabled");
}

void HotReloadHelper::_process(float delta) {
	if (!auto_reload_enabled || watched_files.is_empty()) {
		return;
	}

	time_since_last_check += delta;
	if (time_since_last_check >= check_interval) {
		time_since_last_check = 0.0;
		_check_for_changes();
	}
}

void HotReloadHelper::_check_for_changes() {
	Vector<String> changed_files;

	for (const KeyValue<String, uint64_t> &E : watched_files) {
		if (_has_file_changed(E.key)) {
			changed_files.append(E.key);
		}
	}

	// Reload changed files
	for (const String &path : changed_files) {
		Dictionary result = reload_script(path);
		pending_events.append(result);
	}
}

bool HotReloadHelper::_has_file_changed(const String &p_path) {
	if (!watched_files.has(p_path)) {
		return false;
	}

	uint64_t last_modified = watched_files[p_path];
	Ref<FileAccess> fa = FileAccess::create_for_path(p_path);
	if (!fa.is_valid()) {
		return false;
	}

	// Note: FileAccess doesn't provide modification time directly in a portable way
	// This would need platform-specific implementation
	return false;
}

void HotReloadHelper::set_check_interval(float p_interval) {
	check_interval = p_interval;
}

void HotReloadHelper::set_auto_reload_enabled(bool p_enabled) {
	auto_reload_enabled = p_enabled;
}

void HotReloadHelper::watch_file(const String &p_path) {
	watched_files[p_path] = 0;
}

void HotReloadHelper::unwatch_file(const String &p_path) {
	watched_files.erase(p_path);
}

void HotReloadHelper::unwatch_all_files() {
	watched_files.clear();
}

Array HotReloadHelper::get_pending_events() {
	return pending_events;
}

void HotReloadHelper::clear_pending_events() {
	pending_events.clear();
}

Dictionary HotReloadHelper::signal_script_reloaded(const String &p_script_path, bool p_success, const String &p_error_message) {
	Dictionary event;
	event["event"] = "script_reloaded";
	event["path"] = p_script_path;
	event["success"] = p_success;
	event["error_message"] = p_error_message;
	event["timestamp"] = Time::get_singleton()->get_datetime_string_from_system();
	return event;
}

Dictionary HotReloadHelper::signal_scene_updated(const String &p_scene_path, const Array &p_nodes_changed) {
	Dictionary event;
	event["event"] = "scene_updated";
	event["path"] = p_scene_path;
	event["nodes_changed"] = p_nodes_changed;
	event["timestamp"] = Time::get_singleton()->get_datetime_string_from_system();
	return event;
}

Dictionary HotReloadHelper::reload_script(const String &p_script_path) {
	Dictionary result = _reload_gdscript(p_script_path, false);

	// Add as pending event
	Dictionary event;
	event["event"] = "script_reloaded";
	event["path"] = p_script_path;
	event["success"] = result["success"];
	if (result.has("error_message")) {
		event["error_message"] = result["error_message"];
	}
	pending_events.append(event);

	return result;
}

Dictionary HotReloadHelper::reload_scene(const String &p_scene_path) {
	Dictionary result;
	result["success"] = false;

	SceneTree *st = get_tree();
	if (!st) {
		result["error_message"] = "Scene tree not available";
		return result;
	}

	Error err = st->change_scene(p_scene_path);
	if (err != OK) {
		result["error_message"] = "Failed to reload scene: " + p_scene_path;
		return result;
	}

	result["success"] = true;
	result["path"] = p_scene_path;

	// Add as pending event
	Dictionary event = signal_scene_updated(p_scene_path, Array());
	pending_events.append(event);

	return result;
}

Dictionary HotReloadHelper::reload_all_scripts() {
	Dictionary result;
	result["success"] = true;
	result["reloaded_count"] = 0;

#ifdef MODULE_GDSCRIPT_ENABLED
	// Call GDScriptLanguage's reload_all_scripts
	ScriptLanguage *lang = ScriptServer::get_language("GDScript");
	if (lang) {
		lang->reload_all_scripts();
		result["reloaded_count"] = 1;
	}
#else
	result["success"] = false;
	result["error_message"] = "GDScript not enabled";
#endif

	// Add as pending event
	Dictionary event;
	event["event"] = "all_scripts_reloaded";
	event["success"] = result["success"];
	event["timestamp"] = Time::get_singleton()->get_datetime_string_from_system();
	pending_events.append(event);

	return result;
}

Dictionary HotReloadHelper::_reload_gdscript(const String &p_path, bool p_soft_reload) {
	Dictionary result;
	result["success"] = false;

#ifdef MODULE_GDSCRIPT_ENABLED
	// Get the script from cache
	Ref<GDScript> script = GDScriptCache::get_ref(p_path);
	if (!script.is_valid()) {
		result["error_message"] = "Script not found in cache: " + p_path;
		return result;
	}

	// Reload the script
	Error err = script->reload(p_soft_reload);
	if (err != OK) {
		result["error_message"] = "Failed to reload script";
		return result;
	}

	result["success"] = true;
	result["path"] = p_path;
	result["soft_reload"] = p_soft_reload;
#else
	result["error_message"] = "GDScript not enabled";
#endif

	return result;
}

// ScriptReloader implementation
ScriptReloader::ReloadResult ScriptReloader::reload_script(const String &script_path, bool keep_state) {
	ReloadResult result;

#ifdef MODULE_GDSCRIPT_ENABLED
	Ref<GDScript> script = GDScriptCache::get_ref(script_path);
	if (!script.is_valid()) {
		result.error_message = "Script not found: " + script_path;
		return result;
	}

	Error err = script->reload(keep_state);
	if (err != OK) {
		result.error_message = "Failed to reload script";
		return result;
	}

	result.success = true;
#else
	result.error_message = "GDScript not enabled";
#endif

	return result;
}

ScriptReloader::ReloadResult ScriptReloader::reload_all_scripts(bool keep_state) {
	ReloadResult result;

#ifdef MODULE_GDSCRIPT_ENABLED
	ScriptLanguage *lang = ScriptServer::get_language("GDScript");
	if (lang) {
		lang->reload_all_scripts();
		result.success = true;
	}
#else
	result.error_message = "GDScript not enabled";
#endif

	return result;
}

bool ScriptReloader::has_script_changed(const String &script_path) {
	// This would need to track file modification times
	return false;
}

uint64_t ScriptReloader::get_script_modified_time(const String &script_path) {
	// This would need platform-specific implementation
	return 0;
}
