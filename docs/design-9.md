### 当前任务   
拆分事务逻辑
**任务**：  
1. 需要新建一个Model基类用于之后的Model类派生，提供基础的错误处理接口    
2. 拆分WorkSpace类中的loadFile(fileName)，saveFile(fileName)，DirTreeCommand 中的递归遍历函数 printDirectoryTree到新 Model 类职责：FileSystemService  
3. 拆分WorkSpace类中的isFileModified(fileName)，setFileModified(...)，openFiles_ 字典的维护，activeFileName_ 的切换逻辑到新的Model类职责：DocumentManager用于管理文件状态和编辑器映射。  
4. 让workspace类当协调员，它调用 FileSystemService 读写文件，然后把数据塞给 TextEditor  
5. 将WorkCommand类中写出的一大堆 std::filesystem 的遍历逻辑转移到FileSystemService类来实现，此后则调用FileSystemService类函数实现具体的事务  
6. 修改WorkCommand类中重构后的Command类，以便调用新的Model类函数进行事务处理  
如：LoadCommand，SaveCommand，DirTreeCommand，ExitCommand等  
7. 修改TextEngine类也让其作为Model类的派生  
8. 实现修改后的自动化测试，并更新SUMMARY.md文档  

