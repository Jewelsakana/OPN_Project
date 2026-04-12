### 当前任务  
实现工作区(WorkSpace)模块  
**要求**  
1. 管理当前会话的全局状态，包括已经打开的文件列表，当前活动文件，文件修改状态  
2. 工作区中可以由多个编辑中的Editor，有一个当前的活动文件(Active Editior)  
3. 应该使用map类来管理存储打开的文件  
**任务**：
1. 使用备忘录模式，定义 WorkspaceMemento 类，用于工作区状态的持久化和恢复  
2. 持久化保持的内容：打开的文件列表，当前活动文件，文件修改的状态，日志开关状态，不需要持久化undo/redo的历史记录  
3. 接入观察者接口，引入之前建立的观察者接口，维护一个观察者列表，使用组合方式实现。在 WorkSpace 类中维护一个 std::vector<std::shared_ptr<Observer>>，并提供 attach, detach, notify 方法。 
4. 定义一个从Command类派生出的WorkSpaceCommand类，用于后续的工作区指令的实现（暂时不实现具体代码，只定义）  
5. 定义一个CommandParser类，参数为原始的字符串，返回的应该是Command类用于解析是WorkSpace命令还是Editor命令，将其解析为具体的Command对象返回workspace类，让workspace来判断是工作区命令还是Editor类命令。具体解析暂时不进行实现，只定义  
6. 如果是Editor类命令，WorkSpace应该调用activeEditor->executeCommand(cmd)，传递命令。