# XML 编辑命令实现总结

## 已完成功能

### 1. `insert-before` - 在目标元素前插入新元素
- **格式**: `insert-before <tagName> <newId> <targetId> ["text"]`
- **功能**: 在目标元素前（同级）插入一个带指定ID和可选文本的新元素
- **异常处理**:
  - newId 已存在: 提示"元素ID已存在: [newId]"
  - targetId 不存在: 提示"目标元素不存在: [targetId]"
  - 尝试在根元素前插入: 提示"不能在根元素前插入元素"
- **Undo**: 删除插入的新元素

### 2. `append-child` - 在父元素内追加子元素
- **格式**: `append-child <tagName> <newId> <parentId> ["text"]`
- **功能**: 在父元素内追加一个带指定ID和可选文本的子元素（作为最后一个子元素）
- **异常处理**:
  - parentId 无效: 提示"父元素不存在: [parentId]"
  - newId 重复: 提示"元素ID已存在: [newId]"
- **Undo**: 删除追加的子元素

### 3. `edit-id` - 修改元素ID
- **格式**: `edit-id <oldId> <newId>`
- **功能**: 修改某个元素的ID属性
- **异常处理**:
  - oldId 不存在: 提示"元素不存在: [oldId]"
  - newId 已被占用: 提示"目标ID已存在: [newId]"
  - 尝试修改根元素ID: 提示"不建议修改根元素ID"
- **Undo**: 将ID改回原值

### 4. `edit-text` - 修改元素文本
- **格式**: `edit-text <elementId> ["text"]`
- **功能**: 修改某个元素的文本内容，省略或空字符串则清空原内容
- **异常处理**:
  - elementId 不存在: 提示"元素不存在: [elementId]"
- **Undo**: 恢复修改前的文本

### 5. `delete` - 删除元素（XML模式）
- **格式**: `delete <elementId>`
- **功能**: 删除指定ID的元素及其所有子元素
- **注意**: 当第二个参数不含冒号时自动识别为XML删除模式；含冒号时为文本删除模式
- **异常处理**:
  - elementId 不存在: 提示"元素不存在: [elementId]"
  - 尝试删除根元素: 提示"不能删除根元素"
- **Undo**: 重新插入被删除的元素（保存tagName、parentId、文本、位置等快照）

### 6. `xml-tree` - 显示XML树形结构
- **格式**: `xml-tree [file]`
- **功能**: 以树形结构打印XML文件内容，展示元素的层级关系、属性和文本内容
- **参数**: 不指定参数显示当前活动文件，指定file显示指定XML文件
- **只读**: 不进入撤销栈
- **输出**:
  ```
  bookstore [id="root"]
  ├── book [id="book1", category="COOKING"]
  │   └── title [id="title1", lang="en"]
  │       └── "Everyday Italian"
  ```
- **实现**: XmlTreeCommand → OutputService::outputXmlTree → IXmlDocument遍历方法

## 架构说明

### 新增/修改的文件

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `include/IXmlDocument.h` | 修改 | 新增 6个元素操作方法 + 3个undo辅助方法 |
| `include/XmlDocumentWrapper.h` | 修改 | 新增方法声明 + rebuildIdMap辅助 |
| `src/XmlDocumentWrapper.cpp` | 修改 | 实现所有操作方法，操作后自动重建ID映射 |
| `include/XMLEngine.h` | 删除 | 纯委托层，已删除。命令直接依赖IXmlDocument |
| `src/XMLEngine.cpp` | 删除 | 同上 |
| `include/CommandParser.h` | 修改 | 新增5个XML命令枚举值 + 3个XML解析字段 |
| `include/CommandParserStrategy.h` | 修改 | 新增4个XML解析器类声明 |
| `src/CommandParserStrategy.cpp` | 修改 | 实现4个XML解析器 + 扩展DeleteParser支持XML模式 |
| `src/CommandParser.cpp` | 修改 | 注册4个XML命令解析器 |
| `include/CommandFactory.h` | 修改 | 新增XmlEditor*到上下文 + REGISTER_XML_CMD宏 |
| `src/CommandFactory.cpp` | 修改 | 新增isXmlCommandType判断 + XML命令创建逻辑 |
| `include/XmlEditor.h` | 修改 | 移除XMLEngine成员和getXMLEngine() |
| `src/XmlEditor.cpp` | 修改 | 移除XMLEngine相关代码 |
| `include/XMLCommand.h` | 重写 | 5个具体XML命令类，基类绑定IXmlDocument* |
| `src/XMLCommand.cpp` | 重写 | 命令实现 + 自注册宏调用 |
| `tests/test_xml_commands.cpp` | 新增 | 35个自动化测试用例 |
| `tests/test_editor_factory.cpp` | 修改 | 移除XMLEngine测试，改为IXmlDocument测试 |

