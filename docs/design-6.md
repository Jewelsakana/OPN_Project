### 当前任务  
让WorkSpace类中的createCommandFromParsed和parseAndExecuteCommand与WorkSpace类分离  
即引入命令工程来进行职责分离  
**任务**  
1. 新建一个CommandController类，可以持有WorkSpace的引用或者指针  
2. 将WorkSpace类中的createCommandFromParsed和parseAndExecuteCommand函数转移到CommandController类来，在这里进行处理命令和参数绑定  
3. 将CommandParser类的使用转移到这里，从而可以使用对应的函数  
4. 将命令的分发执行转移到CommandController类中，如果若是 WorkSpace 级命令,则直接调用 cmd->execute()。进行命令执行，如果是Editor 级命令，则调用activeEditor->executeCommand(std::move(cmd))执行。  
5. 确保在创建 Editor 级命令时，从 WorkSpace 获取正确的 TextEngine 和 lines 引用。
6. CommandController也需要捕获并处理所有异常 
7. 修改自动化测试用例满足这个新类的实现