# 命令行文本编辑器 — 实现总结 v1.2

本文件合并了 `重构总结.md`（design-15~18）、`docs/SUMMARY_2.md`（XML 编辑器框架）和 `SUMMARY_2.md`（XML 命令及后续阶段）中的全部实现内容，按设计阶段时间线组织。

---

# 第一部分：文本编辑器重构（design-15 ~ design-18）

## design-15 重构

按照 `design-15.md` 的设计要求，对文本编辑器项目进行了全面重构，涵盖 16 个源文件。重构遵循 SOLID 设计原则，消除了识别的主要代码坏味道。所有 9 个测试套件均通过。

### 新建文件（11 个）

| 文件 | 职责 |
|------|------|
| `include/StringUtils.h` + `src/StringUtils.cpp` | 字符串工具函数（trim, splitString, startsWith, toLower），消除 CommandParser 与 FileSystemService 之间的重复代码 |
| `include/CommandParserStrategy.h` + `src/CommandParserStrategy.cpp` | 策略模式：18 个命令各自独立的解析策略类，替代原来的 if-else 链 |
| `include/CommandFactory.h` + `src/CommandFactory.cpp` | 工厂模式：集中创建 Command 对象，消除 CommandController 中 ~130 行的 switch 语句 |
| `include/ConfigSerializer.h` + `src/ConfigSerializer.cpp` | 配置序列化/反序列化，从 FileSystemService 中分离 |
| `include/DirectoryService.h` + `src/DirectoryService.cpp` | 目录树遍历逻辑，从 FileSystemService 中分离 |
| `include/ConfigManager.h` + `src/ConfigManager.cpp` | 配置管理编排，从 WorkSpace 中分离 |
| `include/ObserverManager.h` + `src/ObserverManager.cpp` | 观察者管理基类，WorkSpace 继承自此基类 |

### 应用的设计模式

1. **策略模式** — `ICommandParserStrategy` 接口 + 18 个具体策略类，替代 CommandParser 中的 if-else 链
2. **工厂模式** — `CommandFactory` 集中管理命令对象创建，消除 Controller 中的并行 switch
3. **模板方法模式** — `TextCommand::recordDeletedText()` 统一预删除记录逻辑
4. **观察者模式** — `ObserverManager` 基类封装观察者管理，WorkSpace 继承复用
5. **备忘录模式** — `WorkspaceMemento` 配合 ConfigManager/ConfigSerializer 实现工作区状态持久化

### 消除的代码坏味道

- 重复代码（StringUtils 提取、recordDeletedText 模板方法）
- 过长方法（CommandParser::parse 从 ~140 行缩减到 ~20 行）
- 过大的类（FileSystemService 拆分 3 个类、WorkSpace 拆分 4 个类）
- 并行 switch 语句（策略模式 + 工厂模式消除）
- 数据泥团（Position 值对象封装 row/col）

---

## design-16 重构

按照 `design-16.md` 的要求，对仍需重构的 6 个问题进行深度重构。所有 9 个测试套件均通过，编译无警告。

### 1. CommandFactory 注册表模式（消除 switch）

**修改文件：**
- `include/CommandFactory.h` — 新增 `EditorCommandCreator` / `WorkSpaceCommandCreator` 函数类型，新增 `registerEditorCreator()` / `registerWorkSpaceCreator()` 静态注册方法
- `src/CommandFactory.cpp` — 用 `std::unordered_map` 注册表替代两个 switch 语句（原第 46 行 5-case 编辑器 switch，原第 81 行 13-case 工作区 switch）
- `src/TextCommands.cpp` — 5 个编辑器命令自注册（Append, Insert, Delete, Replace, Show）
- `src/WorkSpaceCommand.cpp` — 10 个工作区命令自注册（Load, Save, Init, Close, Edit, EditorList, DirTree, Undo, Redo, Exit）
- `src/LogCommand.cpp` — 3 个日志命令自注册（Logon, Logoff, Logshow）

**效果：** 新增命令只需在对应的 .cpp 文件中添加注册 lambda，无需修改 CommandFactory。

### 2. Editor 接口增强（消除 dynamic_cast）

**修改文件：**
- `include/Editor.h` — 新增 `undo()`, `redo()`, `canUndo()`, `canRedo()` 虚方法，提供默认空实现
- `include/TextEditor.h` — 对应方法标记 `override`
- `src/WorkSpaceCommand.cpp` — Undo/Redo 不再需要 `dynamic_pointer_cast<TextEditor>`，直接通过 Editor 接口调用

**效果：** Undo/Redo 命令不再依赖具体 TextEditor 类型，遵循里氏替换原则（LSP）。

### 3. 空指针检查集中化

