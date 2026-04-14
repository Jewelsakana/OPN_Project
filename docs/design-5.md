### 当前任务  
实现CommandParser类的实现
**要求**：  
1. CommandParser类从参数输入的原始字符串进行解析，判断是否是工作区命令还是Editor类命令，并应该建立相关的命令，输入参数，并返回命令和命令相关参数   
2. 在WorkSpace类内根据返回的格式创建命令，并执行相关命令。   
3. 如果是工作区命令则应该交给WorkSpaceCommand 的 execute的执行，否则则应该委托给当前活动编辑器执行  
4. 当前还没有实现工作区命令但可以根据命令建立相关的命令类 
5. `<file>` 表示文件路径，所有命令参数区分大小写  
6. 引入正则表达式 (Regex)来接卸命令，特别是针对 insert <line:col> "text" 这种包含多种分隔符（空格、冒号、双引号）的命令，正则表达式能更稳健地捕获参数。  
7. 处理转义字符，解析器需要支持文本内容中中的转义字符如"\n"的处理  
8. CommandParser也应该要在解析失败或者格式错误时，抛出自定义的错误  
**命令格式**：  
#### Editor类命令  
1. 追加文本：格式：append "text" 命令：AppendCommand  
2. 插入文本：格式：insert <line:col> "text" 命令：InsertCommand  
3. 删除字符：格式：delete <line:col> <len> 命令：DeleteCommand   
4. 替换文本：格式：replace <line:col> <len> "text" 命令：ReplaceCommand  
5. 显示内容：格式：show [start:end] 命令：ShowCommand  
#### WorkSpace类命令  
1. 加载文件：格式：load <file>  
2. 保存文件：格式：save [file|all]  
3. 创建新缓冲区：格式：init <file> [with-log]   
4. 关闭文件：格式：close [file]  
5. 切换活动文件：格式：edit <file>  
6. 显示文件列表：格式：editor-list   
7. 显示目录树：格式：dir-tree [path]  
8. 撤销：格式：undo  
9. 重做：格式：redo  
10. 退出程序：格式： exit 
以上命令都还没有具体实现，也无需现在实现，但需要根据其意思在WorkSpaceCommand类中全部声明  

