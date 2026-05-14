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
class XmlEditor;

// 编辑器命令工厂上下文
struct EditorCommandContext {
    std::vector<std::string>* lines = nullptr;  // 仅文本命令使用
    TextEngine* textEngine = nullptr;           // 仅文本命令使用
    OutputService* outputService = nullptr;
    XmlEditor* xmlEditor = nullptr;             // 仅XML命令使用
    void* pluginContext = nullptr;              // 插件编辑器自定义上下文
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
        WorkSpace* workspace);

    // 注册编辑器命令工厂（由各命令类在启动时调用，type 支持插件运行时注册的 CommandTypeId）
    static void registerEditorCreator(CommandTypeId type, EditorCommandCreator creator);

    // 注册工作区命令工厂（由各命令类在启动时调用，type 支持插件运行时注册的 CommandTypeId）
    static void registerWorkSpaceCreator(CommandTypeId type, WorkSpaceCommandCreator creator);

private:
    static std::unique_ptr<Command> createEditorCommand(
        const EditorParsedCommand& ed,
        const EditorCommandContext& ctx);

    static std::unique_ptr<Command> createWorkSpaceCommand(
        const WorkSpaceParsedCommand& ws,
        WorkSpace* workspace);
};

// 自注册宏：统一命令的注册模式

// 编辑器命令（带守卫条件，ctx.lines + ctx.textEngine 固定前缀）
#define REGISTER_EDITOR_CMD_GUARDED(ENUM, CLASS, GUARD, ...) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerEditorCreator(static_cast<CommandTypeId>(ENUM), \
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> { \
                if ((GUARD) && ctx.lines) return std::make_unique<CLASS>(*ctx.lines, ctx.textEngine, ##__VA_ARGS__); \
                return nullptr; \
            }); \
        return true; \
    }();

// ShowCommand 专用（需要 ctx.outputService，有两个分支）
#define REGISTER_EDITOR_CMD_SHOW(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerEditorCreator(static_cast<CommandTypeId>(ENUM), \
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> { \
                if (ctx.lines) { \
                    if (ed.startLine && ed.endLine) \
                        return std::make_unique<CLASS>(*ctx.lines, ctx.textEngine, ctx.outputService, *ed.startLine, *ed.endLine); \
                    else \
                        return std::make_unique<CLASS>(*ctx.lines, ctx.textEngine, ctx.outputService); \
                } \
                return nullptr; \
            }); \
        return true; \
    }();

// 工作区命令

#define REGISTER_WS_CMD_NOARGS(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerWorkSpaceCreator(static_cast<CommandTypeId>(ENUM), \
            [](const WorkSpaceParsedCommand&) -> std::unique_ptr<Command> { \
                return std::make_unique<CLASS>(); \
            }); \
        return true; \
    }();

#define REGISTER_WS_CMD_FILENAME(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerWorkSpaceCreator(static_cast<CommandTypeId>(ENUM), \
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> { \
                return std::make_unique<CLASS>(ws.fileName.value_or("")); \
            }); \
        return true; \
    }();

#define REGISTER_WS_CMD_REQ_FILENAME(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerWorkSpaceCreator(static_cast<CommandTypeId>(ENUM), \
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> { \
                if (ws.fileName) return std::make_unique<CLASS>(*ws.fileName); \
                return nullptr; \
            }); \
        return true; \
    }();

#define REGISTER_WS_CMD_PATH(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerWorkSpaceCreator(static_cast<CommandTypeId>(ENUM), \
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> { \
                return std::make_unique<CLASS>(ws.path.value_or("")); \
            }); \
        return true; \
    }();

#define REGISTER_WS_CMD_TARGET(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerWorkSpaceCreator(static_cast<CommandTypeId>(ENUM), \
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> { \
                return std::make_unique<CLASS>(ws.target.value_or("")); \
            }); \
        return true; \
    }();

#define REGISTER_WS_CMD_INIT(ENUM, CLASS) \
    static bool _reg_##CLASS = []() { \
        CommandFactory::registerWorkSpaceCreator(static_cast<CommandTypeId>(ENUM), \
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> { \
                if (ws.fileName) return std::make_unique<CLASS>(*ws.fileName, ws.withLog.value_or(false)); \
                return nullptr; \
            }); \
        return true; \
    }();

// 插件编辑器命令注册宏（通用，使用 ctx.pluginContext 和 ed.args）
#define REGISTER_PLUGIN_CMD(TYPE_ID, CLASS) \
    static bool _reg_plugin_##CLASS = []() { \
        CommandFactory::registerEditorCreator(TYPE_ID, \
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> { \
                if (ctx.pluginContext) return std::make_unique<CLASS>(ctx.pluginContext, ed.args); \
                return nullptr; \
            }); \
        return true; \
    }();

// XML 编辑器命令注册宏
#define REGISTER_XML_CMD(ENUM, CLASS, GUARD, ...) \
    static bool _reg_xml_##CLASS = []() { \
        CommandFactory::registerEditorCreator(static_cast<CommandTypeId>(ENUM), \
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> { \
                if ((GUARD) && ctx.xmlEditor) return std::make_unique<CLASS>(&ctx.xmlEditor->getDocument(), ##__VA_ARGS__); \
                return nullptr; \
            }); \
        return true; \
    }();

#endif // COMMANDFACTORY_H