### 设计要点

1. **适配器模式**: 所有XML操作通过`IXmlDocument`接口，不依赖具体XML库
2. **命令模式**: 每个XML操作封装为独立的Command类，支持execute/undo
3. **策略模式**: 命令解析使用策略模式，每个XML命令有独立的Parser
4. **注册表模式**: 使用自注册宏`REGISTER_XML_CMD`消除工厂switch-case
5. **ID映射同步**: 每次增删改操作后通过`collectIds()`重建完整的ID映射表
6. **Undo支持**: 每个命令在执行前保存必要状态（旧ID、旧文本、子树快照等）
7. **简化架构**: 删除了纯委托层`XMLEngine`，`XMLCommand`直接绑定`IXmlDocument*`（与`TextCommand`绑定`TextEngine*`的模式一致），消除了约150行冗余代码

### 测试覆盖
- 基本操作测试: insert-before, append-child, edit-id, edit-text, delete
- 异常测试: 重复ID、不存在元素、根元素保护
- Undo测试: 所有5种命令的撤销恢复
- 命令行解析测试: 8种解析场景
- ID映射同步测试: 验证操作后映射表正确更新
- 回归测试: 全部11个现有测试套件通过

---

## XML编辑命令第二阶段：xml-tree + init/save/load 兼容 + 命令验证

### 7. `xml-tree` 命令实现
- **XmlTreeParser**: 新增解析器，注册到CommandParser策略表
- **XmlTreeCommand**: 只读命令，不进入撤销栈，直接调用OutputService::outputXmlTree
- **OutputService::outputXmlTree**: 递归遍历IXmlDocument，使用树形字符(├──/└──/│)显示层级，包含属性和文本
- **IXmlDocument 遍历方法**: 新增 getRootId()、getChildIds()、getNodeAttributes() 三个纯虚方法
- **XmlDocumentWrapper**: 实现遍历方法，基于pugixml的children()和attributes()迭代器

### 8. `init` 命令 XML 支持
- **自动类型识别**: 通过文件扩展名(.xml→XmlEditor, .txt→TextEditor)自动创建对应编辑器
- **XML 初始内容**: `<?xml version="1.0" encoding="UTF-8"?>\n<root id="root">\n</root>\n`
- **with-log 支持**: 在XML声明前添加 `# log` 注释行
- **FileCoordinator 重构**: 编辑器工厂从 `function<shared_ptr<TextEditor>()>` 改为 `function<shared_ptr<Editor>(const string& extension)>`

### 9. `save` 和 `load` 命令 XML 支持
- **load**: 检测.xml扩展名 → 创建XmlEditor → 调用XmlEditor::loadFromFile
- **save**: 检测编辑器类型 → XmlEditor::saveToFile 或 TextEditor::getLines + FileSystemService::saveFile
- **FileCoordinator**: 通过 dynamic_cast 区分 XML 和文本编辑器，调用不同的保存/加载方法

### 10. Workspace 编辑器创建
- **EditorFactory 集成**: WorkSpace 通过 EditorFactory::createEditor(extension) 创建对应编辑器
- **TextEngine 注入**: 仅对 TextEditor 注入 TextEngine
- **openFile / restoreOpenFiles**: 根据文件扩展名创建编辑器
- **createEditorForExtension**: 替换原来的 createTextEditor，使用注册表模式避免 switch-case

