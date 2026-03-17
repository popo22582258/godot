# MCP Server 完整實作設計

## 日期
2026-03-17

## 目標
實作完整的 MCP Server，支援 WebSocket 和 stdio 傳輸，在編輯器載入後自動啟動。

---

## 1. 架構設計

### 1.1 系統架構

```
┌─────────────────────────────────────────────────────────┐
│                    Godot Engine                         │
├─────────────────────────────────────────────────────────┤
│  MCP Server Module                                      │
│  ┌─────────────────┐  ┌─────────────────────────────┐  │
│  │  Command Line   │  │  MCP Server                │  │
│  │  Parser        │  │  ┌───────────────────────┐  │  │
│  │  --mcp-port    │──│  │  WebSocket Server    │  │  │
│  │  --mcp-stdio   │  │  │  (port: 6550)        │  │  │
│  └─────────────────┘  │  └───────────────────────┘  │  │
│                       │  ┌───────────────────────┐  │  │
│                       │  │  JSON-RPC 2.0 Handler │  │  │
│                       │  └───────────────────────┘  │  │
│                       │  ┌───────────────────────┐  │  │
│                       │  │  Tool Registry        │  │  │
│                       │  │  - get_errors        │  │  │
│                       │  │  - get_scene_tree    │  │  │
│                       │  │  - run_project       │  │  │
│                       │  │  - ...               │  │  │
│                       │  └───────────────────────┘  │  │
│                       └─────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### 1.2 傳輸方式

| 方式 | 參數 | 預設值 |
|------|------|--------|
| WebSocket | `--mcp-port <port>` | 6550 |
| stdio | `--mcp-stdio` | - |

---

## 2. 命令列參數

在 `main/main.cpp` 新增：

```cpp
// 新增參數
"--mcp-port", "Start MCP Server on specified port (default: 6550)"
"--mcp-stdio", "Start MCP Server using stdio (for Claude Code)"
```

---

## 3. MCP Tools 清單

### 3.1 錯誤與診斷

| Tool | 描述 |
|------|------|
| `get_errors` | 取得當前所有錯誤 |
| `get_warnings` | 取得所有警告 |
| `get_script_errors` | 取得腳本錯誤 |
| `validate_and_fix` | 驗證並嘗試修復腳本 |
| `get_error_context` | 取得錯誤上下文 |

### 3.2 場景操作

| Tool | 描述 |
|------|------|
| `get_scene_tree` | 取得完整場景樹 |
| `get_node_properties` | 取得節點屬性 |
| `set_node_property` | 設定節點屬性 |
| `create_node` | 建立新節點 |
| `delete_node` | 刪除節點 |

### 3.3 腳本操作

| Tool | 描述 |
|------|------|
| `validate_script` | 驗證腳本語法 |
| `get_script_symbols` | 取得腳本符號（函數、變數） |
| `create_script` | 建立新腳本 |
| `edit_script` | 編輯腳本內容 |

### 3.4 專案操作

| Tool | 描述 |
|------|------|
| `get_project_structure` | 取得專案結構 |
| `get_project_settings` | 取得專案設定 |
| `save_scene` | 儲存場景 |
| `load_scene` | 載入場景 |

### 3.5 執行控制

| Tool | 描述 |
|------|------|
| `run_project` | 執行專案 |
| `stop_project` | 停止執行 |
| `run_scene` | 執行特定場景 |

### 3.6 編輯器操作

| Tool | 描述 |
|------|------|
| `get_editor_state` | 取得編輯器狀態 |
| `take_screenshot` | 擷取截圖 |
| `execute_menu_item` | 執行選單命令 |

---

## 4. 數據格式

### 4.1 JSON-RPC 2.0 請求格式

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "get_errors",
  "params": {}
}
```

### 4.2 回應格式

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "errors": [...],
    "statistics": {...}
  }
}
```

### 4.3 錯誤回應

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32600,
    "message": "Invalid Request",
    "data": "..."
  }
}
```

---

## 5. 自動啟動邏輯

在 `editor/plugins/mcp_server_plugin.cpp` 中：

```cpp
void _enter_tree() {
    // 檢查命令列參數
    if (CommandLine::get_singleton()->has_arg("--mcp-stdio")) {
        start_mcp_stdio();
    } else if (CommandLine::get_singleton()->has_arg("--mcp-port")) {
        int port = CommandLine::get_singleton()->get_arg_value("--mcp-port", 6550);
        start_mcp_server(port);
    }
}
```

---

## 6. 實現順序

1. **Phase 1**: 命令列參數解析
2. **Phase 2**: 基礎 MCP 框架（JSON-RPC）
3. **Phase 3**: WebSocket 伺服器
4. **Phase 4**: stdio 模式
5. **Phase 5**: Tool Registry 與基礎 Tools
6. **Phase 6**: 完整 Tools 實現

---

## 7. 測試策略

- 單元測試：每個 Tool 獨立測試
- 整合測試：WebSocket 連線測試
- 端到端測試：Claude Code 實際連線測試
