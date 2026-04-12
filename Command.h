#ifndef COMMAND_H
#define COMMAND_H

// Command接口：用于实现命令模式，包含execute()和Undo的方法
class Command {
public:
    virtual ~Command() = default;

    // 执行命令
    virtual void execute() = 0;

    // 撤销命令
    virtual void undo() = 0;

    // 判断命令是否为只读命令（默认返回false）
    // 只读命令执行后不会进入Undo栈
    virtual bool isReadOnly() const { return false; }
};

#endif // COMMAND_H