**修改文件：**
- `include/WorkSpaceCommand.h` — 新增 `protected` 方法 `checkWorkSpace()` 和 `getActiveEditorOrThrow()`
- `src/WorkSpaceCommand.cpp` — ~20 处重复的空指针判断替换为 `checkWorkSpace()` 调用
- `src/LogCommand.cpp` — 5 处空指针判断替换为 `checkWorkSpace()` 调用

**效果：** 消除了 ~30 行重复的 null check 模板代码，统一错误消息。

### 4. WorkSpace 职责拆分（四大协调器 + 门面）

**新建文件：**
| 文件 | 职责 |
|------|------|
| `include/FileCoordinator.h` + `src/FileCoordinator.cpp` | 文件加载/保存/初始化/关闭的业务逻辑 |
| `include/EditorCoordinator.h` + `src/EditorCoordinator.cpp` | 编辑器切换和查询（委托给 DocumentManager） |
| `include/LogCoordinator.h` + `src/LogCoordinator.cpp` | 日志启停和查询 |
| `include/ConfigCoordinator.h` + `src/ConfigCoordinator.cpp` | 配置保存/恢复 |

**架构：**
```
WorkSpace (Facade)
  ├── EditorCoordinator → DocumentManager
  ├── FileCoordinator    → FileSystemService, DocumentManager, LoggerManager
  ├── LogCoordinator     → LoggerManager, OutputService
  └── ConfigCoordinator  → ConfigManager
```

### 5. 删除 FileIOService 别名

移除 `using FileIOService = FileSystemService;`（为从未发生的拆分预留的投机代码）。

### 6. 消除 ShowCommand const_cast

ShowCommand 改为直接继承 `Command`（而非 `TextCommand`），存储 `const std::vector<std::string>&`，移除类型不安全的 `const_cast`。

---

## design-17 重构

按照 `design-17.md` 的要求，针对 5 个代码坏味道进行了重构。全部 9 个测试套件通过，编译无警告。

### 1. 统一自注册 Lambda（消除重复）

**修改文件：**
- `include/CommandFactory.h` — 新增 8 个自注册宏：
  - 工作区命令：`REGISTER_WS_CMD_NOARGS`、`REGISTER_WS_CMD_FILENAME`、`REGISTER_WS_CMD_REQ_FILENAME`、`REGISTER_WS_CMD_PATH`、`REGISTER_WS_CMD_TARGET`、`REGISTER_WS_CMD_INIT`
  - 编辑器命令：`REGISTER_EDITOR_CMD_GUARDED`（带守卫条件的通用宏）、`REGISTER_EDITOR_CMD_SHOW`（ShowCommand 专用）
- `src/WorkSpaceCommand.cpp` — 10 段匿名 namespace 注册代码（~87 行）缩减为 10 行宏调用
- `src/LogCommand.cpp` — 3 段匿名 namespace 注册代码（~24 行）缩减为 3 行宏调用
- `src/TextCommands.cpp` — 5 段匿名 namespace 注册代码（~51 行）缩减为 5 行宏调用

**效果：** 全部 18 个命令的自注册代码从 ~162 行缩减到 ~18 行。

### 2. 消除双重 setWorkSpace()

`CommandController::executeCommand()` 中移除重复的 `wsCommand->setWorkSpace(workspace_)` 调用。

### 3. 拆分过长方法

- `ConfigSerializer::loadConfig()` 提取 `parseConfigLine()` 私有方法
- `WorkSpace::restoreFromMemento()` 拆分为 `restoreOpenFiles()`、`restoreModifiedStates()`、`restoreLogState()` 三个私有方法
- `ExitCommand::execute()` 提取 `ensureNoUnsavedFiles()` 和 `trySaveConfig()` 两个私有方法

### 4. 修正 WorkSpace 接口的抽象层次

- `Event` 新增 `static currentTimestampString()` 方法，封装时间格式化逻辑
- `LogCoordinator::showLog()` 增加 `activeFileName` 参数，将业务逻辑从 WorkSpace 下放
- WorkSpace 门面不再包含底层格式化逻辑和业务判断逻辑

### 5. 数据成员封装

`TextCommand::lines` 和 `TextCommand::textEngine` 从 `protected` 改为 `private`，新增 `protected` getter `lines()` 和 `textEngine()`。

---

## design-18 重构

按照 `design-18.md` 的要求，针对 6 个代码坏味道进行了重构。全部 9 个测试套件通过，编译无警告。

### 1. 抽取公共文件系统头文件（消除代码重复）

新建 `include/FilesystemCompat.h` — 统一的条件编译块，定义 `HAS_FILESYSTEM` 宏。消除了 `DirectoryService.cpp` 和 `WorkSpaceCommand.cpp` 中 ~21 行重复的条件编译代码。

### 2. 抽取重复排序 Lambda

`DirectoryService.cpp` 中新增匿名 namespace 中的 `compareDirectoryEntries()` 自由函数，替代 `buildDirectoryTree()` 和 `buildDirectoryTreeStructure()` 中完全相同的 8 行排序 lambda。

