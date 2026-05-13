### 任务  
实现通过配置来切换拼写检查产品   
**注意**： 
1. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。  
**当前问题和思路**  
1. 拼写检查器应该需要配置读取来实现切换不同的检查器，现在被硬编码为MockSpellChecker，需要更改为读取配置来实现检查器的切换  
2. 当前loadConfig()在spellChecker_初始化之后才调用，即使配置了产品页来不及  
3. 在ConfigSerializer类中新增对拼写检查产品键的解析和保存拼写检查产品键的逻辑  
4. 在WoskSpace类中新增加spellCheckerProduct_ 字段根据memento 的 product 字段调用 setSpellChecker()  
5. 先loadConfig()再初始化拼写检查器，或者提供一个默认回退  
6. 如果配置缺失最好默认为Http的拼写检查产品，使用map根据字符串创建对应的实例  
**要求**  
1. 需要实现或者修改自动化测试  
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 完成任务时清除相关编译的文件   
