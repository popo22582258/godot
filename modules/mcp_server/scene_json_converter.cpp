/**************************************************************************/
/*  scene_json_converter.cpp                                               */
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

#include "scene_json_converter.h"

#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/string/string_builder.h"
#include "scene/main/scene_tree.h"

Error SceneJSONConverter::tscn_to_json(const String &tscn_path, const String &json_path) {
	Dictionary scene_dict;
	Error err = parse_tscn(tscn_path, scene_dict);
	if (err != OK) {
		return err;
	}

	String json_str = JSON::stringify(scene_dict, "\t", false);

	Ref<FileAccess> fa = FileAccess::create_for_path(json_path);
	if (!fa.is_valid()) {
		return ERR_CANT_CREATE;
	}

	fa->store_string(json_str);
	fa->close();

	return OK;
}

Error SceneJSONConverter::json_to_tscn(const String &json_path, const String &tscn_path) {
	Ref<FileAccess> fa = FileAccess::create_for_path(json_path);
	if (!fa.is_valid()) {
		return ERR_CANT_OPEN;
	}

	String json_str = fa->get_as_text();
	fa->close();

	JSON json;
	Error err = json.parse(json_str);
	if (err != OK) {
		return err;
	}

	Dictionary scene_dict = json.get_data();
	if (scene_dict.is_empty()) {
		return ERR_PARSE_ERROR;
	}

	return write_tscn(scene_dict, tscn_path);
}

Error SceneJSONConverter::parse_tscn(const String &tscn_path, Dictionary &r_scene_dict) {
	Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(tscn_path);
	if (!scene.is_valid()) {
		return ERR_CANT_OPEN;
	}

	Node *root = scene->instantiate();
	if (!root) {
		return ERR_CANT_INSTANTIATE;
	}

	r_scene_dict = node_to_dict(root);

	// Add metadata
	r_scene_dict["format_version"] = 1;
	r_scene_dict["type"] = "PackedScene";
	r_scene_dict["source_scene"] = tscn_path;

	root->queue_free();

	return OK;
}

Error SceneJSONConverter::write_tscn(const Dictionary &scene_dict, const String &tscn_path) {
	StringBuilder sb;

	sb.append("[gd_scene load_steps=1 format=3]\n\n");

	// Write external resources
	if (scene_dict.has("resources")) {
		Array resources = scene_dict["resources"];
		for (int i = 0; i < resources.size(); i++) {
			Dictionary res = resources[i];
			sb.append("[ext_resource path=\"");
			sb.append(res["path"]);
			sb.append("\" type=\"");
			sb.append(res["type"]);
			sb.append("\" id=");
			sb.append(res["id"]);
			sb.append("]\n");
		}
		sb.append("\n");
	}

	// Write nodes recursively
	if (scene_dict.has("root")) {
		_write_node_tscn(sb, scene_dict["root"], 1);
	}

	Ref<FileAccess> fa = FileAccess::create_for_path(tscn_path);
	if (!fa.is_valid()) {
		return ERR_CANT_CREATE;
	}

	fa->store_string(sb.string_value());
	fa->close();

	return OK;
}

void SceneJSONConverter::_write_node_tscn(StringBuilder &sb, const Dictionary &node_dict, int indent) {
	String indent_str;
	for (int i = 0; i < indent; i++) {
		indent_str += "\t";
	}

	String type = node_dict["type"];
	String name = node_dict["name"];

	sb.append(indent_str);
	sb.append("[node name=\"");
	sb.append(name);
	sb.append("\" type=\"");
	sb.append(type);
	sb.append("\"]\n");

	// Write properties
	if (node_dict.has("properties")) {
		Array props = node_dict["properties"];
		for (int i = 0; i < props.size(); i++) {
			Dictionary prop = props[i];
			sb.append(indent_str);
			sb.append(prop["name"]);
			sb.append(" = ");
			sb.append(prop["value"]);
			sb.append("\n");
		}
	}

	// Write script
	if (node_dict.has("script")) {
		sb.append(indent_str);
		sb.append("script = ExtResource(\"");
		sb.append(node_dict["script"]);
		sb.append("\")\n");
	}

	// Write children
	if (node_dict.has("children")) {
		Array children = node_dict["children"];
		for (int i = 0; i < children.size(); i++) {
			sb.append("\n");
			_write_node_tscn(sb, children[i], indent);
		}
	}
}

Dictionary SceneJSONConverter::node_to_dict(Node *p_node) {
	Dictionary result;

	result["name"] = p_node->get_name();
	result["type"] = _get_node_type_name(p_node);
	result["path"] = p_node->get_path();

	// Get properties
	Array properties;
	List<PropertyInfo> props;
	p_node->get_property_list(&props);
	for (const PropertyInfo &pi : props) {
		if (pi.usage & PROPERTY_USAGE_STORAGE) {
			Dictionary prop;
			prop["name"] = pi.name;
			prop["type"] = Variant::get_type_name(pi.type);
			prop["value"] = p_node->get(pi.name);
			properties.append(prop);
		}
	}
	if (!properties.is_empty()) {
		result["properties"] = properties;
	}

	// Get script
	Ref<Script> script = p_node->get_script();
	if (script.is_valid()) {
		result["script"] = script->get_path();
	}

	// Get children
	Array children;
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = p_node->get_child(i);
		children.append(node_to_dict(child));
	}
	if (!children.is_empty()) {
		result["children"] = children;
	}

	return result;
}

