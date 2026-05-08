#ifndef COMMANDFACTORY_H
#define COMMANDFACTORY_H

#include "Command.h"
#include "CommandParser.h"
#include <vector>
#include <string>
#include <memory>

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

// CommandFactory：负责从ParsedCommand创建Command对象
class CommandFactory {
public:
    // 从ParsedCommand创建Command
    static std::unique_ptr<Command> createFromParsed(
        const ParsedCommand& parsed,
        WorkSpace* workspace,
        TextEditor* activeTextEditor);

private:
    static std::unique_ptr<Command> createEditorCommand(
        const EditorParsedCommand& ed,
        const EditorCommandContext& ctx);

    static std::unique_ptr<Command> createWorkSpaceCommand(
        const WorkSpaceParsedCommand& ws,
        WorkSpace* workspace);
};

#endif // COMMANDFACTORY_H
