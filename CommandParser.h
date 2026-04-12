#ifndef COMMANDPARSER_H
#define COMMANDPARSER_H

#include <memory>
#include <string>

// 前向声明
class Command;

// CommandParser类：解析原始字符串命令
class CommandParser {
public:
    CommandParser() = default;
    ~CommandParser() = default;

    // 解析命令字符串，返回Command对象
    // 参数：原始命令字符串
    // 返回值：Command对象的unique_ptr，如果解析失败返回nullptr
    std::unique_ptr<Command> parse(const std::string& commandString);

    // 可以添加其他辅助方法，如判断命令类型等
};

#endif // COMMANDPARSER_H