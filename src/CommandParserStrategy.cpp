#include "CommandParserStrategy.h"
#include "StringUtils.h"
#include <regex>
#include <stdexcept>

std::string parseQuotedTextHelper(const std::string& text) {
    if (text.length() < 2 || text.front() != '"' || text.back() != '"') {
        throw ArgumentParseException(text, "Text must be enclosed in double quotes");
    }
    std::string content = text.substr(1, text.length() - 2);
    return processEscapeSequencesHelper(content);
}

std::string processEscapeSequencesHelper(const std::string& text) {
    std::string result;
    result.reserve(text.length());

    for (size_t i = 0; i < text.length(); ++i) {
        if (text[i] == '\\' && i + 1 < text.length()) {
            switch (text[i + 1]) {
                case 'n':  result += '\n'; ++i; break;
                case 't':  result += '\t'; ++i; break;
                case 'r':  result += '\r'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case '\"': result += '\"'; ++i; break;
                default:   result += text[i]; break;
            }
        } else {
            result += text[i];
        }
    }

    return result;
}

namespace {
    // 解析 "line:col" 格式
    std::pair<int, int> parsePosition(const std::string& position) {
        std::regex positionPattern(R"((\d+):(\d+))");
        std::smatch match;
        if (!std::regex_match(position, match, positionPattern)) {
            throw ArgumentParseException(position, "Expected format: <line:col>");
        }
        return {std::stoi(match[1]), std::stoi(match[2])};
    }

    // 创建编辑器命令的 ParsedCommand
    ParsedCommand makeEditor(EditorCommandType type) {
        ParsedCommand p;
        p.type = CommandType::EditorCommand;
        p.data = EditorParsedCommand{};
        p.asEditor()->editorType = type;
        return p;
    }

    // 创建工作区命令的 ParsedCommand
    ParsedCommand makeWorkSpace(WorkSpaceCommandType type) {
        ParsedCommand p;
        p.type = CommandType::WorkSpaceCommand;
        p.data = WorkSpaceParsedCommand{};
        p.asWorkSpace()->workSpaceType = type;
        return p;
    }
}

std::string AppendParser::getCommandName() const { return "append"; }

ParsedCommand AppendParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        throw CommandFormatException("append", "append \"text\"");
    }
    auto parsed = makeEditor(EditorCommandType::Append);
    parsed.asEditor()->text = parseQuotedTextHelper(tokens[1]);
    return parsed;
}

std::string InsertParser::getCommandName() const { return "insert"; }

ParsedCommand InsertParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        throw CommandFormatException("insert", "insert <line:col> \"text\"");
    }
    auto [line, col] = parsePosition(tokens[1]);
    auto parsed = makeEditor(EditorCommandType::Insert);
    parsed.asEditor()->line = line;
    parsed.asEditor()->column = col;
    parsed.asEditor()->text = parseQuotedTextHelper(tokens[2]);
    return parsed;
}

std::string DeleteParser::getCommandName() const { return "delete"; }

ParsedCommand DeleteParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    // 区分 XML 删除（delete <elementId>）和文本删除（delete <line:col> <len>）
    if (tokens.size() == 2 && tokens[1].find(':') == std::string::npos) {
        auto parsed = makeEditor(EditorCommandType::XmlDelete);
        parsed.asEditor()->targetId = tokens[1];
        return parsed;
    }
    if (tokens.size() < 3) {
        throw CommandFormatException("delete", "delete <line:col> <len>  or  delete <elementId>");
    }
    auto [line, col] = parsePosition(tokens[1]);
    int length = std::stoi(tokens[2]);
    if (length <= 0) {
        throw ArgumentParseException(tokens[2], "Delete length must be positive");
    }
    auto parsed = makeEditor(EditorCommandType::Delete);
    parsed.asEditor()->line = line;
    parsed.asEditor()->column = col;
    parsed.asEditor()->length = length;
    return parsed;
}

std::string ReplaceParser::getCommandName() const { return "replace"; }

ParsedCommand ReplaceParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw CommandFormatException("replace", "replace <line:col> <len> \"text\"");
    }
    auto [line, col] = parsePosition(tokens[1]);
    int length = std::stoi(tokens[2]);
    if (length < 0) {
        throw ArgumentParseException(tokens[2], "Delete length cannot be negative");
    }
    auto parsed = makeEditor(EditorCommandType::Replace);
    parsed.asEditor()->line = line;
    parsed.asEditor()->column = col;
    parsed.asEditor()->length = length;
    parsed.asEditor()->text = parseQuotedTextHelper(tokens[3]);
    return parsed;
}

std::string ShowParser::getCommandName() const { return "show"; }

ParsedCommand ShowParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeEditor(EditorCommandType::Show);
    if (tokens.size() > 1) {
        auto [start, end] = parsePosition(tokens[1]);
        parsed.asEditor()->startLine = start;
        parsed.asEditor()->endLine = end;
    }
    return parsed;
}

std::string LoadParser::getCommandName() const { return "load"; }

ParsedCommand LoadParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        throw CommandFormatException("load", "load <file>");
    }
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Load);
    parsed.asWorkSpace()->fileName = tokens[1];
    return parsed;
}

std::string SaveParser::getCommandName() const { return "save"; }

ParsedCommand SaveParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Save);
    if (tokens.size() > 1) {
        parsed.asWorkSpace()->target = tokens[1];
    }
    return parsed;
}

std::string InitParser::getCommandName() const { return "init"; }

