# MCP Server 完整实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 实作完整的 MCP Server，支援 WebSocket 和 stdio 传输，在编辑器载入后自动启动。

**Architecture:** 使用 Godot 现有的网路类别 (StreamPeerTCP, TCPServer) 实现 WebSocket，使用标准 I/O 实现 stdio 模式。

**Tech Stack:** C++ (Godot Engine), JSON-RPC 2.0, TCP Socket

---

## Phase 1: 命令列参数解析

### Task 1: 在 main.cpp 新增 MCP 命令列参数

**Files:**
- Modify: `main/main.cpp:575-590` (在现有参数附近)

**Step 1: 新增帮助选项**

在 `--lsp-port` 参数附近新增：

```cpp
print_help_option("--mcp-port <port>", "Start MCP Server on specified port (default: 6550).\n", CLI_OPTION_AVAILABILITY_EDITOR);
print_help_option("--mcp-stdio", "Start MCP Server using stdio (for Claude Code).\n", CLI_OPTION_AVAILABILITY_EDITOR);
```

**Step 2: 新增参数解析逻辑**

在 main.cpp 中找到参数解析位置（约 1494 行附近），新增：

```cpp
} else if (arg == "--mcp-stdio") {
    // Mark for MCP stdio mode
    OS::get_singleton()->set_environment("GODOT_MCP_STDIO", "1");
} else if (arg == "--mcp-port") {
    // Get port from next argument
    int mcp_port = 6550;
    if (i + 1 < argc) {
        mcp_port = String::to_int(argv[i + 1]);
        i++;
    }
    OS::get_singleton()->set_environment("GODOT_MCP_PORT", itoa(mcp_port));
}
```

**Step 3: 编译测试**

Run: `cd F:/Godot_AI && build_godot.bat`

**Step 4: Commit**

```bash
cd F:/Godot_AI/godot
git add main/main.cpp
git commit -m "feat: Add --mcp-port and --mcp-stdio command line arguments"
```

---

## Phase 2: 基础 MCP 框架

### Task 2: 建立 MCP 协议处理器

**Files:**
- Create: `modules/mcp_server/mcp_protocol.h`
- Create: `modules/mcp_server/mcp_protocol.cpp`

**Step 1: 建立头文件**

```cpp
// modules/mcp_server/mcp_protocol.h
#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/string.h"

class MCPProtocol : public RefCounted {
    GDCLASS(MCPProtocol, RefCounted);

public:
    // Parse JSON-RPC request
    Dictionary parse_request(const String &p_json);

    // Create JSON-RPC response
    String create_response(const Dictionary &p_id, const Variant &p_result);

    // Create JSON-RPC error response
    String create_error_response(const Dictionary &p_id, int p_code, const String &p_message);

    // Get tool name from request
    String get_method(const Dictionary &p_request) const;

    // Get params from request
    Variant get_params(const Dictionary &p_request) const;

    // Get request ID
    Variant get_id(const Dictionary &p_request) const;

private:
    Dictionary parse_json(const String &p_json);
    String stringify(const Dictionary &p_dict);
};

#endif
```

**Step 2: 建立实现文件**

```cpp
// modules/mcp_server/mcp_protocol.cpp
#include "mcp_protocol.h"

Dictionary MCPProtocol::parse_request(const String &p_json) {
    Dictionary request;
    // Use JSON parser
    Error err;
    JSONParseResult parse_result = JSON::parse(p_json, err);
    if (err != OK) {
        return Dictionary();
    }
    return parse_result.result;
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

String MCPProtocol::create_response(const Dictionary &p_id, const Variant &p_result) {
    Dictionary response;
    response["jsonrpc"] = "2.0";
    response["id"] = p_id;
    response["result"] = p_result;
    return JSON::stringify(response);
}

String MCPProtocol::create_error_response(const Dictionary &p_id, int p_code, const String &p_message) {
    Dictionary response;
    response["jsonrpc"] = "2.0";
    response["id"] = p_id;
    Dictionary error;
    error["code"] = p_code;
    error["message"] = p_message;
    response["error"] = error;
    return JSON::stringify(response);
}
```

**Step 3: 在 register_types.cpp 注册**

```cpp
#include "mcp_protocol.h"
// 在 initialize_mcp_server_module 中新增
GDREGISTER_CLASS(MCPProtocol);
```

**Step 4: 编译测试**

**Step 5: Commit**

---

### Task 3: 建立 Tool Registry

**Files:**
- Create: `modules/mcp_server/mcp_tool_registry.h`
- Create: `modules/mcp_server/mcp_tool_registry.cpp`

**Step 1: 建立 Tool Registry 类**

```cpp
// mcp_tool_registry.h
#pragma once

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/string/string.h"

class MCPToolRegistry : public RefCounted {
    GDCLASS(MCPToolRegistry, RefCounted);

private:
    Dictionary tools;

public:
    void register_tool(const String &p_name, const Dictionary &p_definition);
    bool has_tool(const String &p_name) const;
    Dictionary get_tool_definition(const String &p_name) const;
    Array get_all_tool_names() const;
    Variant call_tool(const String &p_name, const Dictionary &p_params);

    // Built-in tools
    Dictionary get_errors(const Dictionary &p_params);
    Dictionary get_scene_tree(const Dictionary &p_params);
    Dictionary run_project(const Dictionary &p_params);
};

#endif
```

**Step 2: 注册基础 Tools**

在 `_bind_methods` 中注册所有工具。

**Step 3: 编译测试**

**Step 4: Commit**

---

## Phase 3: WebSocket 服务器

### Task 4: 实现 TCP 服务器

**Files:**
- Modify: `modules/mcp_server/mcp_server.h`
- Modify: `modules/mcp_server/mcp_server.cpp`

**Step 1: 扩展 MCPServer 类**

