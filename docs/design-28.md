### 当前任务  
重构以及插件化的部分实现
**注意**： 
1. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。  
**任务**：  
1. CommandFactory类中的buildEditorContext()使用if-else链+dynamic_cast 区分编辑器类型，每增加一种Editor子类就必须修改此函数，将上下文构建职责委托给Editor子类自身，如添加populateContext(EditorCommandContext& ctx)   
2.  ommandFactory类中buildEditorContext() 深度访问了 TextEditor 的内部（getLinesRef()、getTextEngine()）和 XmlEditor 的内部（getDocument()），却属于 CommandFactory。需要将上下文构建逻辑移动到Editor子类自身来解决  
3. CommandFactory类中的buildEditorContext直接依赖于具体类TextEditor 和 XmlEditor，而非依赖于Editor抽象，且 createFromParsed 签名接收 TextEditor* 参数，但实际内部通过 workspace->getActiveEditor() 获取抽象 Editor。 
4. CommandFactory类中createFromParsed 用 dynamic_cast<WorkSpaceCommand*> 直接调用 setWorkSpace(),CommandFactory类中知道WorkSpaceCommand的存在和内部connect机制，破坏了信息隐藏。建议在 Command 基类添加虚方法让命令本身决定是否需要 workspace 连接  
5. WorkSpace类中同样用 dynamic_cast 判断 TextEditor 类型来注入 TextEngine，新增编辑器子类需在此添加新分支  
6. IXmlDocument类中是胖接口，包含加载/保存,ID查询，元素变更，XML遍历，Undo/拼写辅助。建议拆分为多个微接口:IXmlReader、IXmlWriter、IXmlNavigator，由 IXmlDocument 多继承。   
7. OutputService类中outputLine() 和 outputText()实现方法完全相同，是两个不同名称但是相同行为的方法，将其合并更改  
8. LogCommand类中LogonCommand::execute() 和LogoffCommand::execute()从空文件名->活动文件名回退的逻辑重复，提取出相同函数  
9.  CommandController类中executeCommand() 用 dynamic_cast 区分 WorkSpaceCommand 和普通 Command。  
**要求**  
1. 需要实现或者修改自动化测试  
2. 需要将所完成的功能写入SUMMARY_1.2.md中  
3. 完成任务时清除相关编译的文件  