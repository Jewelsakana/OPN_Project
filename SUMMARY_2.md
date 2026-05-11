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
- 暂未实现，等后续再实现

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