### 11. 命令类型验证
- **Editor::supportsCommand(EditorCommandType)**: 新增纯虚方法
- **TextEditor**: 支持 Append/Insert/Delete/Replace/Show，拒绝所有 XML 命令
- **XmlEditor**: 支持 InsertBefore/AppendChild/EditId/EditText_/XmlDelete/XmlTree，拒绝所有文本命令
- **CommandFactory**: 在 createFromParsed 中调用 supportsCommand 验证，不匹配则抛出异常

### 新增/修改文件 (第二阶段)

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `include/Editor.h` | 修改 | 新增 pure virtual supportsCommand() |
| `include/CommandParser.h` | 修改 | 新增 XmlTree 枚举值 |
| `include/CommandParserStrategy.h` | 修改 | 新增 XmlTreeParser 类声明 |
| `src/CommandParserStrategy.cpp` | 修改 | 实现 XmlTreeParser |
| `src/CommandParser.cpp` | 修改 | 注册 XmlTreeParser |
| `include/IXmlDocument.h` | 修改 | 新增 getRootId/getChildIds/getNodeAttributes |
| `include/XmlDocumentWrapper.h` | 修改 | 新增遍历方法声明 |
| `src/XmlDocumentWrapper.cpp` | 修改 | 实现遍历方法 |
| `include/OutputService.h` | 修改 | 新增 outputXmlTree 方法声明 |
| `src/OutputService.cpp` | 修改 | 实现 outputXmlTree 递归树形输出 |
| `include/XMLCommand.h` | 修改 | 新增 XmlTreeCommand 类 |
| `src/XMLCommand.cpp` | 修改 | 实现 XmlTreeCommand + 自注册 |
| `include/TextEditor.h` | 修改 | 新增 supportsCommand 声明 |
| `src/TextEditor.cpp` | 修改 | 实现 supportsCommand |
| `include/XmlEditor.h` | 修改 | 新增 supportsCommand 声明 |
| `src/XmlEditor.cpp` | 修改 | 实现 supportsCommand |
| `include/FileCoordinator.h` | 修改 | 工厂签名改为按扩展名创建Editor |
| `src/FileCoordinator.cpp` | 重写 | 支持.txt和.xml双格式的load/save/init |
| `include/WorkSpace.h` | 修改 | createTextEditor → createEditorForExtension |
| `src/WorkSpace.cpp` | 修改 | 集成EditorFactory，扩展名感知编辑器创建 |
| `src/CommandFactory.cpp` | 修改 | 新增 supportsCommand 验证逻辑 |
| `tests/test_xml_integration.cpp` | 新增 | 12组集成测试(xml-tree/init/save/load/command validation/traversal) |
| `tests/test_editor_factory.cpp` | 修改 | MockEditor 实现 supportsCommand |

### 设计要点 (第二阶段)

1. **扩展名驱动**: 通过文件扩展名自动选择编辑器类型，使用EditorFactory注册表避免switch-case
2. **接口隔离**: Editor::supportsCommand 让命令验证逻辑集中在编辑器层，不在命令类中添加判断
3. **注册表模式**: 所有编辑器类型通过 REGISTER_EDITOR 宏注册，新增类型无需修改工厂代码
4. **统一门面**: WorkSpace 通过 createEditorForExtension 统一编辑器创建入口
5. **只读命令**: XmlTreeCommand 设置 isReadOnly=true，不进入撤销栈
6. **遍历接口**: IXmlDocument 新增3个遍历方法，仅暴露字符串接口，不泄露pugi类型

---

## 第三阶段：重构（设计22）

对OutputService、FileCoordinator、CommandFactory、XmlDocumentWrapper四个类进行重构。