### 3. 消除无所事事的类（ConfigCoordinator）

删除 `ConfigCoordinator`（仅 14 行透传调用），调用链从 `WorkSpace → ConfigCoordinator → ConfigManager → ConfigSerializer` 简化为 `WorkSpace → ConfigManager → ConfigSerializer`。

### 4. 删除遗留声明和未使用代码

移除无人调用的 `getName()` 和 `reset()` 虚方法（Model 及其全部子类），移除 `DocumentManager` 中未使用的 `getOpenFileCount()` 和 `getModifiedFileCount()`。

### 5. 清理无用注释

移除空构造函数中的冗余注释、解释代码行为的注释、分隔注释等。

### 6. 命令统一使用 WorkSpace 输出接口（消除类间亲密）

WorkSpace 新增 4 个输出门面方法：`outputError()`、`outputLine()`、`outputList()`、`outputTree()`。所有命令类和服务类不再直接持有/访问 `OutputService` 引用，改为通过 WorkSpace 门面接口输出。

---

# 第二部分：XML 编辑器框架（design-19）

## 项目概述

基于 `design-19.md` 的设计要求，在现有命令行文本编辑器框架基础上，扩展实现了 XML 编辑器子系统。包括 XML 文档解析、DOM 树构建、ID 到节点的快速映射、XML 编辑引擎、XML 命令框架以及基于注册表模式的 EditorFactory。

## 完成的核心模块

### 1. XML 解析库集成

引入轻量级 C++ XML 解析库 **PugiXML**（v1.14），作为底层 XML 解析引擎。PugiXML 以单头文件+源文件形式分发，支持 DOM 遍历、属性访问和文件读写。

### 2. IXmlDocument（抽象接口）

XML 文档的抽象接口，定义统一的 XML 文档操作协议。不依赖任何具体 XML 解析库，是适配器模式中的目标接口（Target）。

- `loadFromString()` / `loadFromFile()`：加载 XML 文档
- `saveToString()` / `saveToFile()`：保存 XML 文档
- `collectIds()`：收集所有节点 ID 并验证唯一性
- `hasNodeWithId()` / `getNodeName()` / `getNodeValue()` / `getNodeAttribute()`：字符串接口查询节点
- `getAllIds()`：获取所有已注册的 ID 列表
- 异常体系：`XmlDocumentException`（基类）、`XmlParseException`（解析错误）、`DuplicateIdException`（重复ID）、`MissingIdException`（缺少ID属性）

**设计意图**：上层模块（XmlEditor、XmlCommand）只依赖此接口。替换 XML 解析库时只需编写新的 `IXmlDocument` 实现类，上层代码无需修改。

### 3. XmlDocumentWrapper（适配器实现）

`IXmlDocument` 的具体实现，使用适配器模式封装 PugiXML 的 `xml_document`。ID 映射（`idToNodeMap_`）作为私有实现细节内部管理，对外只通过 `IXmlDocument` 的字符串接口访问。

- 继承 `IXmlDocument`，实现所有纯虚方法
- 内部使用 `pugi::xml_document` 和 `unordered_map<string, pugi::xml_node>`
- 不对外暴露任何 PugiXML 类型
- `getPugiDocument()` 和 `root()` 为 private，外部无法访问 pugi 类型

### 4. XmlEditor（XML 编辑器）

派生自 `Editor` 接口，解析 XML 文件为 DOM 树形结构，维护 ID 到节点的快速映射。

- 继承 `Editor` 接口：实现 `executeCommand()`、`undo()`、`redo()`、`canUndo()`、`canRedo()`
- `loadFromFile()` / `loadFromString()`：加载 XML 并自动构建 ID 映射
- `saveToFile()`：保存 XML 到文件，自动清除修改标记
- `findNodeById()`：通过 ID 快速查找节点（O(1)哈希查找）
- `rebuildIdMapping()`：文档修改后重建 ID 映射
- `isModified()` / `setModified()` 追踪文档是否被修改
- `CommandManager` 作为私有组件，提供独立的 Undo/Redo 栈

### 5. EditorFactory（编辑器工厂 + 注册表模式）

使用注册表模式（Registry Pattern），根据文件后缀返回对应的 `unique_ptr<Editor>`。

- `createEditor(fileExtension)`：根据后缀创建对应的 Editor 实例
- `registerEditor(extension, creator)`：注册编辑器创建器
- 注册表使用函数内静态对象（Meyer's Singleton），避免静态初始化顺序问题
- **自注册宏** `REGISTER_EDITOR(EXT, CLASS)`：在 Editor 子类的 .cpp 文件中使用，实现启动时自动注册

### 6. ID 唯一性验证

