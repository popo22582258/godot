/**************************************************************************/
/*  scene_json_converter.h                                                 */
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

#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/string/string.h"
#include "core/variant/dictionary.h"
#include "scene/resources/packed_scene.h"

class SceneJSONConverter {
public:
	// Convert TSCN file to JSON
	static Error tscn_to_json(const String &tscn_path, const String &json_path);

	// Convert JSON file to TSCN
	static Error json_to_tscn(const String &json_path, const String &tscn_path);

	// Parse TSCN to Dictionary (for internal use)
	static Error parse_tscn(const String &tscn_path, Dictionary &r_scene_dict);

	// Write Dictionary to TSCN
	static Error write_tscn(const Dictionary &scene_dict, const String &tscn_path);

	// Convert scene node to Dictionary
	static Dictionary node_to_dict(Node *p_node);

	// Create node from Dictionary
	static Node *dict_to_node(const Dictionary &p_dict);

private:
	static Dictionary _resource_to_dict(const Ref<Resource> &p_resource, int &r_next_ext_id);
	static Ref<Resource> _dict_to_resource(const Dictionary &p_dict);

	static Dictionary _node_to_dict_recursive(Node *p_node, int &r_next_node_id);
	static Node *_dict_to_node_recursive(const Dictionary &p_dict, Node *p_parent);

	static String _get_node_type_name(const Node *p_node);
};