### 12. OutputService 重构
- **问题**: outputXmlTree 根节点打印与递归逻辑重复，约70行过长，嵌套两个lambda
- **修改**:
  - 提取 `printXmlSubTree(IXmlDocument&, id, prefix, indent)` 私有递归方法，根节点和子节点统一用同一逻辑
  - 提取 `formatXmlAttrs(IXmlDocument&, id)` 静态私有方法格式化属性字符串
  - `outputXmlTree` 入口缩减为 ~15行，只获取根ID并调用递归方法
  - 根节点使用空 prefix/indent 传入，不再特殊处理

### 13. FileCoordinator 重构
- **问题**: dynamic_cast 分支在 loadFile/saveFile/initFile 中重复3次; createEditorForFile 和 isXmlFile 各自实现相同扩展名提取; 对编辑器内部实现知道太多
- **修改**:
  - 在 `Editor` 接口新增 `loadFromData(string)`, `saveToData() -> string`, `initContent(bool withLog)` 多态方法，由各编辑器子类实现
  - `TextEditor::loadFromData/saveToData/initContent`: 文本行与字符串之间的转换
  - `XmlEditor::loadFromData/saveToData/initContent`: 委托给 XmlDocumentWrapper 的字符串方法
  - `FileCoordinator::loadFile`: 读取原始内容 → `editor->loadFromData(content)`
  - `FileCoordinator::saveFile`: `editor->saveToData()` → 写入文件
  - `FileCoordinator::initFile`: `editor->initContent(withLog)`
  - 消除所有 `dynamic_cast<XmlEditor*>` / `dynamic_cast<TextEditor*>` 分支
  - 提取 `getFileExtension()` 辅助函数消除重复
  - 移除 `createEditorForFile` 方法，内联扩展名提取+工厂调用

### 14. CommandFactory 重构
- **问题**: EditorCommandContext 字段互不相关强行捆绑; isXmlCommandType 硬编码XML命令类型与 supportsCommand 信息重复
- **修改**:
  - `EditorCommandContext::lines` 从 `vector<string>&` 改为 `vector<string>*`（指针），消除 dummyLines 静态变量
  - `EditorCommandContext::textEngine` 从引用改为指针
  - 新增 `buildEditorContext(Editor*, WorkSpace*)` 函数通过 dynamic_cast 确定活跃编辑器类型，填充对应上下文字段
  - 移除 `isXmlCommandType()` 函数（Editor::supportsCommand 已在外部完成验证）
  - 更新 `REGISTER_EDITOR_CMD_GUARDED` 宏：改用 `ctx.lines` 空指针检查，使用 `*ctx.lines` 解引用
  - 更新 `REGISTER_EDITOR_CMD_SHOW` 宏：同上
  - `REGISTER_XML_CMD` 宏：使用 `ctx.xmlEditor` 空指针检查

### 15. XmlDocumentWrapper 重构
- **问题**: insertBefore 和 appendChild 创建元素代码重复; collectIds 根节点与递归lambda重复; getPugiDocument/root 为 public 暴露 pugi 类型
- **修改**:
  - 提取 `createElement(parent, tagName, id, text, insertBefore, targetNode)` 私有方法统一创建元素逻辑
  - 提取 `registerNodeId(node)` 私有方法统一 ID 验证 + 去重 + 插入映射逻辑
  - `collectIds()` 根节点和子节点统一通过 `registerNodeId` + 递归遍历处理，减少约 20 行重复代码
  - `getPugiDocument()` 和 `root()` 改为 private，外部不再能访问 pugi 类型
  - 新增 `rebuildIdMap()` 私有方法（等同于 `collectIds()`），语义更清晰

### 修改文件 (第三阶段)

