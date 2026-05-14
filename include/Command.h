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

    // 判断命令是否为工作区级别命令（默认false）
    // 工作区命令直接调用execute()，编辑器命令通过Editor::executeCommand()路由
    virtual bool isWorkSpaceLevel() const { return false; }

    // 设置工作区引用（由工作区命令重写，编辑器命令忽略）
    virtual void setWorkSpace(class WorkSpace* /*workspace*/) {}
};

#endif // COMMAND_H