#ifndef COMMANDCONTROLLER_H
#define COMMANDCONTROLLER_H

#include <memory>
#include <string>
#include "Command.h"
#include "CommandParser.h"

// 前向声明
class WorkSpace;
class TextEditor;

// CommandController类：负责命令的创建、分发和执行
// 职责分离：将命令工厂和分发逻辑从WorkSpace中分离出来
class CommandController {
public:
    // 构造函数，持有WorkSpace的引用
    explicit CommandController(WorkSpace* workspace);

    // 解析并执行命令字符串
    void parseAndExecuteCommand(const std::string& commandString);

    // 从ParsedCommand创建Command对象
    std::unique_ptr<Command> createCommandFromParsed(const ParsedCommand& parsed);

    // 执行命令（分发逻辑）
    void executeCommand(std::unique_ptr<Command> command);

    // 获取关联的WorkSpace
    WorkSpace* getWorkSpace() const;

private:
    WorkSpace* workspace_;  // 指向关联的WorkSpace（不拥有所有权）

    // 从WorkSpace获取活动编辑器（转换为TextEditor）
    TextEditor* getActiveTextEditor() const;
};

#endif // COMMANDCONTROLLER_H