加载 XML 时自动执行 ID 唯一性验证。所有 XML 元素必须有唯一的 `id` 属性。缺少 ID 属性时抛出 `MissingIdException`，发现重复 ID 时抛出 `DuplicateIdException`。

## 设计模式应用

| 设计模式 | 应用位置 | 说明 |
|---------|---------|------|
| 适配器模式 | IXmlDocument + XmlDocumentWrapper | 抽象接口定义目标协议，具体适配器封装 PugiXML |
| 工厂模式 | EditorFactory | 根据文件后缀创建对应 Editor |
| 注册表模式 | EditorFactory + REGISTER_EDITOR | 自注册机制，支持插件化扩展 |
| 命令模式 | XMLCommand | XML 编辑操作封装为命令对象 |
| 模板方法模式 | Model 基类 | safeExecute() 提供统一异常处理框架 |

## 架构图

```
Editor (接口)
  ├── TextEditor (.txt)
  └── XmlEditor (.xml)
        ├── IXmlDocument (抽象接口) ← 依赖倒置
        │     └── XmlDocumentWrapper (适配器) → pugi::xml_document
        ├── CommandManager (Undo/Redo管理)
        └── (ID映射逻辑封装在XmlDocumentWrapper内部)

EditorFactory (注册表模式)
  └── registry: map<extension, creator>
        ├── ".txt" → TextEditor
        ├── ".xml" → XmlEditor
        └── ... (支持插件扩展)

Command (接口)
  ├── TextCommand → InsertCommand, DeleteCommand, ...
  └── XMLCommand → InsertBeforeCommand, AppendChildCommand, ...
```

## 构建和运行

```bash
mingw32-make all        # 构建项目
mingw32-make test       # 运行所有测试
mingw32-make clean      # 清理编译文件
```

## 设计19 新增文件清单

### 头文件（include/）
- `pugixml.hpp` — PugiXML 库头文件
- `pugiconfig.hpp` — PugiXML 编译配置
- `IXmlDocument.h` — XML 文档抽象接口（适配器模式目标接口）
- `XmlDocumentWrapper.h` — PugiXML 适配器实现
- `XmlEditor.h` — XML 编辑器接口
- `XMLCommand.h` — XML 命令基类接口
- `EditorFactory.h` — 编辑器工厂接口（含自注册宏）

### 源文件（src/）
- `pugixml.cpp` — PugiXML 库实现
- `XmlDocumentWrapper.cpp` — XML 文档适配器实现
- `XmlEditor.cpp` — XML 编辑器实现（含 .xml 自注册）
- `XMLCommand.cpp` — XML 命令基类实现
- `EditorFactory.cpp` — 编辑器工厂实现

### 修改的现有文件
- `src/TextEditor.cpp` — 添加 `#include "EditorFactory.h"` 和 `.txt` 后缀自注册

---

# 第三部分：XML 编辑命令实现（design-20 ~ design-22）

## 第一阶段：基本 XML 编辑命令

### 1. `insert-before` - 在目标元素前插入新元素

- **格式**: `insert-before <tagName> <newId> <targetId> ["text"]`
- **功能**: 在目标元素前（同级）插入一个带指定 ID 和可选文本的新元素
- **异常处理**: newId 已存在 / targetId 不存在 / 尝试在根元素前插入
- **Undo**: 删除插入的新元素

### 2. `append-child` - 在父元素内追加子元素

- **格式**: `append-child <tagName> <newId> <parentId> ["text"]`
- **功能**: 在父元素内追加一个带指定 ID 和可选文本的子元素（作为最后一个子元素）
- **异常处理**: parentId 无效 / newId 重复
- **Undo**: 删除追加的子元素

### 3. `edit-id` - 修改元素 ID

- **格式**: `edit-id <oldId> <newId>`
- **功能**: 修改某个元素的 ID 属性
- **异常处理**: oldId 不存在 / newId 已被占用 / 尝试修改根元素 ID
- **Undo**: 将 ID 改回原值

### 4. `edit-text` - 修改元素文本

- **格式**: `edit-text <elementId> ["text"]`
- **功能**: 修改某个元素的文本内容，省略或空字符串则清空原内容
- **异常处理**: elementId 不存在
- **Undo**: 恢复修改前的文本

### 5. `delete` - 删除元素（XML 模式）

- **格式**: `delete <elementId>`
- **功能**: 删除指定 ID 的元素及其所有子元素
- **注意**: 当第二个参数不含冒号时自动识别为 XML 删除模式
- **异常处理**: elementId 不存在 / 尝试删除根元素
- **Undo**: 重新插入被删除的元素（保存 tagName、parentId、文本、位置等快照）

### 第一阶段设计要点

