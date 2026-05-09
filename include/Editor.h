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

    // Undo/Redo操作（默认空实现，子类可重写）
    virtual void undo() {}
    virtual void redo() {}
    virtual bool canUndo() const { return false; }
    virtual bool canRedo() const { return false; }
};

#endif // EDITOR_H