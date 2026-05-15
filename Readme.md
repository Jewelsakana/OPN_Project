# 命令行多文本编辑器 — 插件化架构

基于 C++17 的命令行文本编辑器，支持纯文本和 XML 两种编辑模式，采用插件化架构设计，可方便地扩展新的编辑器类型（如数独游戏等）。

## 架构概览

采用 **四层插件化架构**，共 12 个独立 package：

```
Layer 3: 应用层       workspace                        ← WorkSpace 门面 + 四大协调器
Layer 2: 领域层       text / xml / log                 ← 具体编辑器 + 日志子系统
Layer 1: 核心抽象层   command / editor / service        ← 命令框架 + 编辑器接口 + 后端服务
Layer 0: 基础设施层   common / event / util            ← 共享结构 + 观察者 + 工具（零依赖）
插件层:              plugin/spell / plugin/stats        ← 拼写检查 + 编辑时长统计
```

**核心设计模式**：适配器、策略、注册表、命令、门面、观察者、模板方法、备忘录、装饰器、依赖注入

## 目录结构

```
├── src/
│   ├── main.cpp                     ← 程序入口
│   ├── command/       (6 文件)      ← 命令解析/创建/分发/撤销
│   ├── workspace/     (6 文件)      ← 工作区门面 + 文档管理 + 协调器
│   ├── service/       (5 文件)      ← 文件IO/目录/输出/序列化/编辑器工厂
│   ├── log/           (3 文件)      ← 日志记录器/管理器/命令
│   ├── event/         (2 文件)      ← 事件模型/观察者管理
│   ├── util/          (1 文件)      ← 字符串工具函数
│   ├── text/          (3 文件)      ← 文本编辑器 + 引擎 + 命令
│   ├── xml/           (4 文件)      ← XML 编辑器 + PugiXML 适配器
│   └── plugin/
│       ├── spell/     (4 文件)      ← 拼写检查（Mock + HTTP）
│       └── stats/     (2 文件)      ← 编辑时长统计
├── include/                         ← 与 src/ 完全镜像
│   ├── common/        (1 文件)      ← TextSegment, SpellCheckResult
│   ├── command/       (7 文件)
│   ├── workspace/     (7 文件)
│   ├── service/       (6 文件)
│   ├── log/           (3 文件)
│   ├── event/         (3 文件)
│   ├── editor/        (1 文件)
│   ├── util/          (2 文件)
│   ├── text/          (3 文件)
│   ├── xml/           (5 文件)
│   └── plugin/
│       ├── spell/     (6 文件)
│       └── stats/     (2 文件)
├── tests/             (14 文件)     ← 自动化测试套件
├── docs/                            ← 设计文档与总结
├── Makefile                         ← GNU Make 构建（自动发现源文件）
├── .editor_config                   ← 持久化配置文件
├── design_2.md                      ← 架构设计文档
└── Readme.md                        ← 本文件
```

## 构建与运行

### 环境要求

| 项目 | 说明 |
|------|------|
| 语言 | C++17 |
| 编译器 | GCC 8.1+ / MinGW-w64 |
| 构建工具 | GNU Make |
| 外部依赖 | WinHTTP（-lwinhttp，Windows SDK 自带） |

### 命令

```bash
mingw32-make all          # 构建项目
./text_editor             # 运行编辑器（交互式命令行）
mingw32-make test         # 运行全部 14 个测试套件
mingw32-make clean        # 清除编译产物
```

### 交互式命令示例

```
> init test.txt
> append "Hello World"
> show
> save
> init data.xml with-log
> append-child book book1 root "1984"
> insert-before chapter ch1 book1 "Chapter One"
> xml-tree
> editor-list
> spell-check
> editor-list tree
> exit
```

## 测试

14 个自动化测试套件，全部通过（0 编译警告）：

| 测试套件 | 内容 |
|---------|------|
| `test_commandparser` | 23 个命令的策略派发解析 |
| `test_commands` | 文本编辑命令 execute/undo |
| `test_documentmanager` | 文件打开/关闭/切换/修改状态 |
| `test_engine` | TextEngine 文本算法 |
| `test_log` | 日志记录命令 |
| `test_log_recovery` | 会话恢复与日志重放 |
| `test_loggermanager` | 日志生命周期管理 |
| `test_outputservice` | 各类格式化输出 |
| `test_workspace` | 工作区门面 + 备忘录 |
| `test_editor_factory` | 编辑器工厂 + XML 文档映射 |
| `test_xml_commands` | XML 编辑命令（35 项） |
| `test_xml_integration` | XML 集成（init/save/load/验证） |
| `test_spell_check` | 拼写检查（18 项） |
| `test_edit_duration` | 编辑时长统计（9 组） |

## 插件扩展

新增编辑器类型（如 `.sdk` 数独）只需创建文件，**零核心代码修改**：

```
src/plugin/sudoku/              include/plugin/sudoku/
  SudokuEditor.cpp                SudokuEditor.h
  SudokuCommands.cpp              SudokuCommands.h
  SudokuParser.cpp                SudokuParser.h
```

**五步集成**：

```cpp
// 1. 注册编辑器
REGISTER_EDITOR(".sdk", SudokuEditor)

// 2. 获取命令类型 ID
static auto CMD = CommandRegistry::registerEditorType("sudoku-set");

// 3. 注册解析器
REGISTER_PARSER(SudokuSetParser)

// 4. 注册命令工厂
REGISTER_PLUGIN_CMD(CMD, SudokuSetCommand)

// 5. 实现 Editor 子类（populateContext + supportsCommand + initialize + ...）
```

## 设计文档

- [架构设计文档](design_2.md) — 模块划分、依赖关系、接口设计、设计模式汇总
- [实现总结](docs/SUMMARY_1.2.md) — design-15 至 design-29 全部实现记录
- [设计文档](docs/) — design-1 至 design-29 分阶段设计提示词