1. **适配器模式**: 所有 XML 操作通过 `IXmlDocument` 接口，不依赖具体 XML 库
2. **命令模式**: 每个 XML 操作封装为独立的 Command 类，支持 execute/undo
3. **策略模式**: 命令解析使用策略模式，每个 XML 命令有独立的 Parser
4. **注册表模式**: 使用自注册宏 `REGISTER_XML_CMD` 消除工厂 switch-case
5. **ID 映射同步**: 每次增删改操作后通过 `collectIds()` 重建完整的 ID 映射表
6. **简化架构**: `XMLEngine` 纯委托层已删除，`XMLCommand` 直接绑定 `IXmlDocument*`（与 `TextCommand` 绑定 `TextEngine*` 的模式一致）

---

## 第二阶段：xml-tree + init/save/load 兼容 + 命令验证

### 6. `xml-tree` - 显示 XML 树形结构

- **格式**: `xml-tree [file]`
- **功能**: 以树形结构打印 XML 文件内容，展示元素的层级关系、属性和文本内容
- **只读**: 不进入撤销栈
- **输出示例**:
  ```
  bookstore [id="root"]
  ├── book [id="book1", category="COOKING"]
  │   └── title [id="title1", lang="en"]
  │       └── "Everyday Italian"
  ```
- **实现**: XmlTreeCommand → OutputService::outputXmlTree → IXmlDocument 遍历方法

### 7. `init` 命令 XML 支持

- **自动类型识别**: 通过文件扩展名（.xml→XmlEditor, .txt→TextEditor）自动创建对应编辑器
- **XML 初始内容**: `<?xml version="1.0" encoding="UTF-8"?>\n<root id="root">\n</root>\n`
- **with-log 支持**: 在 XML 声明前添加 `# log` 注释行
- **FileCoordinator 重构**: 编辑器工厂从 `function<shared_ptr<TextEditor>()>` 改为 `function<shared_ptr<Editor>(const string& extension)>`

### 8. `save` 和 `load` 命令 XML 支持

- **load**: 检测 .xml 扩展名 → 创建 XmlEditor → 调用 XmlEditor::loadFromFile
- **save**: 检测编辑器类型 → 通过 Editor 多态方法序列化后写入文件
- **FileCoordinator**: 通过 Editor 多态接口消除 dynamic_cast 分支

### 9. 命令类型验证

- **Editor::supportsCommand(EditorCommandType)**: 新增纯虚方法
- **TextEditor**: 支持 Append/Insert/Delete/Replace/Show，拒绝所有 XML 命令
- **XmlEditor**: 支持 InsertBefore/AppendChild/EditId/EditText_/XmlDelete/XmlTree，拒绝所有文本命令
- **CommandFactory**: 在 createFromParsed 中调用 supportsCommand 验证，不匹配则抛出异常

### 10. Workspace 编辑器创建

- **EditorFactory 集成**: WorkSpace 通过 EditorFactory::createEditor(extension) 创建对应编辑器
- **TextEngine 注入**: 仅对 TextEditor 注入 TextEngine
- **openFile / restoreOpenFiles**: 根据文件扩展名创建编辑器

---

## 第三阶段：重构（设计22）

对 OutputService、FileCoordinator、CommandFactory、XmlDocumentWrapper 四个类进行重构。

### 11. OutputService 重构

- 提取 `printXmlSubTree(IXmlDocument&, id, prefix, indent)` 私有递归方法
- 提取 `formatXmlAttrs(IXmlDocument&, id)` 静态私有方法格式化属性字符串
- `outputXmlTree` 入口缩减为 ~15 行

### 12. FileCoordinator 重构（多态替代 dynamic_cast）

- 在 `Editor` 接口新增 `loadFromData(string)`, `saveToData() -> string`, `initContent(bool withLog)` 多态方法
- `TextEditor`: 文本行与字符串之间的转换
- `XmlEditor`: 委托给 XmlDocumentWrapper 的字符串方法
- `FileCoordinator` 消除所有 `dynamic_cast<XmlEditor*>` / `dynamic_cast<TextEditor*>` 分支

### 13. CommandFactory 重构

- `EditorCommandContext::lines` 从引用改为指针，消除 dummyLines 静态变量
- 新增 `buildEditorContext(Editor*, WorkSpace*)` 函数通过 dynamic_cast 确定活跃编辑器类型
- 移除 `isXmlCommandType()` 函数（Editor::supportsCommand 已在外部完成验证）

### 14. XmlDocumentWrapper 重构

- 提取 `createElement(parent, tagName, id, text, insertBefore, targetNode)` 私有方法统一创建元素逻辑
- 提取 `registerNodeId(node)` 私有方法统一 ID 验证 + 去重 + 插入映射逻辑
- `getPugiDocument()` 和 `root()` 改为 private，pugi 类型不再对外暴露

### 15. TextEditor 序列化委托给 TextEngine

- TextEngine 新增 `stringToLines(content)` 和 `linesToString(lines)` 方法
- TextEditor::loadFromData/saveToData 委托给 TextEngine

