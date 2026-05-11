# XML编辑器框架实现总结

## 项目概述
基于design-19.md的设计要求，在现有命令行文本编辑器框架基础上，扩展实现了XML编辑器子系统。包括XML文档解析、树形结构（DOM树）构建、ID到节点的快速映射、XML编辑引擎、XML命令框架以及基于注册表模式的EditorFactory。

## 完成的核心模块

### 1. XML解析库集成
- **PugiXML库** (`pugixml.hpp`, `pugixml.cpp`, `pugiconfig.hpp`)：引入轻量级C++ XML解析库PugiXML（v1.14），作为底层XML解析引擎。PugiXML以单头文件+源文件形式分发，支持DOM遍历、属性访问和文件读写。

### 2. XmlDocumentWrapper（适配器模式）
- **XmlDocumentWrapper类** (`XmlDocumentWrapper.h/.cpp`)：使用适配器模式封装PugiXML的`xml_document`，提供统一的XML文档操作接口，便于后续替换底层XML解析库。
  - `loadFromString()`：从字符串加载XML
  - `loadFromFile()`：从文件加载XML
  - `saveToString()`：将XML保存为字符串
  - `saveToFile()`：将XML保存到文件
  - `collectIds()`：递归遍历所有元素节点，收集ID到节点的映射，检测重复ID和缺失ID
  - 底层可通过`getDocument()`访问原始PugiXML文档对象
- **异常体系**：
  - `XmlDocumentException`：XML文档异常基类
  - `XmlParseException`：解析错误异常（格式错误、文件不存在等）
  - `DuplicateIdException`：重复ID异常
  - `MissingIdException`：缺少ID属性异常

### 3. XmlEditor（XML编辑器）
- **XmlEditor类** (`XmlEditor.h/.cpp`)：派生自`Editor`接口，解析XML文件为DOM树形结构，维护ID到节点的快速映射（`std::unordered_map<std::string, pugi::xml_node>`）。
  - 继承`Editor`接口：实现`executeCommand()`、`undo()`、`redo()`、`canUndo()`、`canRedo()`
  - `loadFromFile()` / `loadFromString()`：加载XML并自动构建ID映射
  - `saveToFile()`：保存XML到文件，自动清除修改标记
  - `findNodeById()`：通过ID快速查找节点（O(1)哈希查找）
  - `hasNodeWithId()`：检查ID是否存在
  - `getAllIds()`：获取所有已注册的节点ID列表
  - `rebuildIdMapping()`：文档修改后重建ID映射
  - **修改状态标记**：`isModified()` / `setModified()` 追踪文档是否被修改
  - `CommandManager`作为私有组件，提供独立的Undo/Redo栈

### 4. XMLEngine（XML操作引擎）
- **XMLEngine类** (`XMLEngine.h/.cpp`)：继承自`Model`基类，为后续XML文件的具体操作提供框架。
  - `findNodeById()`：通过ID查找XML节点（委托给XmlEditor）
  - `isDocumentLoaded()`：检查文档是否已加载
  - `validate()` / `isValid()`：Model接口实现
  - 持有XmlEditor指针（不持有所有权），通过编辑器访问文档

### 5. XMLCommand（XML命令框架）
- **XMLCommand类** (`XMLCommand.h/.cpp`)：派生自`Command`接口，XML编辑命令的基类。
  - 通过`getEngine()`（protected）使派生类可以访问`XMLEngine`
  - 通过`getEditor()`（protected）使派生类可以访问`XmlEditor`
  - `execute()`和`undo()`留给具体子类实现
  - 当前为框架结构，具体命令操作预留给后续扩展

### 6. EditorFactory（编辑器工厂 + 注册表模式）
- **EditorFactory类** (`EditorFactory.h/.cpp`)：使用注册表模式（Registry Pattern），根据文件后缀返回对应的`unique_ptr<Editor>`，避免大量if-else或switch-case语句。
  - `createEditor(fileExtension)`：根据后缀创建对应的Editor实例
  - `registerEditor(extension, creator)`：注册编辑器创建器
  - 注册表使用函数内静态对象（Meyer's Singleton），避免静态初始化顺序问题
- **自注册宏**：`REGISTER_EDITOR(EXT, CLASS)`——在Editor子类的.cpp文件中使用，实现启动时自动注册，无需修改工厂代码即可扩展新的编辑器类型。
  - `TextEditor`注册为`.txt`后缀处理器
  - `XmlEditor`注册为`.xml`后缀处理器