Node *SceneJSONConverter::dict_to_node(const Dictionary &p_dict) {
	String type = p_dict["type"];
	String name = p_dict["name"];

	Object *obj = ClassDB::create_instance(type);
	if (!obj) {
		return nullptr;
	}

	Node *node = Object::cast_to<Node>(obj);
	if (!node) {
		memdelete(obj);
		return nullptr;
	}

	node->set_name(name);

	// Set properties
	if (p_dict.has("properties")) {
		Array props = p_dict["properties"];
		for (int i = 0; i < props.size(); i++) {
			Dictionary prop = props[i];
			String prop_name = prop["name"];
			Variant prop_value = prop["value"];
			node->set(prop_name, prop_value);
		}
	}

	// Set script
	if (p_dict.has("script")) {
		String script_path = p_dict["script"];
		Ref<Script> script = ResourceLoader::get_singleton()->load(script_path);
		if (script.is_valid()) {
			node->set_script(script);
		}
	}

	// Add children
	if (p_dict.has("children")) {
		Array children = p_dict["children"];
		for (int i = 0; i < children.size(); i++) {
			Node *child = dict_to_node(children[i]);
			if (child) {
				node->add_child(child);
			}
		}
	}

	return node;
}

String SceneJSONConverter::_get_node_type_name(const Node *p_node) {
	// Return the actual class name
	return p_node->get_class_name();
}

Dictionary SceneJSONConverter::_resource_to_dict(const Ref<Resource> &p_resource, int &r_next_ext_id) {
	Dictionary result;
	result["id"] = "ext_" + itoa(r_next_ext_id);
	r_next_ext_id++;

	result["type"] = p_resource->get_class_name();
	result["path"] = p_resource->get_path();

	return result;
}

Ref<Resource> SceneJSONConverter::_dict_to_resource(const Dictionary &p_dict) {
	String path = p_dict["path"];
	return ResourceLoader::get_singleton()->load(path);
}

Dictionary SceneJSONConverter::_node_to_dict_recursive(Node *p_node, int &r_next_node_id) {
	Dictionary result;

	String node_name = p_node->get_name();
	if (node_name.is_empty()) {
		node_name = "_node_" + itoa(r_next_node_id);
		r_next_node_id++;
	}

	result["name"] = node_name;
	result["type"] = p_node->get_class_name();

	// Get exported properties
	List<PropertyInfo> props;
	p_node->get_property_list(&props);

	Array properties;
	for (const PropertyInfo &pi : props) {
		if (pi.usage & PROPERTY_USAGE_EXPORT) {
			Variant value = p_node->get(pi.name);
			Dictionary prop;
			prop["name"] = pi.name;
			prop["type"] = Variant::get_type_name(pi.type);
			// Convert to string representation
			prop["value"] = value;
			properties.append(prop);
		}
	}

	if (!properties.is_empty()) {
		result["properties"] = properties;
	}

	// Get script
	Ref<Script> script = p_node->get_script();
	if (script.is_valid()) {
		result["script"] = script->get_path();
	}

	// Process children
	Array children;
	for (int i = 0; i < p_node->get_child_count(); i++) {
		Node *child = p_node->get_child(i);
		children.append(_node_to_dict_recursive(child, r_next_node_id));
	}

	if (!children.is_empty()) {
		result["children"] = children;
	}

	return result;
}

Node *SceneJSONConverter::_dict_to_node_recursive(const Dictionary &p_dict, Node *p_parent) {
	String type = p_dict.has("type") ? p_dict["type"] : "Node";
	String name = p_dict.has("name") ? p_dict["name"] : "Node";

	Object *obj = ClassDB::create_instance(type);
	if (!obj) {
		return nullptr;
	}

	Node *node = Object::cast_to<Node>(obj);
	if (!node) {
		memdelete(obj);
		return nullptr;
	}

	node->set_name(name);

	// Set properties
	if (p_dict.has("properties")) {
		Array props = p_dict["properties"];
		for (int i = 0; i < props.size(); i++) {
			Dictionary prop = props[i];
			if (prop.has("name") && prop.has("value")) {
				node->set(prop["name"], prop["value"]);
			}
		}
	}

	// Set script
	if (p_dict.has("script")) {
		Ref<Script> script = ResourceLoader::get_singleton()->load(p_dict["script"]);
		if (script.is_valid()) {
			node->set_script(script);
		}
	}

	// Add children
	if (p_dict.has("children")) {
		Array children = p_dict["children"];
		for (int i = 0; i < children.size(); i++) {
			Node *child = _dict_to_node_recursive(children[i], node);
			if (child) {
				node->add_child(child);
			}
		}
	}

	return node;
}
