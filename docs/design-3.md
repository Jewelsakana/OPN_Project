### 当前任务    
具体的 Command 子类实现  
**目标**：实现具体的文本编辑器命令的实现，通过调用之前的实现的TextEngine类中的函数来实现  
**注意**：  
1. 要求由之前实现的CommandManager类管理并调用命令，实现Undo/redo的功能  
2. CommandManager应该等待命令执行完成后压入Undo栈中或者出现了异常抛出则不压入Undo栈中  
3. 每个Command 子类应该持有TextEngine*的指针以便调用  
**任务**：  
1. 实现InsertCommand、DeleteCommand、AppendCommand、ShowCommand、ReplaceCommand命令，这些命令内部调用写好的TextEngine类的函数，其中ReplaceCommand相当于先执行delete，再执行insert,替换文本可以为空字符串，效果等同于删除  
2. 同时ShowCommand属于只读命令，不应该进入UndoStack中  
3. Undo的数据备份：AppendCommand应该执行前记录末尾位置，undo时删除，InsertCommand执行前记录插入内容的长度，undo时执行delete,DeleteCommand执行前应该记录被删除掉的字符串的内容，否则无法Undo  
4. 生成相关的测试代码实现自动化测试，可以检测Undo/Redo是否实现  
