**角色**：你是一个熟悉面向对象技术的程序员；  
**任务**：你需要实现一个基于命令行的文本编辑器，支持同时打开多个文本文件，提供工作区管理，日志记录，状态持久化功能  
**编程语言**：C++
先实现以下任务，严格按照我的规划进行处理，之后的任务我会逐一说明让你实现  
任务：  
1. 定义Command接口：用于实现命令模式，包含execute()和Undo的方法  
2. 定义Observe接口：用于实现观察者模式，定义Update(Event e)，以便日志模块监听（日志模块先不实现）  
3. 定义事件模型Event：封装执行时间戳，命令内容和目标文件名，用于日志记录  
4. 定义Editor接口：只是用来作为基类  
5. 定于TextEditor类:使用行数组(`List<String>`)存储文本，每个元素是一行，需要标记 modified 状态。它需要调用TextEngine进行逻辑计算并在内部更新自己的行数组  
6. 定义TextEngine类：作为实现复杂的字符串算法的类，不持有数据，仅负责算法,对TextEditor采用只读暴露+结果应用，即TextEditor接收TextEngine计算的结果并在TextEditor内部更新自己(算法包含字符串的append,insert,delete,show，现在不进行实现)  
7. 定义CommandManager类：作为Editor的私有组件，维护该文件独立的UndoStack和redoStack，负责 execute()、undo() 和 redo() 的逻辑调度。并且Editor会将命令委托给CommandManager类进行处理，让CommandManager调用命令，在执行命令失败时，不应该将命令压入UndoStack     
8. 当TextEngine检测到非法操作时，需要抛出自定义异常，当CommandManager必须捕获到该异常以确保失败的操作不会进入Undo栈，并最终由Ui层(暂时不实现)统一捕获并向用户显示错误信息，而不导致程序崩溃  
现在先实现这些模块，之后会交给你其他任务  

