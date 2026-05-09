### 当前任务    
再对不符合设计要求的地方进行重构  
**任务**：  
1. 自注册 lambda 高度重复。可以统一为一个宏或者模版函数  
2. 消除双重 setWorkSpace()，应该只在工厂中设置，控制器去掉重复的调用  
3. 子程序过长：loadConfig()、restoreFromMemento()、ExitCommand::execute()  
   * ConfigSerializer::loadConfig() — 将键值对解析提取为独立方法  
   * WorkSpace::restoreFromMemento() — 按职责拆分  
   * ExitCommand::execute() — 将未保存文件的检查和配置保存提取出来  
4. 修正 WorkSpace 接口的抽象层次：notifySessionStart() 中的时间格式化逻辑和 showLog() 中的文件名解析逻辑都不应该出现在门面层， 将时间格式化移到工具函数或 Event 类中，showLog 中的默认文件名解析下放到 LogCoordinator  
5. 数据成员封装：TextCommand::lines 和 TextCommand::textEngine 从 protected 改为 private，提供 protected getter  
**要求**：  
1. 根据上述修改进行重构  
2. 需要重新编译通过所有的测试用例  
3. 需要清除编译出来的.o和.exe文件  
4. 将修改写入重构总结.md  