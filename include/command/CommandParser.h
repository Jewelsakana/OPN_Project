#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <memory>
#include <string>
#include <stdexcept>
#include <regex>
#include <vector>
#include <optional>
#include <variant>
#include <functional>

// 前向声明
class Command;
class ICommandParserStrategy;
using CommandParserStrategyPtr = std::unique_ptr<ICommandParserStrategy>;

// CommandParser解析异常基类
class CommandParseException : public std::runtime_error {
public:
    explicit CommandParseException(const std::string& message)
        : std::runtime_error(message) {}
};

// 命令格式错误异常
class CommandFormatException : public CommandParseException {
public:
    explicit CommandFormatException(const std::string& command, const std::string& expectedFormat)
        : CommandParseException("Command format error: '" + command + "'. Expected format: " + expectedFormat) {}
};

// 参数解析错误异常
class ArgumentParseException : public CommandParseException {
public:
    explicit ArgumentParseException(const std::string& argument, const std::string& reason)
        : CommandParseException("Argument parse error: '" + argument + "'. Reason: " + reason) {}
};

// 未知命令异常
class UnknownCommandException : public CommandParseException {
public:
    explicit UnknownCommandException(const std::string& command)
        : CommandParseException("Unknown command: '" + command + "'") {}
};

// 命令类型枚举
enum class CommandType {
    EditorCommand,
    WorkSpaceCommand
};

// 命令类型 ID（运行时注册，支持插件扩展）
using CommandTypeId = int;

// 编辑器命令子类型枚举（内置命令类型，插件可调用 CommandRegistry::registerType 注册新类型）
enum class EditorCommandType {
    Append,
    Insert,
    Delete,
    Replace,
    Show,
    // XML 编辑命令
    InsertBefore,
    AppendChild,
    EditId,
    EditText_,
    XmlDelete,
    XmlTree
};

// 工作区命令子类型枚举（内置命令类型）
enum class WorkSpaceCommandType {
    Load,
    Save,
    Init,
    Close,
    Edit,
    EditorList,
    DirTree,
    Undo,
    Redo,
    Exit,
    Logon,
    Logoff,
    Logshow,
    SpellCheck
};

// 命令类型注册表：支持插件在运行时注册新的命令类型
class CommandRegistry {
public:
    // 注册新的编辑器命令类型，返回分配的 ID（>= 1000 避免与内置枚举冲突）
    static CommandTypeId registerEditorType(const std::string& name);
    static CommandTypeId registerWorkSpaceType(const std::string& name);
};

// 编辑器命令解析结果
struct EditorParsedCommand {
    CommandTypeId editorType = 0;
    std::optional<int> line;
    std::optional<int> column;
    std::optional<int> length;
    std::optional<std::string> text;
    std::optional<int> startLine;
    std::optional<int> endLine;
    // XML 命令专用字段
    std::optional<std::string> tagName;
    std::optional<std::string> newId;
    std::optional<std::string> targetId;
    // 插件命令通用参数（原始 token 列表，插件自行解析）
    std::vector<std::string> args;
};

// 工作区命令解析结果
struct WorkSpaceParsedCommand {
    CommandTypeId workSpaceType = 0;
    std::optional<std::string> fileName;
    std::optional<std::string> target;
    std::optional<std::string> path;
    std::optional<bool> withLog;
};

// ParsedCommand：使用variant按命令类型分离字段
struct ParsedCommand {
    CommandType type;
    std::variant<EditorParsedCommand, WorkSpaceParsedCommand> data;

    EditorParsedCommand* asEditor() {
        return std::get_if<EditorParsedCommand>(&data);
    }
    WorkSpaceParsedCommand* asWorkSpace() {
        return std::get_if<WorkSpaceParsedCommand>(&data);
    }
    const EditorParsedCommand* asEditor() const {
        return std::get_if<EditorParsedCommand>(&data);
    }
    const WorkSpaceParsedCommand* asWorkSpace() const {
        return std::get_if<WorkSpaceParsedCommand>(&data);
    }
};

// CommandParser类：解析原始字符串命令（使用策略模式）
class CommandParser {
public:
    CommandParser();
    ~CommandParser();

    // 解析命令字符串，返回ParsedCommand对象
    ParsedCommand parse(const std::string& commandString);

    // 注册命令解析策略工厂（由 REGISTER_PARSER 宏调用，或插件手动调用）
    static void registerStrategyFactory(std::function<std::unique_ptr<ICommandParserStrategy>()> factory);

private:
    std::vector<CommandParserStrategyPtr> strategies_;

    // 收集所有自注册的策略到实例的 strategies_ 中
    void registerStrategies();

    // 分割命令行参数
    std::vector<std::string> splitCommandLine(const std::string& commandString);
};

// 解析器自注册宏：在 CommandParserStrategy.cpp 或插件 .cpp 中使用
// 用法：REGISTER_PARSER(AppendParser)
#define REGISTER_PARSER(CLASS) \
    static bool _reg_parser_##CLASS = []() { \
        CommandParser::registerStrategyFactory( \
            []() -> std::unique_ptr<ICommandParserStrategy> { return std::make_unique<CLASS>(); }); \
        return true; \
    }();

#endif // COMMANDPARSER_H