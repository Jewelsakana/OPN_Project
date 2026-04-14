#ifndef LOGCOMMAND_H
#define LOGCOMMAND_H

#include "Command.h"
#include "WorkSpaceCommand.h"
#include <string>

// 前向声明
class WorkSpace;

// LogCommand基类：所有日志相关命令的基类
// 派生自WorkSpaceCommand，因为日志命令需要访问工作区状态
class LogCommand : public WorkSpaceCommand {
public:
    virtual ~LogCommand() = default;
};

// 启动日志记录命令
class LogonCommand : public LogCommand {
public:
    explicit LogonCommand(const std::string& fileName = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_; // 空字符串表示当前活动文件
    bool wasLogging_ = false; // 执行前是否已经在记录日志
};

// 停止日志记录命令
class LogoffCommand : public LogCommand {
public:
    explicit LogoffCommand(const std::string& fileName = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_; // 空字符串表示当前活动文件
    bool wasLogging_ = false; // 执行前是否在记录日志
};

// 显示日志记录命令
class LogshowCommand : public LogCommand {
public:
    explicit LogshowCommand(const std::string& fileName = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_; // 空字符串表示当前活动文件
};

#endif // LOGCOMMAND_H