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
