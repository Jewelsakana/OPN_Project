#include "CommandParser.h"
#include "Command.h"
#include "TextCommands.h"
#include "WorkSpaceCommand.h"
#include <sstream>
#include <algorithm>
#include <cctype>

// 辅助函数：去除字符串首尾空格
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

// 辅助函数：检查字符串是否以特定前缀开头
static bool startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) return false;
    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

// 辅助函数：将字符串转换为小写
static std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

ParsedCommand CommandParser::parse(const std::string& commandString) {
    if (commandString.empty()) {
        throw CommandParseException("Empty command string");
    }

    // 分割命令行参数
    auto tokens = splitCommandLine(commandString);
    if (tokens.empty()) {
        throw CommandParseException("No tokens found in command string");
    }

    // 获取命令名（转换为小写，不区分大小写）
    std::string commandName = toLower(tokens[0]);

    // 判断命令类型
    if (commandName == "load" || commandName == "save" || commandName == "init" ||
        commandName == "close" || commandName == "edit" || commandName == "editor-list" ||
        commandName == "dir-tree" || commandName == "undo" || commandName == "redo" ||
        commandName == "exit" || commandName == "log-on" || commandName == "log-off" || commandName == "log-show") {
        return parseWorkSpaceCommand(commandName, tokens);
    } else if (commandName == "append" || commandName == "insert" || commandName == "delete" ||
               commandName == "replace" || commandName == "show") {
        return parseEditorCommand(commandName, tokens);
    } else {
        throw UnknownCommandException(commandName);
    }
}

ParsedCommand CommandParser::parseEditorCommand(const std::string& command, const std::vector<std::string>& tokens) {
    ParsedCommand parsed;
    parsed.type = CommandType::EditorCommand;

    if (command == "append") {
        if (tokens.size() < 2) {
            throw CommandFormatException("append", "append \"text\"");
        }
        // 解析引号内的文本
        std::string text = parseQuotedText(tokens[1]);

        parsed.editorType = EditorCommandType::Append;
        parsed.text = text;
        return parsed;
    }
    else if (command == "insert") {
        if (tokens.size() < 3) {
            throw CommandFormatException("insert", "insert <line:col> \"text\"");
        }

        // 解析行:列格式
        std::string position = tokens[1];
        std::regex positionPattern(R"((\d+):(\d+))");
        std::smatch match;

        if (!std::regex_match(position, match, positionPattern)) {
            throw ArgumentParseException(position, "Expected format: <line:col>");
        }

        int line = std::stoi(match[1]);
        int col = std::stoi(match[2]);

        // 解析文本
        std::string text = parseQuotedText(tokens[2]);

        parsed.editorType = EditorCommandType::Insert;
        parsed.line = line;
        parsed.column = col;
        parsed.text = text;
        return parsed;
    }
    else if (command == "delete") {
        if (tokens.size() < 3) {
            throw CommandFormatException("delete", "delete <line:col> <len>");
        }

        // 解析行:列格式
        std::string position = tokens[1];
        std::regex positionPattern(R"((\d+):(\d+))");
        std::smatch match;

        if (!std::regex_match(position, match, positionPattern)) {
            throw ArgumentParseException(position, "Expected format: <line:col>");
        }

        int line = std::stoi(match[1]);
        int col = std::stoi(match[2]);

        // 解析长度
        int length = std::stoi(tokens[2]);
        if (length <= 0) {
            throw ArgumentParseException(tokens[2], "Delete length must be positive");
        }

        parsed.editorType = EditorCommandType::Delete;
        parsed.line = line;
        parsed.column = col;
        parsed.length = length;
        return parsed;
    }
    else if (command == "replace") {
        if (tokens.size() < 4) {
            throw CommandFormatException("replace", "replace <line:col> <len> \"text\"");
        }

        // 解析行:列格式
        std::string position = tokens[1];
        std::regex positionPattern(R"((\d+):(\d+))");
        std::smatch match;

        if (!std::regex_match(position, match, positionPattern)) {
            throw ArgumentParseException(position, "Expected format: <line:col>");
        }

        int line = std::stoi(match[1]);
        int col = std::stoi(match[2]);

        // 解析长度
        int length = std::stoi(tokens[2]);
        if (length < 0) {
            throw ArgumentParseException(tokens[2], "Delete length cannot be negative");
        }

        // 解析替换文本
        std::string text = parseQuotedText(tokens[3]);

        parsed.editorType = EditorCommandType::Replace;
        parsed.line = line;
        parsed.column = col;
        parsed.length = length;
        parsed.text = text;
        return parsed;
    }
    else if (command == "show") {
        int startLine = 0;
        int endLine = -1;

        if (tokens.size() > 1) {
            std::string range = tokens[1];
            std::regex rangePattern(R"((\d+):(\d+))");
            std::smatch match;

            if (std::regex_match(range, match, rangePattern)) {
                startLine = std::stoi(match[1]);
                endLine = std::stoi(match[2]);
            } else {
                throw ArgumentParseException(range, "Expected format: [start:end]");
            }
        }

        parsed.editorType = EditorCommandType::Show;
        parsed.startLine = startLine;
        parsed.endLine = endLine;
        return parsed;
    }

    // 不应该到达这里
    throw UnknownCommandException(command);
}