```cpp
class MCPServer : public RefCounted {
    GDCLASS(MCPServer, RefCounted);

private:
    int port = 6550;
    bool running = false;
    Ref<MCPToolRegistry> tool_registry;
    Ref<MCPProtocol> protocol;

    // TCP server (using Godot's built-in classes)
    // Note: Will use simple socket implementation

public:
    void set_port(int p_port) { port = p_port; }
    int get_port() const { return port; }

    bool start_server();
    void stop_server();
    bool is_running() const { return running; }

    void set_tool_registry(const Ref<MCPToolRegistry> &p_registry);

    Dictionary get_status() const;

    // Process a single request
    String process_request(const String &p_request);

    // Broadcast notification to all connected clients
    void broadcast_notification(const String &p_method, const Variant &p_params);
};
```

**Step 2: 实现服务器逻辑**

```cpp
bool MCPServer::start_server() {
    running = true;
    print_line("MCP Server started on port " + itoa(port));
    return true;
}

void MCPServer::stop_server() {
    running = false;
    print_line("MCP Server stopped");
}

String MCPServer::process_request(const String &p_request) {
    Dictionary request = protocol->parse_request(p_request);
    if (request.is_empty()) {
        return protocol->create_error_response(Dictionary(), -32700, "Parse error");
    }

    String method = protocol->get_method(request);
    Variant params = protocol->get_params(request);
    Variant id = protocol->get_id(request);

    // Call tool
    Variant result = tool_registry->call_tool(method, params);

    return protocol->create_response(id, result);
}
```

**Step 3: 编译测试**

**Step 4: Commit**

---

## Phase 4: stdio 模式

### Task 5: 实现 stdio 模式

**Files:**
- Create: `modules/mcp_server/mcp_stdio_handler.h`
- Create: `modules/mcp_server/mcp_stdio_handler.cpp`

**Step 1: 建立 Stdio Handler**

```cpp
// mcp_stdio_handler.h
#pragma once

#include "core/object/ref_counted.h"
#include "core/string/string.h"

class MCPStdioHandler : public RefCounted {
    GDCLASS(MCPStdioHandler, RefCounted);

private:
    Ref<MCPServer> mcp_server;
    bool running = false;

public:
    void set_server(const Ref<MCPServer> &p_server);
    void start();
    void stop();
    bool is_running() const { return running; }

    // Process single line from stdin
    void process_line(const String &p_line);
};

#endif
```

**Step 2: 实现循环读取**

```cpp
void MCPStdioHandler::start() {
    running = true;
    print_line("MCP Stdio Mode Ready");

    while (running) {
        // Read line from stdin (would need platform-specific implementation)
        // For now, we'll use Godot's main loop

        if (ConsoleServer::get_singleton()) {
            // Try to read from console
        }
    }
}
```

**Step 3: 编译测试**

**Step 4: Commit**

---

## Phase 5: 自动启动

### Task 6: 在 EditorPlugin 中自动启动

**Files:**
- Create: `modules/mcp_server/mcp_server_plugin.h`
- Create: `modules/mcp_server/mcp_server_plugin.cpp`

**Step 1: 建立 EditorPlugin**

```cpp
// mcp_server_plugin.h
#pragma once

#include "editor/plugins/editor_plugin.h"

class MCPServerPlugin : public EditorPlugin {
    GDCLASS(MCPServerPlugin, EditorPlugin);

private:
    Ref<MCPServer> mcp_server;
    Ref<MCPStdioHandler> stdio_handler;

public:
    MCPServerPlugin();
    ~MCPServerPlugin();

    void _enter_tree() override;
    void _exit_tree() override;

    String get_plugin_name() const override { return "MCPServer"; }
};

#endif
```

**Step 2: 实现自动启动逻辑**

```cpp
void MCPServerPlugin::_enter_tree() {
    // Check for MCP command line arguments
    bool use_stdio = OS::get_singleton()->has_environment("GODOT_MCP_STDIO");
    String port_str = OS::get_singleton()->get_environment("GODOT_MCP_PORT");

    if (use_stdio) {
        // Start in stdio mode
        mcp_server.instantiate();
        stdio_handler.instantiate();
        stdio_handler->set_server(mcp_server);
        stdio_handler->start();
        print_line("MCP Server started in stdio mode");
    } else if (!port_str.is_empty()) {
        // Start in WebSocket mode
        int port = port_str.to_int();
        mcp_server.instantiate();
        mcp_server->set_port(port);
        mcp_server->start_server();
        print_line("MCP Server started on port " + itoa(port));
    }
}

void MCPServerPlugin::_exit_tree() {
    if (mcp_server.is_valid()) {
        mcp_server->stop_server();
    }
    if (stdio_handler.is_valid()) {
        stdio_handler->stop();
    }
}
```

**Step 3: 注册 Plugin**

在 `register_types.cpp` 中新增注册。

**Step 4: 编译测试**

**Step 5: Commit**

---

## Phase 6: 完整 Tools 实现

### Task 7: 实现所有 MCP Tools

**Files:**
- Modify: `modules/mcp_server/mcp_tool_registry.cpp`

实现以下 Tools：

1. **get_errors** - 调用 DebugScanner
2. **get_warnings** - 调用 DebugScanner
3. **get_scene_tree** - 获取场景树
4. **run_project** - 执行项目
5. **stop_project** - 停止执行
6. **get_project_structure** - 获取项目结构

---

## 执行选项

**Plan complete and saved to `docs/plans/2026-03-17-mcp-server-full-implementation-design.md`. Two execution options:**

**1. Subagent-Driven (this session)** - I dispatch fresh subagent per task, review between tasks, fast iteration

**2. Parallel Session (separate)** - Open new session with executing-plans, batch execution with checkpoints

**Which approach?**
