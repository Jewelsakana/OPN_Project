### 当前任务  
进一步拆分WorkSpace上完成的功能和日志命令的具体实现    
**任务**：  
1. 创建独立的LoggerManager类,将文件日志管理模块单独拆分为一个类，将startLoggingForFile、stopLoggingForFile、isLoggingForFile等方法，还有fileLoggers_成员变量迁移到新的LoggerManager类中实现（已完成）  
2. 在LogCommand类实现具体的日志命令：  
   1. log-on：  
    * 格式：log-on [file]  
    * 功能：为指定文件（或者当前活动文件）启用日志记录  
    * 参数：不指定参数则为当前活动文件启用日志，'file'则是为指定文件启用日志  
    * 行为：后续该文件的所有的编辑操作，保存行为都被记录到 `.filename.log` 文件中，如果文件首行是#log，可自动启用  
   2. log-off：
    * 格式：log-off [file]  
    * 功能：关闭指定文件（或者当前活动文件）的日志记录  
    * 参数：不指定参数则关闭当前活动文件日志，'file'则是关闭指定文件的日志  
    * 行为：只是停止监听该文件的日志事件，但不删除已有日志  
   3. log-show：  
    * 格式：log-show [file]  
    * 功能：显示指定文件(或当前活动文件)的日志记录  
    * 参数说明：不指定参数则显示当前活动文件的日志，'file'则显示指定文件的日志  
    * 输出格式：显示filename.log文件的内容。使用 OutputService类中的函数进行输出  
3. 更新自动化测试用例并且更新SUMMARY.md  
