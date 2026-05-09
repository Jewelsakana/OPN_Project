### 当前任务  
对仍然需要重构的地方进行重构  
**任务**：  
1. CommandFactory.cpp中仍然有两个大的switch语句，利用注册表模式替代，在CommandFactor.cpp中维护一个std::unordered_map<CommandType，std::function<unique_ptr<Command>()>>，每个命令类在程序启动时候自注册  
2. Editor接口太薄，使用undo(),redo(),canUndo(),canRedo()提升到Editor接口中，提供默认空实现  
3. 空指针检查重复多次,在workSpaceCommand中的基类加入一个protected方法checkWorkSpace(),把判断空指针集中在一处，execute() 和 undo() 开头调用它即可。同理把UndoCommand和RedoCommand获取TextEditor的重复逻辑也抽成基类的protected的辅助方法  
4. WorkSpace职责过多。 将职责按域区分开：  
    * FileCoordinator:文件加载/保存/初始化/关闭   
    * EditorCoordinator:编辑器切换和查询  
    * LogCoordinator:日志启停和查询  
    * ConfigCoordinator:配置保存/恢复   
WorkSpace 本身退化为持有这些 Coordinator 的门面，只做组合和委托  
5. FileIOService 别名:直接删掉  
6. const_cast 绕过类型安全：ShowCommand 是只读命令，不应该持有非 const 引用。修改 TextCommand 的设计，让 TextCommand 接受 const 引用（只读命令用）  
**要求**：  
1. 根据上述修改进行重构  
2. 需要重新编译通过所有的测试用例  
3. 需要清除编译出来的.o和.exe文件  
4. 将修改写入重构总结.md  
