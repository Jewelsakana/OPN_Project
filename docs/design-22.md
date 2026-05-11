### 当前任务  
重构OutputService类，FileCoordinator类，CommandFactory类，XmlDocumentWrapper类  
**问题**：  
* OutputService类：
  1. outputXmlTree 方法的根节点打印逻辑与printNodeRec 内部逻辑几乎完全相同  
  2. outputXmlTree 约 70 行，过长了，要把递归遍历逻辑提取为独立的私有方法  
  3. 嵌套过深了，内置两个lambda方法，整体复杂度很高  
* FileCoordinator类：  
  1. createEditorForFile 和 isXmlFile 各自独立实现了相同的 rfind('.') + substr 逻辑  
  2. dynamic_cast 分发模式重复 3 次：loadFile,saveFile,initFile中都有相同的if-XmlEditor-else-TextEditor 分支结构  
  3. case语句需要并行修改，都使用dynamic_cast 按类型分支，需要修改的地方太多了  
  4. FileCoordinator 直接访问 TextEditor的getLines()/setLines() 和 XmlEditor的loadFromFile()/saveToFile()/loadFromString()，对各个编辑器的内部实现知道得太多。应该通过 Editor 多态接口来解耦。  
* CommandFactory类：
  1. EditorCommandContext把互不相关的字段强行捆绑  
  2. isXmlCommandType()硬编码了哪些类型是XML命令，但Editor::supportsCommand() 已经提供了相同的多态信息  
  3. createEditorCommand()和 createWorkSpaceCommand()是完全相同的结果：查注册表->调用创建器->为空抛出异常  
* XmlDocumentWrapper类:  
  1. 新建元素模式重复:insertBefore和appendChild有完全相同的地方  
  2. collectIds 中根节点与递归 lambda 重复，根节点单独处理了一遍 ID 验证/去重/加入映射的逻辑，与 lambda 内部完全一致。  
  3. getPugiDocument() 和 root() 是 public 方法，直接暴露 pugi::xml_node 和 pugi::xml_document 类型  
**修改**：  
1. 	FileCoordinator：在 Editor 接口中增加 loadFromFile/saveToFile/initContent 多态方法，消除三个方法中的 dynamic_cast 分支链  
2. FileCoordinator:提取公共扩展名解析函数，消除 isXmlFile 和 createEditorForFile 中的重复  
3. OutputService:将 outputXmlTree 根节点打印统一到递归方法中（让根节点也用 printNodeRec），消除重复  
4. OutputService: 将 outputXmlTree 拆分为两个方法（公共入口 + 私有递归遍历）  
5. XmlDocumentWrapper:提取公共的"创建带 ID 和文本的新元素"方法；将 getPugiDocument/root 改为 private + friend  
6. CommandFactory:将 EditorCommandContext 拆分为 TextEditorContext / XmlEditorContext，消除 dummyLines；用 supportsCommand 替换 isXmlCommandType switch  
7. 如果还有其他解决上述问题的方法，可以告诉我进行后使用  
**要求**：  
1. 根据上述修改进行重构  
2. 需要重新编译通过所有的测试用例  
3. 需要清除编译出来的.o和.exe文件  
4. 将修改写入SUMMARY_2.md  