/**************************************************************************/
/*  mcp_protocol.cpp                                                      */
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

#include "mcp_protocol.h"

#include "core/object/class_db.h"
#include "core/io/json.h"

void MCPProtocol::_bind_methods() {
	ClassDB::bind_method(D_METHOD("parse_request", "json"), &MCPProtocol::parse_request);
	ClassDB::bind_method(D_METHOD("create_response", "id", "result"), &MCPProtocol::create_response);
	ClassDB::bind_method(D_METHOD("create_error_response", "id", "code", "message"), &MCPProtocol::create_error_response);
	ClassDB::bind_method(D_METHOD("get_method", "request"), &MCPProtocol::get_method);
	ClassDB::bind_method(D_METHOD("get_params", "request"), &MCPProtocol::get_params);
	ClassDB::bind_method(D_METHOD("get_id", "request"), &MCPProtocol::get_id);
	ClassDB::bind_method(D_METHOD("validate_request", "request"), &MCPProtocol::validate_request);
}

Dictionary MCPProtocol::parse_request(const String &p_json) {
	Dictionary request;
	Variant result = JSON::parse_string(p_json);
	if (result.get_type() != Variant::DICTIONARY) {
		return Dictionary();
	}

	return result;
}

String MCPProtocol::create_response(const Variant &p_id, const Variant &p_result) {
	Dictionary response;
	response["jsonrpc"] = "2.0";
	response["id"] = p_id;
	response["result"] = p_result;
	return JSON::stringify(response);
}

String MCPProtocol::create_error_response(const Variant &p_id, int p_code, const String &p_message) {
	Dictionary response;
	response["jsonrpc"] = "2.0";
	response["id"] = p_id;
	Dictionary error;
	error["code"] = p_code;
	error["message"] = p_message;
	response["error"] = error;
	return JSON::stringify(response);
}

String MCPProtocol::get_method(const Dictionary &p_request) const {
	return p_request.get("method", "");
}

Variant MCPProtocol::get_params(const Dictionary &p_request) const {
	return p_request.get("params", Variant());
}

Variant MCPProtocol::get_id(const Dictionary &p_request) const {
	return p_request.get("id", Variant());
}

bool MCPProtocol::validate_request(const Dictionary &p_request) const {
	// Must be JSON-RPC 2.0
	String jsonrpc = p_request.get("jsonrpc", "");
	if (jsonrpc != "2.0") {
		return false;
	}

	// Must have a method
	String method = p_request.get("method", "");
	if (method.is_empty()) {
		return false;
	}

	return true;
}
