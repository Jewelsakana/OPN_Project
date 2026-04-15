**/build**  
各种编译过程中生成的目标文件的文件夹    
**/docs**  
design-系列是提示词文档,SUMMARY.md是让ai每次执行完后整个项目的总结的文件夹 
**/include**  
头文件文件夹  
**/src**  
源文件文件夹  
**/tests**  
测试文件文件夹  
**.editor_config**  
配置文件，持久化存储信息  
**Makefile**  
Make编译文件  
**Readme.md**:  
本文件  
**run_tests.ps1**:  
用于Windows中运行的测试脚本  
**text_editor.exe**:  
可运行文件  
**总结构：**
lab1-src/
├── include/                    # 头文件目录（接口定义）
│   ├── Command.h              # 命令模式抽象接口
│   ├── CommandController.h    # 命令控制器（工厂模式）
│   ├── CommandManager.h       # 命令管理器
│   ├── CommandParser.h        # 命令解析器
│   ├── DataStructures.h       # 数据结构定义（FileInfo, TreeNode等）
│   ├── DocumentManager.h      # 文档管理器
│   ├── Editor.h               # 编辑器抽象接口
│   ├── Event.h                # 事件类（观察者模式）
│   ├── FileSystemService.h    # 文件系统服务抽象
│   ├── LogCommand.h           # 日志命令类
│   ├── Logger.h               # 日志器接口
│   ├── LoggerManager.h        # 日志管理器
│   ├── Model.h                # 模型基类（统一资源管理）
│   ├── Observe.h              # 观察者接口（观察者模式）
│   ├── OutputService.h        # 输出服务
│   ├── TextCommands.h         # 文本编辑命令类
│   ├── TextEditor.h           # 文本编辑器实现
│   ├── TextEngine.h           # 文本引擎（策略模式）
│   ├── WorkSpace.h            # 工作区（协调者模式）
│   └── WorkSpaceCommand.h     # 工作区命令类
├── src/                       # 源文件目录（具体实现）
│   ├── CommandController.cpp
│   ├── CommandManager.cpp
│   ├── CommandParser.cpp
│   ├── DocumentManager.cpp
│   ├── Event.cpp
│   ├── FileSystemService.cpp
│   ├── LogCommand.cpp
│   ├── Logger.cpp
│   ├── LoggerManager.cpp
│   ├── OutputService.cpp
│   ├── TextCommands.cpp
│   ├── TextEditor.cpp
│   ├── TextEngine.cpp
│   ├── WorkSpace.cpp
│   ├── WorkSpaceCommand.cpp
│   └── main.cpp              # 程序入口
├── tests/                     # 测试目录（单元测试）
│   ├── test_commandparser.cpp   # 命令解析测试
│   ├── test_commands.cpp        # 文本命令测试
│   ├── test_documentmanager.cpp # 文档管理器测试
│   ├── test_engine.cpp          # 文本引擎测试
│   ├── test_log.cpp             # 日志功能测试
│   ├── test_log_recovery.cpp    # 日志恢复测试
│   ├── test_loggermanager.cpp   # 日志管理器测试
│   ├── test_outputservice.cpp   # 输出服务测试
│   └── test_workspace.cpp       # 工作区测试
├── docs/                      # 设计文档目录
│   ├── SUMMARY.md            # 详细实现总结
│   ├── design-1.md           # 设计文档分章节
│   └── ... (design-2.md 到 design-14.md)
├── build/                     # 构建输出目录（自动生成）
│   ├── *.o                   # 编译目标文件
│   └── test_*.exe            # 测试可执行文件
├── Makefile                   # 构建配置文件
├── run_tests.ps1             # Windows测试脚本
├── text_editor.exe           # 生成的可执行文件
└── .editor_config            # 编辑器配置文件
