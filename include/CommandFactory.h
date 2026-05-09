#ifndef COMMANDFACTORY_H
#define COMMANDFACTORY_H

#include "Command.h"
#include "CommandParser.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

class TextEngine;
class OutputService;
class WorkSpace;
class TextEditor;

// 编辑器命令工厂上下文
struct EditorCommandContext {
    std::vector<std::string>& lines;
    TextEngine* textEngine;
    OutputService* outputService;
};

// 工厂函数类型
using EditorCommandCreator = std::function<std::unique_ptr<Command>(const EditorParsedCommand&, const EditorCommandContext&)>;
using WorkSpaceCommandCreator = std::function<std::unique_ptr<Command>(const WorkSpaceParsedCommand&)>;

// CommandFactory：负责从ParsedCommand创建Command对象（使用注册表模式）
class CommandFactory {
public:
    // 从ParsedCommand创建Command
    static std::unique_ptr<Command> createFromParsed(
        const ParsedCommand& parsed,
        WorkSpace* workspace,
        TextEditor* activeTextEditor);

    // 注册编辑器命令工厂（由各命令类在启动时调用）
    static void registerEditorCreator(EditorCommandType type, EditorCommandCreator creator);

    // 注册工作区命令工厂（由各命令类在启动时调用）
    static void registerWorkSpaceCreator(WorkSpaceCommandType type, WorkSpaceCommandCreator creator);

private:
    static std::unique_ptr<Command> createEditorCommand(
        const EditorParsedCommand& ed,
        const EditorCommandContext& ctx);

    static std::unique_ptr<Command> createWorkSpaceCommand(
        const WorkSpaceParsedCommand& ws,
        WorkSpace* workspace);
};

#endif // COMMANDFACTORY_H