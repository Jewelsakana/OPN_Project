### 任务  
修改BUG和实现HttpSpellCheckerAdapter类  
**注意**： 
1. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。   
**BUG**：  
1. 当XML编辑器判断出错误后，会出现两次的错误提示，要将其只变为一次   
**任务**：  
1. 实现对HttpSpellCheckerAdapter类的具体事务，调用https://dev.languagetool.org/public-http-api的api进行拼写检查  
**要求**  
1. 需要实现自动化测试(使用Mock对象进行测试拼写服务)     
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 完成任务时清除相关编译的文件   
