#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <memory>
#include <string>
#include <stdexcept>
#include <regex>
#include <vector>
#include <optional>

// 前向声明
class Command;

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

// 编辑器命令子类型枚举
enum class EditorCommandType {
    Append,
    Insert,
    Delete,
    Replace,
    Show
};

// 工作区命令子类型枚举
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
    Exit
};

// ParsedCommand：解析后的命令信息，不包含具体Command对象
// 只包含从字符串中提取的参数信息
struct ParsedCommand {
    CommandType type;

    // 对于编辑器命令
    EditorCommandType editorType;
    std::optional<int> line;      // 行号（1-based）
    std::optional<int> column;    // 列号（1-based）
    std::optional<int> length;    // 长度（用于delete/replace）
    std::optional<std::string> text; // 文本内容
    std::optional<int> startLine; // 起始行（用于show）
    std::optional<int> endLine;   // 结束行（用于show）

    // 对于工作区命令
    WorkSpaceCommandType workSpaceType;
    std::optional<std::string> fileName; // 文件名
    std::optional<std::string> target;   // 目标（用于save等）
    std::optional<std::string> path;     // 路径（用于dir-tree等）
    std::optional<bool> withLog;         // with-log选项
};

// CommandParser类：解析原始字符串命令
class CommandParser {
public:
    CommandParser() = default;
    ~CommandParser() = default;

    // 解析命令字符串，返回ParsedCommand对象
    // 参数：原始命令字符串
    // 返回值：解析后的命令信息，包含命令类型和参数
    ParsedCommand parse(const std::string& commandString);

    // 可以添加其他辅助方法，如判断命令类型等
private:
    // 辅助方法：解析编辑器命令
    ParsedCommand parseEditorCommand(const std::string& command, const std::vector<std::string>& tokens);

    // 辅助方法：解析工作区命令
    ParsedCommand parseWorkSpaceCommand(const std::string& command, const std::vector<std::string>& tokens);

    // 辅助方法：解析带引号的文本参数
    std::string parseQuotedText(const std::string& text);

    // 辅助方法：处理转义字符
    std::string processEscapeSequences(const std::string& text);

    // 辅助方法：分割命令行参数
    std::vector<std::string> splitCommandLine(const std::string& commandString);
};

#endif // COMMANDPARSER_H