### 16. XML 文件日志记录支持

移除 FileCoordinator 中阻止 XML 文件启用日志的 `!isXml` 守卫，pugixml 可容忍 `# log` 文本在 XML 声明之前。

---

# 第四部分：统计时模块（design-23）

### 17. 编辑时长统计器 (EditDurationTracker)

- **模式**: 观察者模式——EditDurationTracker 实现 Observe 接口，通过 `update(Event&)` 接收命令事件检测文件切换
- **计时规则**:
  - 开始计时：文件成为活动文件时
  - 停止计时：切换到其他文件时（记录前一个文件的累计时长）
  - 累计时长：每次切换时累加，同一会话中反复切换会累计
  - 重置时长：文件关闭时自动清除记录
- **实现要点**:
  - 使用 `std::chrono::steady_clock` 计时，精度为秒
  - `update()` 通过事件目标文件名检测文件切换
  - 所有方法 try-catch 保护，失败仅输出警告不抛异常

### 18. 时长格式化 (StringUtils::formatDuration)

- **新增方法**: `StringUtils::formatDuration(int totalSeconds) → string`
- **显示规则**: < 1分钟→"X秒" / 1-59分钟→"X分钟" / 1-23小时→"X小时Y分钟" / ≥ 24小时→"X天Y小时"

### 19. editor-list 命令增强

- **格式**: `editor-list [tree]`
  - 无参数：列表形式显示（带时长装饰）
  - `tree`：树形格式显示
- **装饰器模式**: 通过 `EditDurationDecorator` 为每个文件名附加时长信息

### 20. 装饰器解耦 (EditDurationDecorator)

- **职责分离**:
  - `StringUtils::formatDuration` — 时长格式化（秒→字符串）
  - `EditDurationTracker` — 时长数据查询
  - `EditDurationDecorator` — 装饰逻辑组合（文件名+时长+状态标记）
  - `EditorListCommand` — 命令编排（调用装饰器 + 输出）

### 21. 观察者模式修正

close 事件完全由 CommandController → Event → Observer 通知链路触发，`WorkSpace::closeFile` 不再直接调用 tracker 方法。

---

# 第五部分：拼写检查模块（design-24 ~ design-27）

## 拼写检查命令 `spell-check`

- **格式**: `spell-check [file]`
  - 不指定参数：检查当前活动文件
  - `file`：检查指定文本文件
- **功能**: 检查文本文件和 XML 文件中的拼写错误
- **只读**: isReadOnly() 返回 true，不进入撤销栈

### 适配器模式设计

