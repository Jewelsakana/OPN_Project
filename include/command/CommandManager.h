#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "Command.h"
#include <stack>
#include <memory>

// CommandManager类：作为Editor的私有组件，维护该文件独立的UndoStack和redoStack
class CommandManager {
public:
    CommandManager();
    ~CommandManager() = default;

    // 执行命令
    // 如果执行成功，将命令压入undo栈；如果失败，不压入栈
    void executeCommand(std::unique_ptr<Command> command);

    // 撤销最近执行的命令
    void undo();

    // 重做最近撤销的命令
    void redo();

    // 检查是否可以撤销
    bool canUndo() const;

    // 检查是否可以重做
    bool canRedo() const;

    // 清空所有栈（用于文件关闭或重置）
    void clear();

private:
    std::stack<std::unique_ptr<Command>> undoStack;  // 撤销栈
    std::stack<std::unique_ptr<Command>> redoStack;  // 重做栈
};

#endif // COMMANDMANAGER_H