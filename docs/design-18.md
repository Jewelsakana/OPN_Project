### 当前任务    
继续进行重构  
1. 代码重复：抽取公共文件系统头文件，DirectoryService.cpp和WorkSpaceCommand.cpp中有完全相同的条件编译块，将其进行消除  
2. 同理，DirectoryService 中目录排序 lambda 在 buildDirectoryTree() 和 buildDirectoryTreeStructure() 中完全重复，抽取出私有方法  
3. 消除无所事事的类：ConfigCoordinator,删除ConfigCoordinator，让 WorkSpace 直接持有并调用 ConfigManager,减少一层无意义的间接  
4. 删除遗留声明和未使用代码：	
   * DirTreeCommand::printDirectoryTree() — 实现已移到 DirectoryService，声明是遗留  
   * Model::getName()：仅 DirectoryService 重写了，其余子类不重写，且无人调用  
   * Model::reset()：空实现，无子类重写，无调用方  
   * getOpenFileCount() / getModifiedFileCount() — 无外部调用  
5. 清理无用注释  
6. 类间亲密：命令直接访问 WorkSpace 内部服务，WorkSpace应提供统一的输出接口(如outputError、outputLine)，而不是直接暴露OutputService引用，让命令使用输出接口，真正实现 Facade 封装  
**要求**：  
1. 根据上述修改进行重构  
2. 需要重新编译通过所有的测试用例  
3. 需要清除编译出来的.o和.exe文件  
4. 将修改写入重构总结.md  