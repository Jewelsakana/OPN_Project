### 当前任务  
统一输出逻辑  
**任务**：  
1. 创建 OutputService 类，继承 Model 基类  
2. 在 WorkSpace 中持有 OutputService 实例  
3. 修改命令类使用 OutputService：  
    * EditorListCommand → outputService.outputList(...)  
    * DirTreeCommand → outputService.outputTree(...)  
    * ExitCommand → outputService.outputError(...) 和 outputService.outputLine(...)  
    * ShowCommand输出文本
4. 确保outputList 和 outputTree 接收的是结构化数据，而不是已经拼好的字符串，所以可能会在DoucumentManager和FileSystemService类中需要修改相关的函数  
5. 统一错误处理：Model 层依然抛出错误，在 CommandController 的 execute 最外层捕获异常，并调用 outputService.outputError(e.what())。  
6. 目前不需要先实现日志功能，现在只需要进行格式化输出处理  
7. 需要进行自动化测试和更新SUMMARY.md文档  