ParsedCommand InitParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        throw CommandFormatException("init", "init <file> [with-log]");
    }
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Init);
    parsed.asWorkSpace()->fileName = tokens[1];
    if (tokens.size() > 2) {
        if (StringUtils::toLower(tokens[2]) != "with-log") {
            throw ArgumentParseException(tokens[2], "Expected 'with-log' or nothing");
        }
        parsed.asWorkSpace()->withLog = true;
    } else {
        parsed.asWorkSpace()->withLog = false;
    }
    return parsed;
}

std::string CloseParser::getCommandName() const { return "close"; }

ParsedCommand CloseParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Close);
    if (tokens.size() > 1) {
        parsed.asWorkSpace()->fileName = tokens[1];
    }
    return parsed;
}

std::string EditParser::getCommandName() const { return "edit"; }

ParsedCommand EditParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        throw CommandFormatException("edit", "edit <file>");
    }
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Edit);
    parsed.asWorkSpace()->fileName = tokens[1];
    return parsed;
}

std::string EditorListParser::getCommandName() const { return "editor-list"; }

ParsedCommand EditorListParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::EditorList);
    if (tokens.size() > 1) {
        if (StringUtils::toLower(tokens[1]) != "tree") {
            throw ArgumentParseException(tokens[1], "Expected 'tree' or nothing");
        }
        parsed.asWorkSpace()->target = tokens[1];  // "tree"
    }
    return parsed;
}

std::string DirTreeParser::getCommandName() const { return "dir-tree"; }

ParsedCommand DirTreeParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::DirTree);
    if (tokens.size() > 1) {
        parsed.asWorkSpace()->path = tokens[1];
    }
    return parsed;
}

std::string UndoParser::getCommandName() const { return "undo"; }

ParsedCommand UndoParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() != 1) {
        throw CommandFormatException("undo", "undo");
    }
    return makeWorkSpace(WorkSpaceCommandType::Undo);
}

std::string RedoParser::getCommandName() const { return "redo"; }

ParsedCommand RedoParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() != 1) {
        throw CommandFormatException("redo", "redo");
    }
    return makeWorkSpace(WorkSpaceCommandType::Redo);
}

std::string ExitParser::getCommandName() const { return "exit"; }

ParsedCommand ExitParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() != 1) {
        throw CommandFormatException("exit", "exit");
    }
    return makeWorkSpace(WorkSpaceCommandType::Exit);
}

std::string LogonParser::getCommandName() const { return "log-on"; }

ParsedCommand LogonParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Logon);
    if (tokens.size() > 1) {
        parsed.asWorkSpace()->fileName = tokens[1];
    }
    return parsed;
}

std::string LogoffParser::getCommandName() const { return "log-off"; }

ParsedCommand LogoffParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Logoff);
    if (tokens.size() > 1) {
        parsed.asWorkSpace()->fileName = tokens[1];
    }
    return parsed;
}

std::string LogshowParser::getCommandName() const { return "log-show"; }

ParsedCommand LogshowParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeWorkSpace(WorkSpaceCommandType::Logshow);
    if (tokens.size() > 1) {
        parsed.asWorkSpace()->fileName = tokens[1];
    }
    return parsed;
}

// === XML 命令策略实现 ===

std::string InsertBeforeParser::getCommandName() const { return "insert-before"; }

ParsedCommand InsertBeforeParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw CommandFormatException("insert-before", "insert-before <tagName> <newId> <targetId> [\"text\"]");
    }
    auto parsed = makeEditor(EditorCommandType::InsertBefore);
    parsed.asEditor()->tagName = tokens[1];
    parsed.asEditor()->newId = tokens[2];
    parsed.asEditor()->targetId = tokens[3];
    if (tokens.size() > 4) {
        parsed.asEditor()->text = parseQuotedTextHelper(tokens[4]);
    }
    return parsed;
}

std::string AppendChildParser::getCommandName() const { return "append-child"; }

ParsedCommand AppendChildParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw CommandFormatException("append-child", "append-child <tagName> <newId> <parentId> [\"text\"]");
    }
    auto parsed = makeEditor(EditorCommandType::AppendChild);
    parsed.asEditor()->tagName = tokens[1];
    parsed.asEditor()->newId = tokens[2];
    parsed.asEditor()->targetId = tokens[3];
    if (tokens.size() > 4) {
        parsed.asEditor()->text = parseQuotedTextHelper(tokens[4]);
    }
    return parsed;
}

std::string EditIdParser::getCommandName() const { return "edit-id"; }

ParsedCommand EditIdParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        throw CommandFormatException("edit-id", "edit-id <oldId> <newId>");
    }
    auto parsed = makeEditor(EditorCommandType::EditId);
    parsed.asEditor()->targetId = tokens[1];  // oldId
    parsed.asEditor()->newId = tokens[2];      // newId
    return parsed;
}

std::string EditTextParser::getCommandName() const { return "edit-text"; }

ParsedCommand EditTextParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        throw CommandFormatException("edit-text", "edit-text <elementId> [\"text\"]");
    }
    auto parsed = makeEditor(EditorCommandType::EditText_);
    parsed.asEditor()->targetId = tokens[1];  // elementId
    if (tokens.size() > 2) {
        parsed.asEditor()->text = parseQuotedTextHelper(tokens[2]);
    }
    return parsed;
}

std::string XmlTreeParser::getCommandName() const { return "xml-tree"; }

ParsedCommand XmlTreeParser::parse(const std::string&, const std::vector<std::string>& tokens) {
    auto parsed = makeEditor(EditorCommandType::XmlTree);
    if (tokens.size() > 1) {
        parsed.asEditor()->text = tokens[1];  // optional file parameter
    }
    return parsed;
}

