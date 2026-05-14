#ifndef COMMANDPARSERSTRATEGY_H
#define COMMANDPARSERSTRATEGY_H

#include "CommandParser.h"
#include <string>
#include <vector>
#include <memory>

// 命令解析策略接口
class ICommandParserStrategy {
public:
    virtual ~ICommandParserStrategy() = default;

    // 返回此策略处理的命令名（小写）
    virtual std::string getCommandName() const = 0;

    // 解析命令，返回ParsedCommand
    // 参数：commandName - 命令名（已转为小写），tokens - 分割后的token列表
    virtual ParsedCommand parse(const std::string& commandName,
                                const std::vector<std::string>& tokens) = 0;
};

using CommandParserStrategyPtr = std::unique_ptr<ICommandParserStrategy>;

// 辅助函数：解析带引号的文本
std::string parseQuotedTextHelper(const std::string& text);

// 辅助函数：处理转义字符
std::string processEscapeSequencesHelper(const std::string& text);

// === 编辑器命令策略 ===

class AppendParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class InsertParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class DeleteParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class ReplaceParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class ShowParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

// === 工作区命令策略 ===

class LoadParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class SaveParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class InitParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class CloseParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class EditParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class EditorListParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class DirTreeParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class UndoParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class RedoParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class ExitParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class LogonParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class LogoffParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class LogshowParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

// === XML 命令策略 ===

class InsertBeforeParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class AppendChildParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class EditIdParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class EditTextParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class SpellCheckParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

class XmlTreeParser : public ICommandParserStrategy {
public:
    std::string getCommandName() const override;
    ParsedCommand parse(const std::string& commandName,
                        const std::vector<std::string>& tokens) override;
};

#endif // COMMANDPARSERSTRATEGY_H
