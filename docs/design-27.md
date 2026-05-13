### 当前任务  
重构ConfigSerializer类，TextCommands类，DirectoryService类  
**任务**：  
1. ConfigSerializer类：parseConfigLine()7个参数中有6个out-参数，本来就是WorkspaceMemento的字段集合，散落为独立参数是反模式，应该用一个结构体封装， saveConfig()中openFiles和loggedFiles的逗号分隔列表输出逻辑完全相同（for循环+条件逗号），可以提取重复的列表序列化重复  
2. TextCommands类：ShowCommand 直接继承 Command，让ShowCommand 的 lines_、textEngine_ 成员与 TextCommand 重复声明。InsertCommand类，AppendCommand类中的多行插入的分支直接调用了底层逻辑，绕过了TextEnigne，这些数据操作应该委托给TextEngine  
3. DirectoryService类：getDirectoryTree() 和 getDirectoryTreeStructure() 中的路径验证逻辑逐字重复，buildDirectoryTree() 和 buildDirectoryTreeStructure() 中收集+排序目录条目的代码也完全相同。buildDirectoryTree()混合了遍历逻辑和 ASCII art 前缀拼接。核心的递归遍历目录逻辑应该只写一次。输出格式应该作为遍历的不同消费策略  
**要求**  
1. 需要实现或者修改自动化测试  
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 完成任务时清除相关编译的文件  