| 文件 | 变更类型 | 说明 |
|------|---------|------|
| `include/Editor.h` | 修改 | 新增 loadFromData/saveToData/initContent 多态方法 |
| `include/TextEditor.h` | 修改 | 新增多态方法声明 |
| `src/TextEditor.cpp` | 修改 | 实现多态方法 |
| `include/XmlEditor.h` | 修改 | 新增多态方法声明 |
| `src/XmlEditor.cpp` | 修改 | 实现多态方法 |
| `include/FileCoordinator.h` | 修改 | 工厂签名改为按扩展名创建Editor; 移除 createEditorForFile |
| `src/FileCoordinator.cpp` | 重写 | 多态派发消除 dynamic_cast; 提取 getFileExtension |
| `include/FileSystemService.h` | 修改 | 新增 readFileContent/writeFileContent |
| `src/FileSystemService.cpp` | 修改 | 实现原始文件读写 |
| `include/OutputService.h` | 修改 | 新增 printXmlSubTree/formatXmlAttrs 私有方法 |
| `src/OutputService.cpp` | 修改 | 分解 outputXmlTree 为3方法 |
| `include/CommandFactory.h` | 修改 | EditorCommandContext 引用改指针; 更新宏 |
| `src/CommandFactory.cpp` | 修改 | 新增 buildEditorContext; 移除 isXmlCommandType/dummyLines |
| `include/XmlDocumentWrapper.h` | 修改 | 新增 createElement/registerNodeId/rebuildIdMap; getPugiDocument/root 改 private |
| `src/XmlDocumentWrapper.cpp` | 重写 | 提取公共方法消除重复; 统一 collectIds |
| `include/TextEngine.h` | 修改 | 新增 linesToString/stringToLines 序列化方法 |
| `src/TextEngine.cpp` | 修改 | 实现序列化/反序列化方法 |
| `src/TextEditor.cpp` | 修改 | loadFromData/saveToData 委托给 TextEngine |
| `src/FileCoordinator.cpp` | 修改 | 移除 !isXml 守卫，XML文件也支持日志记录 |

### 设计要点 (第三阶段)

1. **多态替代 dynamic_cast**: FileCoordinator 通过 Editor 多态接口消除所有 dynamic_cast 分支，FileCoordinator 不再需要知道编辑器具体类型
2. **接口解耦**: Editor 子类各自实现 loadFromData/saveToData/initContent，FileCoordinator 只依赖 Editor 抽象接口
3. **单一职责**: OutputService::outputXmlTree 拆分为入口+递归+格式化三个职责明确的方法
4. **公共逻辑提取**: XmlDocumentWrapper 的 createElement/registerNodeId 消除了 insertBefore 与 appendChild 之间、collectIds 根节点与递归之间的重复代码
5. **封装强化**: XmlDocumentWrapper 的 getPugiDocument/root 改为 private，pugi 类型不再对外暴露
6. **消除硬编码**: CommandFactory 移除 isXmlCommandType switch，上下文构建通过 dynamic_cast 判断活跃编辑器类型，新增编辑器类型无需修改此代码

### 补充修复 (第三阶段)

#### 16. TextEditor 序列化委托给 TextEngine
- **问题**: TextEditor::loadFromData/saveToData 直接实现 string ↔ vector&lt;string&gt; 转换逻辑，但 TextEngine 作为文本操作引擎已有 show() 等方法，序列化应归入 TextEngine
- **修改**:
  - TextEngine 新增 `stringToLines(content)` 反序列化方法和 `linesToString(lines)` 序列化方法
  - TextEditor::loadFromData 委托 `textEngine->stringToLines(content)`
  - TextEditor::saveToData 委托 `textEngine->linesToString(lines)`
  - TextEditor.cpp 移除不再需要的 `#include <sstream>`

#### 17. XML 文件日志记录支持
- **问题**: FileCoordinator 中 `initFile` 和 `loadFile` 的 `!isXml` 守卫阻止 XML 文件启用日志。实际上观察者通知机制在 CommandController::parseAndExecuteCommand 中统一调用 `workspace_->notify(event)`，对两种编辑器类型都有效
- **修改**:
  - `initFile`: 移除 `!isXml` 守卫，`if (withLog)` 直接启动日志
  - `loadFile`: 移除 `!isXml` 守卫，检测到 `# log` 头即启动日志
  - pugixml 可容忍 `# log` 文本在 XML 声明之前（作为文档级 PCDATA 节点），root 元素仍正常解析