ParsedCommand CommandParser::parseWorkSpaceCommand(const std::string& command, const std::vector<std::string>& tokens) {
    ParsedCommand parsed;
    parsed.type = CommandType::WorkSpaceCommand;

    if (command == "load") {
        if (tokens.size() < 2) {
            throw CommandFormatException("load", "load <file>");
        }
        parsed.workSpaceType = WorkSpaceCommandType::Load;
        parsed.fileName = tokens[1];
        return parsed;
    }
    else if (command == "save") {
        parsed.workSpaceType = WorkSpaceCommandType::Save;
        if (tokens.size() > 1) {
            parsed.target = tokens[1];
        }
        return parsed;
    }
    else if (command == "init") {
        if (tokens.size() < 2) {
            throw CommandFormatException("init", "init <file> [with-log]");
        }
        parsed.workSpaceType = WorkSpaceCommandType::Init;
        parsed.fileName = tokens[1];
        if (tokens.size() > 2) {
            if (toLower(tokens[2]) != "with-log") {
                throw ArgumentParseException(tokens[2], "Expected 'with-log' or nothing");
            }
            parsed.withLog = true;
        } else {
            parsed.withLog = false;
        }
        return parsed;
    }
    else if (command == "close") {
        parsed.workSpaceType = WorkSpaceCommandType::Close;
        if (tokens.size() > 1) {
            parsed.fileName = tokens[1];
        }
        return parsed;
    }
    else if (command == "edit") {
        if (tokens.size() < 2) {
            throw CommandFormatException("edit", "edit <file>");
        }
        parsed.workSpaceType = WorkSpaceCommandType::Edit;
        parsed.fileName = tokens[1];
        return parsed;
    }
    else if (command == "editor-list") {
        if (tokens.size() != 1) {
            throw CommandFormatException("editor-list", "editor-list");
        }
        parsed.workSpaceType = WorkSpaceCommandType::EditorList;
        return parsed;
    }
    else if (command == "dir-tree") {
        parsed.workSpaceType = WorkSpaceCommandType::DirTree;
        if (tokens.size() > 1) {
            parsed.path = tokens[1];
        }
        return parsed;
    }
    else if (command == "undo") {
        if (tokens.size() != 1) {
            throw CommandFormatException("undo", "undo");
        }
        parsed.workSpaceType = WorkSpaceCommandType::Undo;
        return parsed;
    }
    else if (command == "redo") {
        if (tokens.size() != 1) {
            throw CommandFormatException("redo", "redo");
        }
        parsed.workSpaceType = WorkSpaceCommandType::Redo;
        return parsed;
    }
    else if (command == "exit") {
        if (tokens.size() != 1) {
            throw CommandFormatException("exit", "exit");
        }
        parsed.workSpaceType = WorkSpaceCommandType::Exit;
        return parsed;
    }
    else if (command == "log-on") {
        parsed.workSpaceType = WorkSpaceCommandType::Logon;
        if (tokens.size() > 1) {
            parsed.fileName = tokens[1];
        }
        return parsed;
    }
    else if (command == "log-off") {
        parsed.workSpaceType = WorkSpaceCommandType::Logoff;
        if (tokens.size() > 1) {
            parsed.fileName = tokens[1];
        }
        return parsed;
    }
    else if (command == "log-show") {
        parsed.workSpaceType = WorkSpaceCommandType::Logshow;
        if (tokens.size() > 1) {
            parsed.fileName = tokens[1];
        }
        return parsed;
    }

    // 不应该到达这里
    throw UnknownCommandException(command);
}

std::string CommandParser::parseQuotedText(const std::string& text) {
    if (text.length() < 2 || text.front() != '"' || text.back() != '"') {
        throw ArgumentParseException(text, "Text must be enclosed in double quotes");
    }

    // 提取引号内的内容
    std::string content = text.substr(1, text.length() - 2);

    // 处理转义字符
    return processEscapeSequences(content);
}

std::string CommandParser::processEscapeSequences(const std::string& text) {
    std::string result;
    result.reserve(text.length());

    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == '\\' && i + 1 < text.length()) {
            switch (text[i + 1]) {
                case 'n':
                    result += '\n';
                    ++i;
                    break;
                case 't':
                    result += '\t';
                    ++i;
                    break;
                case 'r':
                    result += '\r';
                    ++i;
                    break;
                case '\\':
                    result += '\\';
                    ++i;
                    break;
                case '\"':
                    result += '\"';
                    ++i;
                    break;
                default:
                    // 如果不是有效的转义序列，保持原样
                    result += text[i];
                    break;
            }
        } else {
            result += text[i];
        }
    }

    return result;
}

std::vector<std::string> CommandParser::splitCommandLine(const std::string& commandString) {
    std::vector<std::string> tokens;
    std::string token;
    bool inQuotes = false;
    bool escapeNext = false;

    for (char c : commandString) {
        if (escapeNext) {
            token += c;
            escapeNext = false;
        } else if (c == '\\') {
            escapeNext = true;
        } else if (c == '"') {
            inQuotes = !inQuotes;
            token += c;
        } else if (c == ' ' && !inQuotes) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }

    if (!token.empty()) {
        tokens.push_back(token);
    }

    return tokens;
}