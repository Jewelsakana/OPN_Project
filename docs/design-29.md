### 当前任务  
插件化的实现  
**注意**： 
1. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。
**任务**：
1. EditorCommandType是封闭枚举：  
   * 原因：使用封闭枚举会让插件无法再运行时注册新命令类型   
   * 解决思路：将命令类型从enum class 改为可运行时可注册的标志（如int id + string name），插件调用CommandRegistry::registerType("sudoku-set")获取ID  
2. EditorCommandContext 字段是硬编码的:  
   * 只有lines/textEngine(文本)和xmlEditor(XML)四字段，插件无法在不改动核心头文件的情况下添加  
   * 解决思路：使用 any/void* 机制，让每个 Editor 子类提供自己的上下文对象  
3. EditorParsedCommand字段是为文本/XML设计的  
   * tagName、newId、targetId 是 XML 专用字段，插件只能通过复用现有字段（语义错位）或者修改结构体来支持  
   * 解决思路：将解析结果改为variant或者继承体系，让插件定义自己的解析结构类型  
4. registerStrategies() 是集中式硬编码列表  
   * 所有的解析器都在此手动push_back，插件无法在此函数之外注册自己的解析器  
   * 解决思路：将策略注册改为静态自注册，让插件.cpp文件的静态初始化自动注册  
5. Editor.h中包含了ISpellChecker.h，为了进行TextSegment类型，应该将TextSegment转移到独立的头文件中，让Editor接口不依赖ISpellChecker  
6. 实现packgae隔离，将所有源文件按照package实现类型进行隔离，从而让插件能够作为独立的，可插拔的单元管理，同时应该修改Makefile文件，实现自动化编译  
**要求**  
1. 需要实现或者修改自动化测试  
2. 需要将所完成的功能写入SUMMARY_1.2.md中  
3. 完成任务时清除相关编译的文件  
