### 当前任务  
构建xmlEditor编辑器和注册相关的命令  
**任务**：  
1. 构建派生于Editor类的XmlEditor类， 解析XML文件为**树形结构**(DOM树)，需要从id到节点对象的快速映射，以支持快速查找  
2. XmlEditor类需要标记是否被修改过  
3. 需要引入一个解析库(PugiXML)编写一个XmlDocumentWrapper 类，将PugiXML的xml_document封装起来，使用适配器模式，以支持不同的对XML解析库的适配  
4. 实现一个XMLEngine类用于后续对XML文件的具体操作（先暂时不实现具体操作）  
5. 实现XMLCommand派生于Command类，XMLCommand 及其派生类应能访问 XmlEditor 的 XMLEngine，具体的先暂时不实现，仅建立一个类   
6. 实现 EditorFactory 类。它应包含一个注册机制，能够根据文件后缀返回 unique_ptr<Editor>，以便后续能够以插件形势扩展新的编辑器等，参考CommandFactory类注册机制的实现方式实现，避免大量if-else或则switch-case语句    
**要求**  
1. 需要实现自动化测试，需覆盖：不同后缀文件的正确识别、XML 节点的 ID 映射正确性、以及 XmlEditor 的 Modified 状态切换   
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 统一要求XML所有元素必须有唯一id，出现重复ID时，应该能记录错误或者排除异常    
4. 完成任务时清除相关编译的文件  

