# Godot AI Fork - 操作手冊

## 目錄
1. [版本資訊](#1-版本資訊)
2. [安裝與啟動](#2-安裝與啟動)
3. [MCP Server 配置](#3-mcp-server-配置)
4. [MCP 工具清單](#4-mcp-工具清單)
5. [Orchestrator 視覺化腳本](#5-orchestrator-視覺化腳本)
6. [Claude Code 整合](#6-claude-code-整合)
7. [命令列操作](#7-命令列操作)
8. [常見問題排除](#8-常見問題排除)
9. [建構說明](#9-建構說明)

---

## 1. 版本資訊

| 項目 | 內容 |
|------|------|
| 引擎版本 | Godot 4.7.0-dev (AI Fork) |
| MCP Server | ✅ 已整合 |
| Orchestrator | v2.5.0 (需另行安裝編譯) |
| 建構日期 | 2026-03-18 |

### 新增功能
- MCP Server 自動啟動
- Orchestrator 視覺化腳本工具
- 命令列增強
- JSON 結構化輸出

---

## 2. 安裝與啟動

### 2.1 執行 Editor

編譯完成後，執行檔案位於：
```
F:\Godot_AI\godot\bin\godot.windows.editor.x86_64.exe
```

### 2.2 命令列模式

```bash
# Headless 模式（無 GUI）
godot --headless --path /project

# 指定專案路徑
godot --path "C:\MyGame"

# 啟用 MCP Stdio 模式
godot --headless --path /project --mcp-stdio

# 啟用 MCP WebSocket 模式
godot --headless --path /project --mcp-port 6550
```

---

## 3. MCP Server 配置

### 3.1 預設配置

| 項目 | 預設值 |
|------|--------|
| 監聽位址 | localhost |
| WebSocket 連接埠 | 6550 |
| 通訊協定 | JSON-RPC 2.0 |

### 3.2 啟動方式

**方式一：Stdio 模式（推薦用於 Claude Code 整合）**
```bash
godot --mcp-stdio
```

**方式二：WebSocket 模式**
```bash
godot --mcp-port 6550
```

**方式三：自動啟動**
MCP Server 會在 Editor 啟動時自動初始化，無需額外參數。

### 3.3 環境變數

| 變數名稱 | 說明 |
|---------|------|
| `GODOT_MCP_STDIO` | 設為 1 啟用 Stdio 模式 |
| `GODOT_MCP_PORT` | MCP Server 連接埠 |

---

## 4. MCP 工具清單

### 4.1 內建工具

| 工具名稱 | 功能說明 | 參數 |
|---------|---------|------|
| `get_errors` | 取得專案中的所有錯誤 | 無 |
| `get_warnings` | 取得專案中的所有警告 | 無 |
| `get_scene_tree` | 取得場景樹結構 | 無 |
| `get_project_structure` | 取得專案檔案結構 | `path` |
| `run_project` | 執行目前專案 | 無 |
| `stop_project` | 停止執行中的專案 | 無 |
| `validate_script` | 驗證 GDScript 語法 | `script_path` |
| `list_tools` | 列出所有可用的 MCP 工具 | 無 |

### 4.2 Orchestrator 工具

| 工具名稱 | 功能說明 | 參數 |
|---------|---------|------|
| `orchestrator_create_script` | 建立新的 Orchestrator 視覺腳本 | `path`, `name`, `base_type` |
| `orchestrator_list_scripts` | 列出專案中所有 Orchestrator 腳本 | `path` |
| `orchestrator_get_script_info` | 取得腳本詳細資訊 | `path` |
| `orchestrator_list_node_types` | 列出可用的節點類型 | 無 |
| `orchestrator_add_node` | 新增節點到腳本 | `path`, `node_type`, `node_name`, `node_id` |
| `orchestrator_connect_nodes` | 連接兩個節點 | `path`, `from_node`, `from_pin`, `to_node`, `to_pin` |
| `orchestrator_execute` | 載入腳本準備執行 | `path` |

### 4.3 工具呼叫範例

```python
# 取得專案結構
{
    "method": "tools/call",
    "params": {
        "name": "get_project_structure",
        "arguments": {"path": "res://"}
    }
}

# 建立 Orchestrator 腳本
{
    "method": "tools/call",
    "params": {
        "name": "orchestrator_create_script",
        "arguments": {
            "path": "res://scripts/player_logic.torch",
            "name": "PlayerLogic",
            "base_type": "Node"
        }
    }
}
```

---

## 5. Orchestrator 視覺化腳本

### 5.1 安裝 Orchestrator

Orchestrator 是 Godot 的視覺化腳本外掛，需要另行編譯安裝：

**步驟 1：編譯 DLL（需要 CMake）**
```bash
cd godot/modules/orchestrator
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

**步驟 2：複製 DLL 到專案**
```
你的專案/
└── addons/
    └── orchestrator/
        └── orchestrator.windows.64.release.dll
```

**步驟 3：在 Godot 中啟用**
- 開啟 `專案 > 專案設定 > 外掛程式`
- 啟用 `Orchestrator`

### 5.2 Orchestrator 腳本格式

Orchestrator 腳本使用 `.torch` 副檔名：

```text
[orchestration type="OScript" load_steps=2 format=3 uid="uid://abc123"]

[resource]
base_type = &"Node3D"
brief_description = "A camera controller"
description = "Camera logic for 3D games"
variables = Array[OScriptVariable]([])
functions = Array[OScriptFunction]([])
connections = Array[int]([])
nodes = Array[OScriptNode]([])
graphs = Array[OScriptGraph]([])
```

### 5.3 可用節點類型

| 類別 | 節點 |
|------|------|
| 事件節點 | OScriptNodeEvent, OScriptNodeEventMouseButton, OScriptNodeEventKey |
| 流程控制 | OScriptNodeBranch, OScriptNodeSequence, OScriptNodeSwitch, OScriptNodeFor |
| 函數節點 | OScriptNodeFunctionEntry, OScriptNodeCallScriptFunction, OScriptNodeCallMemberFunction |
| 變數節點 | OScriptNodeVariableGet, OScriptNodeVariableSet |
| 屬性節點 | OScriptNodePropertyGet, OScriptNodePropertySet |
| 運算節點 | OScriptNodeOperator, OScriptNodeComparison |
| 數學節點 | OScriptNodeMathConstant, OScriptNodeCompose, OScriptNodeDecompose |
| 場景節點 | OScriptNodeSceneNode, OScriptNodeInstantiate |

---

## 6. Claude Code 整合

### 6.1 MCP 設定

在 Claude Code 設定檔中加入：

```json
{
    "mcpServers": {
        "godot": {
            "command": "F:/Godot_AI/godot/bin/godot.windows.editor.x86_64.exe",
            "args": ["--headless", "--path", "${workspaceFolder}", "--mcp-stdio"]
        }
    }
}
```

### 6.2 常用操作範例

```python
# 1. 取得專案結構
tools.call("get_project_structure", {"path": "res://"})

# 2. 執行專案
tools.call("run_project", {})

# 3. 停止執行
tools.call("stop_project", {})

# 4. 驗證腳本
tools.call("validate_script", {"script_path": "res://player.gd"})

# 5. 取得所有錯誤
tools.call("get_errors", {})

# 6. 建立 Orchestrator 腳本
tools.call("orchestrator_create_script", {
    "path": "res://scripts/my_script.torch",
    "name": "MyScript",
    "base_type": "Node"
})

# 7. 列出 Orchestrator 腳本
tools.call("orchestrator_list_scripts", {"path": "res://scripts/"})
```

---

## 7. 命令列操作

### 7.1 基本參數

```bash
# 專案相關
--path <path>              # 指定專案路徑
--headless                  # 無頭模式

# MCP Server
--mcp-stdio                 # Stdio 模式
--mcp-port <port>           # WebSocket 模式

# 驗證
--validate-scripts          # 驗證所有 GDScript

# 匯出
--export-pack <path>        # 匯出為 .pck
```

### 7.2 結構化輸出

所有 CLI 輸出支援 JSON Lines 格式：

```bash
# 驗證腳本並輸出 JSON
godot --headless --path . --validate-scripts 2>&1 | jq '.'
```

---

## 8. 常見問題排除

### 8.1 MCP Server 無法啟動

**問題**: 編譯後 MCP Server 無法啟動

**解決方案**:
1. 確認 `modules/mcp_server/` 正確包含在編譯中
2. 檢查連接埠 6550 是否被佔用
3. 查看編輯器日誌中的錯誤訊息

### 8.2 Orchestrator DLL 載入失敗

**問題**: Orchestrator 無法載入

**解決方案**:
1. 確認 DLL 檔案放置正確 (`addons/orchestrator/`)
2. 檢查 Godot 版本與 Orchestrator 版本相容性
3. Orchestrator 需要 Godot 4.7.x

### 8.3 編譯錯誤

**問題**: 編譯時出現錯誤

**解決方案**:
```bash
# 安裝必要依賴
python misc\scripts\install_accesskit.py
python misc\scripts\install_d3d12_sdk_windows.py

# 或跳過可選依賴
scons accesskit=no d3d12=no
```

### 8.4 Claude Code 無法連線

**問題**: Claude Code 無法連接到 MCP Server

**解決方案**:
1. 確認 Godot 正在執行且 MCP Server 已啟動
2. 檢查防火牆設定
3. 確認使用正確的連線參數

---

## 9. 建構說明

### 9.1 編譯指令

```bash
# 完整版（含 MCP Server）
cd F:\Godot_AI\godot
scons -j$(nproc) platform=windows target=editor module_mcp_server_enabled=yes

# 停用可選功能（加速編譯）
scons -j$(nproc) platform=windows target=editor \
    module_mcp_server_enabled=yes \
    accesskit=no \
    d3d12=no
```

### 9.2 輸出位置

```
godot/bin/
├── godot.windows.editor.x86_64.exe        # 主執行檔
└── godot.windows.editor.x86_64.console.exe # 主控台版本
```

### 9.3 專案結構

```
Godot_AI/
├── godot/                    # Godot 引擎原始碼
│   ├── modules/
│   │   ├── mcp_server/      # MCP Server 模組
│   │   └── orchestrator/    # Orchestrator 原始碼
│   └── bin/                  # 編譯輸出
├── build_godot.bat           # 編譯腳本
└── Godot_AI_操作手冊.md      # 本手冊
```

---

## 附錄

### A. MCP 工具快速參考表

| 操作 | MCP 工具 | Claude Code 呼叫 |
|------|---------|-----------------|
| 取得錯誤 | `get_errors` | `tools.call("get_errors", {})` |
| 取得警告 | `get_warnings` | `tools.call("get_warnings", {})` |
| 執行專案 | `run_project` | `tools.call("run_project", {})` |
| 停止執行 | `stop_project` | `tools.call("stop_project", {})` |
| 驗證腳本 | `validate_script` | `tools.call("validate_script", {"script_path": "..."})` |
| 建立 Orchestrator 腳本 | `orchestrator_create_script` | `tools.call("orchestrator_create_script", {...})` |
| 列出 Orchestrator 腳本 | `orchestrator_list_scripts` | `tools.call("orchestrator_list_scripts", {...})` |
| 列出節點類型 | `orchestrator_list_node_types` | `tools.call("orchestrator_list_node_types", {})` |

### B. 相關連結

- Godot 官方文檔: https://docs.godotengine.org/
- MCP 協議規範: https://modelcontextprotocol.io/specification
- Orchestrator 文檔: https://docs.cratercrash.space/orchestrator
- Claude Code 文檔: https://code.claude.com/docs/

---

**手冊版本**: 2.0
**更新日期**: 2026-03-18
**適用引擎**: Godot 4.7.0-dev (AI Fork)
