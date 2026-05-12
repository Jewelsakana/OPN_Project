### 当前任务  
拼写检查模块的实现  
**注意**：  
1. 实现命令时需要从CommandParser.h加入enum字段注册宏调用，注册策略还需要在CommandParserstratery.h/.cpp中加入解析器，并在自身的Command类自注册编辑器命令工厂以及在CommandFactory类中添加自注册宏  
2. 最好不要看pugiconfig.hpp和pugixml.hpp这两个类，这是外部库，很大。
3. 要方便地能够切换不同的拼写检查服务，而不是与某一个具体产品绑定  
4. 主要是第三方库管理实现，而不是算法实现  
**任务**：
1. 使用适配器模式，让第三方库依赖被限制在适配器内，创建ISpellChecker 接口，让编辑器依赖接口而非具体实现  
2. 能够通过配置或者注入方便地切换不同拼写检查产品，让依赖从外部注入而非内部实现，同时可以使用Mock对象进行测试，而非真实库中  
3. 使用API：https://dev.languagetool.org/public-http-api 进行拼写检查服务  
4. 创建命令：`spell-check` - 拼写检查  
* 格式：spell-check [file]  
* 功能：检查文本文件，xml文件中的拼写服务  
* 参数说明：  
  * 不指定参数：检查当前活动文件  
  * `file`：检查知道文本文件  
* 输出格式参考：  
``` 
拼写检查结果:  
第1行，第5列: "recieve" -> 建议: receive  
第3行，第12列: "occured" -> 建议: occurred  
```  
```  
拼写检查结果:  
元素 title1: "Itallian" -> 建议: Italian  
元素 author2: "Rowlling" -> 建议: Rowling  
```    
5. 在Editor基类中定义一个获取获取待检查文本列表的虚函数，让 XmlEditor 和 TextEditor 分别实现它（XML 需过滤掉标签名，仅保留 Text 节点内容）。  
6. 拼写检查的结果应该由适配器中返回一个统一的结构体列表，最后由OutputService类负责输出  
7. 考虑到C++环境，可以先实现一个Mock适配器，预留HttpSpellCheckerAdapter 的结构  
**要求**  
1. 需要实现自动化测试(使用Mock对象进行测试拼写服务)     
2. 需要将所完成的功能写入SUMMARY_2.md中  
3. 完成任务时清除相关编译的文件   
4. 保证不与某一个具体的产品绑定  