- **目标接口**: `ISpellChecker` — `checkText(TextSegment) → vector<SpellCheckResult>`
- **Mock 适配器**: `MockSpellChecker` — 27 条内置常见拼写错误映射表
- **HTTP 适配器**: `HttpSpellCheckerAdapter` — 调用 LanguageTool API (https://api.languagetool.org/v2/check)
- **依赖注入**: 通过 `WorkSpace::setSpellChecker()` 注入不同实现

### 数据结构

- **TextSegment**: 待检查文本片段，包含 text / line / column / elementId
- **SpellCheckResult**: 统一结果结构体，包含 line / column / elementId / original / suggestions

### Editor 多态

- **Editor 基类**: 新增 `getTextsToCheck()` 虚函数
- **TextEditor**: 委托给 TextEngine，每行创建一个 TextSegment
- **XmlEditor**: 委托给 IXmlDocument，遍历提取文本节点

### 输出格式

- 文本文件: `第1行，第5列: "recieve" -> 建议: receive`
- XML 文件: `元素 title1: "Itallian" -> 建议: Italian`
- 无错误: `拼写检查结果: 未发现拼写错误`

## 职责迁移到引擎类

### getTextsToCheck() 职责迁移

- **TextEngine**: 新增 `getTextsToCheck(lines)` — 遍历行数组，跳过空行
- **IXmlDocument**: 新增纯虚方法 `getTextsToCheck()`
- **XmlDocumentWrapper**: 实现 `getTextsToCheck()` — 遍历 `idToNodeMap_`

### initContent() 职责迁移

- **TextEngine**: 新增 `initContent(bool withLog)` → 返回 `{"#log"}` 或 `{""}`
- **IXmlDocument**: 新增纯虚方法 `initContent(bool withLog)`
- **XmlDocumentWrapper**: 实现 `initContent()` — 构建 XML 字符串 → loadFromString → collectIds

## Bug 修复

### 修复双重错误提示 Bug

`CommandController::executeCommand()` 的 try-catch 输出错误后 re-throw，外层 `parseAndExecuteCommand()` 再次 catch 并输出同一错误。修复：移除 `executeCommand()` 中的 try-catch，让异常传播到 `parseAndExecuteCommand()` 统一处理。

## HttpSpellCheckerAdapter 完整实现

- **HTTP 客户端**: 使用 WinHTTP API（`<windows.h>`, `<winhttp.h>`）发送 HTTPS POST 请求
- **API**: 调用 `https://api.languagetool.org/v2/check`，参数 `language=en-US&text=<url-encoded>`
- **URL 编码**: 自定义实现，字母数字保持原样，空格→%20，其他→%XX
- **JSON 解析**: 轻量级字符串解析器，提取 `matches[].offset`、`matches[].length`、`replacements[].value`

## HttpClient 提取与职责分离

提取通用 `HttpClient` 类（~120 行），让 `HttpSpellCheckerAdapter` 仅保留 LanguageTool 协议（~120 行）。

| 类 | 职责 | 行数 |
|----|------|------|
| `HttpClient` | HTTP POST 传输（WinHTTP 样板代码） | ~120 |
| `HttpSpellCheckerAdapter` | LanguageTool 协议（构建请求 + 解析 JSON 响应） | ~120 |

## 配置驱动的产品切换

通过 `.editor_config` 中的 `spellCheckerProduct` 键驱动产品选择：

```
spellCheckerProduct: http    # 使用 LanguageTool HTTP API（默认）
spellCheckerProduct: mock    # 使用内置 Mock 拼写检查器
```

- 使用 map-based 工厂创建具体产品实例
- 新增产品只需在 map 中加一行，无需修改构造函数
- `setSpellChecker()` 仍可在构造后覆盖配置（依赖注入优先级高于配置）

---

# 第六部分：第九阶段重构 — 消除代码坏味道（design-27）

### ConfigSerializer：参数列表过长 + 代码重复

- 新增 `ConfigData` 结构体封装 6 个 out-参数 → `parseConfigLine(key, value, ConfigData&)`
- `ConfigData::toMemento()` 方法直接创建 `WorkspaceMemento`
- 提取 `writeCommaSeparatedList(file, key, items)` 消除列表序列化重复

### TextCommands：继承体系不一致 + 绕过引擎

- `ShowCommand` 改为继承 `TextCommand`，删除重复的 `lines_` / `textEngine_` 成员
- `TextEngine` 新增 `deleteLines(lines, startRow, count)` 和 `insertLine(lines, row, line)` 行级操作方法
- `InsertCommand::undo()` 和 `AppendCommand::undo()` 改为委托 `TextEngine` 而非直接操作 `lines_`

### DirectoryService：重复的路径验证与目录遍历

- 提取 `resolveDirPath(path)` → 返回验证后的 `fs::path`（消除 12×2 行重复）
- 提取 `getSortedEntries(path)` → 返回排序后的条目列表（消除 5×2 行重复）

---

# 附录：测试覆盖总览

| 测试套件 | 测试内容 | 状态 |
|---------|---------|------|
| test_commandparser | 命令解析器（策略模式） | PASS |
| test_commands | 文本编辑器命令 | PASS |
| test_documentmanager | 文档管理器 | PASS |
| test_engine | 文本引擎 | PASS |
| test_log | 日志功能 | PASS |
| test_log_recovery | 日志恢复 | PASS |
| test_loggermanager | 日志管理器 | PASS |
| test_outputservice | 输出服务 | PASS |
| test_workspace | 工作区 | PASS |
| test_editor_factory | 编辑器工厂与XML编辑器（48 个断言） | PASS |
| test_xml_commands | XML 编辑命令（35 个自动化用例） | PASS |
| test_xml_integration | XML 集成测试（xml-tree/init/save/load/验证/遍历） | PASS |
| test_spell_check | 拼写检查（14 项 + 4 项配置测试） | PASS |
| test_edit_duration | 编辑时长统计（9 组测试） | PASS |

---

# 第七部分：design-28 重构 — 消除代码坏味道

按照 `design-28.md` 的要求，针对 9 个代码坏味道进行了重构。全部 14 个测试套件通过，编译无警告。

## 1. Command 基类增强（消除 dynamic_cast 分发）

**修改文件：**
- `include/Command.h` — 新增 `isWorkSpaceLevel()` 虚方法（默认 false）和 `setWorkSpace(WorkSpace*)` 虚方法（默认空实现）
- `include/WorkSpaceCommand.h` — `setWorkSpace` 标记 `override`；新增 `isWorkSpaceLevel()` 返回 true
- `src/CommandFactory.cpp` — `createFromParsed` 中移除 `dynamic_cast<WorkSpaceCommand*>`，改为直接调用 `cmd->setWorkSpace(workspace)`
- `src/CommandController.cpp` — `executeCommand` 中移除 `dynamic_cast<WorkSpaceCommand*>`，改为 `command->isWorkSpaceLevel()` 判断；移除 `#include "WorkSpaceCommand.h"`
- `include/CommandController.h` — 移除 `getActiveTextEditor()` 声明和 `TextEditor*` 前向声明
- `src/CommandController.cpp` — 移除 `getActiveTextEditor()` 实现（含 `dynamic_cast<TextEditor*>`）

**效果：** Command 的多态路由完全由虚方法完成，不再依赖 dynamic_cast 判断命令类型。

## 2. Editor 接口增强（populateContext + initialize）

**修改文件：**
- `include/Editor.h` — 新增 `populateContext(EditorCommandContext& ctx)` 和 `initialize()` 虚方法（默认空实现）
- `include/TextEditor.h` — 新增两个方法的 override 声明
- `src/TextEditor.cpp` — `populateContext` 设置 ctx.lines 和 ctx.textEngine；`initialize` 创建 TextEngine
- `include/XmlEditor.h` — 新增 `populateContext` override 声明
- `src/XmlEditor.cpp` — `populateContext` 设置 ctx.xmlEditor

**效果：** Editor 子类自己提供命令上下文，消除了 CommandFactory 中的 if-else + dynamic_cast 链。

## 3. CommandFactory 简化（消除 OCP 违规 + DIP 违规 + 死参数）

**修改文件：**
- `include/CommandFactory.h` — `createFromParsed` 签名从 3 参数 `(parsed, workspace, TextEditor*)` 简化为 2 参数 `(parsed, workspace)`；移除 `TextEditor` 前向声明
- `src/CommandFactory.cpp` — `buildEditorContext()` 从 ~10 行 if-else 链简化为 `activeEditor->populateContext(ctx)` 一行；移除 `#include "TextEditor.h"` 和 `#include "XmlEditor.h"`

**效果：** 新增 Editor 子类无需修改 CommandFactory，上下文构建由虚方法派发自动完成。

## 4. WorkSpace 简化（消除 dynamic_cast 注入）

**修改文件：**
- `src/WorkSpace.cpp` — `createEditorForExtension()` 从 ~15 行（含 dynamic_cast 注入 TextEngine + 回退分支）简化为 ~6 行（`createEditor → initialize → return`）；移除 `#include "TextEngine.h"` 和 `#include "XmlEditor.h"`

**效果：** 所有编辑器初始化由 `Editor::initialize()` 虚方法自动派发，新增子类无需修改 WorkSpace。

## 5. IXmlDocument 接口拆分（消除胖接口）

**修改文件：**
- `include/IXmlDocument.h` — 原有 29 个方法的单一胖接口拆分为三个微接口 + 一个组合接口：
  - `IXmlReader`（7 方法）：ID 查询与只读访问
  - `IXmlWriter`（5 方法）：元素增删改操作
  - `IXmlNavigator`（13 方法）：加载/保存、生命周期、结构遍历、undo 快照、拼写检查
  - `IXmlDocument`：继承以上三个微接口，保持向后兼容

**效果：** 客户端可按需依赖微接口。所有现有代码通过 `IXmlDocument&` 组合接口仍正常编译。

## 6. OutputService 去重（合并 outputLine/outputText）

**修改文件：**
- `include/OutputService.h` — 移除 `outputText` 声明
- `src/OutputService.cpp` — 移除 `outputText` 实现（与 outputLine 行为完全相同）
- `src/TextCommands.cpp` — `ShowCommand::execute` 中 `outputText` → `outputLine`
- `tests/test_outputservice.cpp` — 3 处 `outputText` → `outputLine`

**效果：** 消除了两个不同名称但相同实现的方法。

## 7. LogCommand 重复逻辑提取

**修改文件：**
- `include/LogCommand.h` — `LogCommand` 基类新增 `protected` 方法 `resolveTargetFile(const std::string& fileName)`
- `src/LogCommand.cpp` — `LogonCommand::execute()` 和 `LogoffCommand::execute()` 中 ~7 行重复的空文件名→活动文件名回退逻辑替换为 `resolveTargetFile(fileName_)` 调用；`LogshowCommand::execute()` 同样简化

**效果：** 消除了 ~20 行重复代码，三个日志命令统一使用同一 helper。

## 设计28 重构统计

| 指标 | 数值 |
|------|------|
| 消除的 dynamic_cast | 6 处（CommandFactory×3 + CommandController×2 + WorkSpace×1） |
| 消除的重复代码 | ~30 行（outputText 实现 + LogCommand 回退逻辑） |
| 移除的死参数 | 1 个（createFromParsed 的 TextEditor*） |
| 拆分的微接口 | 3 个（IXmlReader / IXmlWriter / IXmlNavigator） |
| 新增的虚方法 | 3 个（isWorkSpaceLevel / populateContext / initialize） |
| 移除的无用 include | 6 处（TextEditor / XmlEditor / TextEngine / WorkSpaceCommand） |
| 编译警告 | 0 |
| 测试通过 | 14/14 套件全数通过 |
