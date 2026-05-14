#include "CommandParser.h"
#include "Command.h"
#include "CommandParserStrategy.h"
#include "StringUtils.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>

CommandTypeId CommandRegistry::registerEditorType(const std::string& /*name*/) {
    static CommandTypeId nextId = 1000;
    return nextId++;
}

CommandTypeId CommandRegistry::registerWorkSpaceType(const std::string& /*name*/) {
    static CommandTypeId nextId = 2000;
    return nextId++;
}

namespace {
    using ParserFactory = std::function<std::unique_ptr<ICommandParserStrategy>()>;
    std::vector<ParserFactory>& parserRegistry() {
        static std::vector<ParserFactory> registry;
        return registry;
    }
}

CommandParser::CommandParser() {
    registerStrategies();
}

CommandParser::~CommandParser() = default;

void CommandParser::registerStrategyFactory(ParserFactory factory) {
    parserRegistry().push_back(std::move(factory));
}

void CommandParser::registerStrategies() {
    for (auto& factory : parserRegistry()) {
        strategies_.push_back(factory());
    }
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
