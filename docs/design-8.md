### 当前任务  
实现剩余的WorkCommand具体类   
**要求**：  
1. 和design-7.md的要求相同，这是design-7.md的派生  
2. 如果有需要可以在相关类中更新或者创建函数  
3. 也需要根据具体情况更新或者创建自动化测试进行测试  
**任务**：  
1. EditorListCommand类:  
* 格式：editor-list 
* 功能：显示工作区中所有打开的文件及其状态  
* 格式：  
  `* file1.txt [modified]`  
  `file2.txt`  
  *:表示当前活动文件标价  
  [modified]：表示已修改未保存文件  
2. DirTreeCommand类：  
* 格式：dir-tree [path]  
* 功能：以树形结构显示当前工作目录(或者指定目录)的文件和文件夹  
* 参数：不指定参数时显示当前工作目录；'path'则指定要显示的目录路径  
* 说明：需要使用 `├──`、`└──` 和 `│` 字符绘制树形结构，也要显示目录和文件的层级关系  
* 建议：实现 DirTreeCommand 时可以使用 C++17 的 <filesystem> 库
3. UndoCommand类：  
* 格式：undo  
* 功能：撤销上一次编辑操作  
* 说明：只会撤销改变文件状态的命令，显示类命令(show,dir-tree,editor-list等)不进入撤销栈  
4. RedoCommand类：  
* 格式：redo  
* 功能：重做上一次撤销的操作。
注:实际上在CommandManager类已经做好了Undo栈，redo栈以及处理好了进栈的命令了，实际应该只是调用当前活动文件的Undo，Redo命令？  
5. ExitCommand类：  
* 格式：exit  
* 功能：退出编辑器程序  
* 行为：需要保存工作区状态到配置文件中，如果有未保存的文件，则逐一提示是否保存  


