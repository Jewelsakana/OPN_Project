### 当前任务  
实现WorkSpaceCommand具体类  
**要求**：  
1. 具体的WorkSpaceCommand类可以调用workspace类的实现的函数，且如果需要，可以在workspace类建立或者更新新的函数，以及根据需要可以更新CommandParser类和CommandController类   
2. 需要创建自动化测试进行测试  
**任务**：  
1. LoadCommand：
* 格式：load <file>  
* 功能：加载文件  
* 行为：如果文件存在则读取并且解析内容(读取后创建一个Editor类？)，如果文件不存在，则创建新的文件，标记为已修改，并使文件成为当前活动文件  
* 要求：可以在WorkSpace中增加文件读取逻辑 
2. saveCommand：
* 格式：save [file|all]  
* 功能：保存文件内容到磁盘上  
* 参数说明：如果不指定参数则保存当前活动文件，如果有file则保存指定文件，如果为all则保存所以已经打开的文件。  
* 行为：保存成功则消除已修改标记，如果路径无法写入则提示错误信息（抛出异常）  
* 要求：需确保 WorkSpace::fileModifiedStates_ 得到正确更新  
3. InitCommand：  
* 格式：init <file> [with-log]  
* 功能：创建一个未保存的新缓冲文件，并初始化基础结构  
* 参数：'file':创建纯文本文件，`with-log`(可选)：是否在第一行添加'# log'以启用日志  
* 说明：新缓冲区标记为已修改，需要使用'save'命令来指定路径保存，创建后应该自动成为当前活动文件  
4. CloseCommand:  
* 格式：close [file]  
* 功能：关闭当前活动文件或指定文件  
* 行为：文件已修改且未保存，则提示"文件已修改，是否保存(y/n)"，关闭后如果还有其他打开的文件则切换到最近使用的文件  
5. EditCommand：  
* 格式:edit <file>  
* 功能：切换当前活动文件  
* 行为：文件必须已在工作区内打开，如果切换失败（即没有在工作区内打开，则抛出异常"文件未打开: [file]"   