### 7. ID唯一性验证
- 加载XML时自动执行ID唯一性验证
- 所有XML元素必须有唯一的`id`属性
- 缺少ID属性时抛出`MissingIdException`
- 发现重复ID时抛出`DuplicateIdException`
- 异常包含具体的节点名称或重复ID值，便于定位问题

## 设计模式应用

| 设计模式 | 应用位置 | 说明 |
|---------|---------|------|
| 适配器模式 | XmlDocumentWrapper | 封装PugiXML，便于替换底层XML解析库 |
| 工厂模式 | EditorFactory | 根据文件后缀创建对应Editor |
| 注册表模式 | EditorFactory + REGISTER_EDITOR | 自注册机制，支持插件化扩展 |
| 命令模式 | XMLCommand | XML编辑操作封装为命令对象 |
| 模板方法模式 | Model基类 | safeExecute()提供统一异常处理框架 |

## 测试覆盖

### 测试文件：`tests/test_editor_factory.cpp`（10个测试组，共48个测试断言）

| 测试组 | 测试内容 | 通过状态 |
|--------|---------|---------|
| Test 1 | EditorFactory根据后缀正确创建Editor（5项） | PASS |
| Test 2 | XmlDocumentWrapper加载和保存XML（6项） | PASS |
| Test 3 | XML节点ID映射正确性（5项） | PASS |
| Test 4 | 重复ID检测（2项） | PASS |
| Test 5 | XmlEditor Modified状态切换（6项） | PASS |
| Test 6 | XmlEditor ID查找（5项） | PASS |
| Test 7 | XMLEngine基本功能（5项） | PASS |
| Test 8 | XMLCommand基本结构（3项） | PASS |
| Test 9 | 复杂XML嵌套元素映射（4项） | PASS |
| Test 10 | EditorFactory注册机制扩展（4项） | PASS |

### 测试覆盖的核心需求：
1. **不同后缀文件的正确识别**（Test 1, Test 10）：验证`.txt`→TextEditor、`.xml`→XmlEditor、未知后缀→nullptr
2. **XML节点的ID映射正确性**（Test 3, Test 6, Test 9）：验证ID数量、节点名称、节点内容、层级结构
3. **XmlEditor的Modified状态切换**（Test 5）：验证初始状态、加载后、修改后、保存后、清空后的状态变化

## 构建和运行

```bash
# 构建项目
mingw32-make all

# 运行所有测试
mingw32-make test

# 运行特定测试
g++ -std=c++17 -Iinclude -I. tests/test_editor_factory.cpp build/*.o -o build/test_editor_factory
./build/test_editor_factory

# 清理编译文件
mingw32-make clean
```

## 新增文件清单

### 头文件（include/）
- `pugixml.hpp` — PugiXML库头文件
- `pugiconfig.hpp` — PugiXML编译配置
- `XmlDocumentWrapper.h` — XML文档适配器接口
- `XMLEngine.h` — XML操作引擎接口
- `XmlEditor.h` — XML编辑器接口
- `XMLCommand.h` — XML命令基类接口
- `EditorFactory.h` — 编辑器工厂接口（含自注册宏）

### 源文件（src/）
- `pugixml.cpp` — PugiXML库实现
- `XmlDocumentWrapper.cpp` — XML文档适配器实现
- `XMLEngine.cpp` — XML操作引擎实现
- `XmlEditor.cpp` — XML编辑器实现（含.xml自注册）
- `XMLCommand.cpp` — XML命令基类实现
- `EditorFactory.cpp` — 编辑器工厂实现

### 测试文件（tests/）
- `test_editor_factory.cpp` — 编辑器工厂与XML编辑器测试套件

### 文档（docs/）
- `SUMMARY_2.md` — 本实现总结文档

### 修改的现有文件
- `src/TextEditor.cpp` — 添加`#include "EditorFactory.h"`和`.txt`后缀自注册

## 架构图

```
Editor (接口)
  ├── TextEditor (.txt)
  └── XmlEditor (.xml)
        ├── XmlDocumentWrapper (适配器) → pugi::xml_document
        ├── XMLEngine (操作引擎)
        ├── CommandManager (Undo/Redo管理)
        └── idToNodeMap_ (ID→节点快速映射)

EditorFactory (注册表模式)
  └── registry: map<extension, creator>
        ├── ".txt" → TextEditor
        ├── ".xml" → XmlEditor
        └── ... (支持插件扩展)

Command (接口)
  ├── TextCommand → InsertCommand, DeleteCommand, ...
  └── XMLCommand → (后续扩展: InsertXMLCommand, DeleteXMLCommand, ...)
```
