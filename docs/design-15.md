### 当前任务  
对整个程序进行重构   
**原因**：  
1. CommandParser.cpp:代码重复，子程序过长，case语句需要并行修改，基本数据类型过度使用，使用了全局变量  
2. CommandControll.cpp:case语句需要并行修改,子程序过长,类间过于亲密  
3. WorkSpace.cpp:类内聚性差，接口抽象层次不一样  
4. FileSystemService.cpp:类内聚性差,代码重复,子程序过长  
5. TextCommands.cpp：代码重复，参数列表过长，基本数据类型使用过度  
6. CommandParser.h:数据成员公有，相关联数据未组织成类  
**修改**：  
1. CommandParser.cpp:使用策略模式替代硬编码的if-else链，让每个命令自行注册其解析器  
2. CommandControll.cpp:将 createCommandFromParsed()的创建逻辑移到各命令类自身（工厂方法），消除并行switch   
3. WorkSpace.cpp:拆分：将配置持久化逻辑移到独立的configManager类，将观察者管理提取为mixin/基类  
4. FileSystemService.cpp:拆分为 FileIOService + DirectoryService + ConfigSerializer  
5. TextCommands.cpp：提取公共的删除前记录逻辑到基类TextCommand;封装Position对象  
6. CommandParser.h:将ParsedCommand 的字段按命令类型分离（用 std::variant 或继承），封装公有数据成员  
7. 提取公共工具函数(trim,splitSpring)到独立的StringUtils头文件，消除重复定义  
**要求**：  
1. 根据上述修改进行重构  
2. 需要重新编译通过所有的测试用例  
3. 需要清除编译出来的.o和.exe文件  
4. 将修改写入一个新的重构.md  
