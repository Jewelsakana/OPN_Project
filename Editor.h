#ifndef EDITOR_H
#define EDITOR_H

#include <memory>

// 前向声明
class Command;

// Editor接口：所有编辑器的基类
class Editor {
public:
    virtual ~Editor() = default;

    // 执行命令
    virtual void executeCommand(std::unique_ptr<Command> command) = 0;

    // 可以添加其他通用的编辑方法
};

#endif // EDITOR_H