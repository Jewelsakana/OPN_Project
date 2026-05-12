#include "CommandParser.h"
#include "Command.h"
#include "CommandParserStrategy.h"
#include "StringUtils.h"
#include <sstream>
#include <algorithm>
#include <cctype>

CommandParser::CommandParser() {
    registerStrategies();
}

CommandParser::~CommandParser() = default;

void CommandParser::registerStrategies() {
    strategies_.push_back(std::make_unique<AppendParser>());
    strategies_.push_back(std::make_unique<InsertParser>());
    strategies_.push_back(std::make_unique<DeleteParser>());
    strategies_.push_back(std::make_unique<ReplaceParser>());
    strategies_.push_back(std::make_unique<ShowParser>());
    strategies_.push_back(std::make_unique<LoadParser>());
    strategies_.push_back(std::make_unique<SaveParser>());
    strategies_.push_back(std::make_unique<InitParser>());
    strategies_.push_back(std::make_unique<CloseParser>());
    strategies_.push_back(std::make_unique<EditParser>());
    strategies_.push_back(std::make_unique<EditorListParser>());
    strategies_.push_back(std::make_unique<DirTreeParser>());
    strategies_.push_back(std::make_unique<UndoParser>());
    strategies_.push_back(std::make_unique<RedoParser>());
    strategies_.push_back(std::make_unique<ExitParser>());
    strategies_.push_back(std::make_unique<LogonParser>());
    strategies_.push_back(std::make_unique<LogoffParser>());
    strategies_.push_back(std::make_unique<LogshowParser>());
    // XML 命令解析器
    strategies_.push_back(std::make_unique<InsertBeforeParser>());
    strategies_.push_back(std::make_unique<AppendChildParser>());
    strategies_.push_back(std::make_unique<EditIdParser>());
    strategies_.push_back(std::make_unique<EditTextParser>());
    strategies_.push_back(std::make_unique<XmlTreeParser>());
    // 拼写检查命令解析器
    strategies_.push_back(std::make_unique<SpellCheckParser>());
}

ParsedCommand CommandParser::parse(const std::string& commandString) {
    if (commandString.empty()) {
        throw CommandParseException("Empty command string");
    }

    auto tokens = splitCommandLine(commandString);
    if (tokens.empty()) {
        throw CommandParseException("No tokens found in command string");
    }

    std::string commandName = StringUtils::toLower(tokens[0]);

    for (auto& strategy : strategies_) {
        if (strategy->getCommandName() == commandName) {
            return strategy->parse(commandName, tokens);
        }
    }

    throw UnknownCommandException(commandName);
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
