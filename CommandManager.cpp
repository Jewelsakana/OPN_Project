#include "CommandManager.h"
#include "TextEngine.h"  // 为了TextEngineException
#include <iostream>

CommandManager::CommandManager() {
}

void CommandManager::executeCommand(std::unique_ptr<Command> command) {
    if (!command) {
        return;
    }

    try {
        // 执行命令
        command->execute();

        // 执行成功，压入undo栈
        undoStack.push(std::move(command));

        // 清空redo栈（因为新的命令分支）
        while (!redoStack.empty()) {
            redoStack.pop();
        }
    } catch (const TextEngineException& e) {
        // 捕获TextEngine异常，不压入undo栈
        // 异常会向上层传播，由UI层处理
        throw;  // 重新抛出异常，让上层处理
    } catch (const std::exception& e) {
        // 捕获其他异常，同样不压入undo栈
        throw;
    }
}

void CommandManager::undo() {
    if (undoStack.empty()) {
        return;
    }

    auto command = std::move(undoStack.top());
    undoStack.pop();

    try {
        command->undo();
        // 撤销成功，压入redo栈
        redoStack.push(std::move(command));
    } catch (const std::exception& e) {
        // 撤销失败，不应该发生，但处理一下
        // 将命令放回undo栈？这里选择丢弃命令
        std::cerr << "Undo failed: " << e.what() << std::endl;
    }
}

void CommandManager::redo() {
    if (redoStack.empty()) {
        return;
    }

    auto command = std::move(redoStack.top());
    redoStack.pop();

    try {
        command->execute();
        // 重做成功，压入undo栈
        undoStack.push(std::move(command));
    } catch (const std::exception& e) {
        // 重做失败，不应该发生，但处理一下
        // 将命令放回redo栈？这里选择丢弃命令
        std::cerr << "Redo failed: " << e.what() << std::endl;
    }
}

bool CommandManager::canUndo() const {
    return !undoStack.empty();
}

bool CommandManager::canRedo() const {
    return !redoStack.empty();
}

void CommandManager::clear() {
    while (!undoStack.empty()) {
        undoStack.pop();
    }
    while (!redoStack.empty()) {
        redoStack.pop();
    }
}