#include "LogCommand.h"
#include "WorkSpace.h"
#include "OutputService.h"
#include "CommandFactory.h"
#include <iostream>

// 自注册：日志命令工厂
namespace {
    static bool _reg_logon = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Logon,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                return std::make_unique<LogonCommand>(ws.fileName.value_or(""));
            });
        return true;
    }();

    static bool _reg_logoff = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Logoff,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                return std::make_unique<LogoffCommand>(ws.fileName.value_or(""));
            });
        return true;
    }();

    static bool _reg_logshow = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Logshow,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                return std::make_unique<LogshowCommand>(ws.fileName.value_or(""));
            });
        return true;
    }();
}

// LogonCommand实现
LogonCommand::LogonCommand(const std::string& fileName) : fileName_(fileName) {}

void LogonCommand::execute() {
    checkWorkSpace();
    // 获取目标文件名（如果为空则使用当前活动文件）
    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("No active file to start logging");
        }
    }
    // 记录执行前的状态
    wasLogging_ = workspace_->isLoggingForFile(targetFile);
    // 调用工作区方法启动日志记录
    workspace_->startLoggingForFile(targetFile);
    // 保存实际文件名（用于撤销）
    fileName_ = targetFile;
    workspace_->getOutputService().outputLine("Logging started for file: " + targetFile);
}

void LogonCommand::undo() {
    checkWorkSpace();
    // 如果执行前不在记录日志，则停止日志记录
    if (!wasLogging_) {
        workspace_->stopLoggingForFile(fileName_);
        workspace_->getOutputService().outputLine("Logging stopped for file (undo): " + fileName_);
    }
    // 如果执行前已经在记录日志，则不需要做任何事情
}

bool LogonCommand::isReadOnly() const {
    return false; // 启动日志会修改状态
}

// LogoffCommand实现
LogoffCommand::LogoffCommand(const std::string& fileName) : fileName_(fileName) {}

void LogoffCommand::execute() {
    checkWorkSpace();
    // 获取目标文件名（如果为空则使用当前活动文件）
    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("No active file to stop logging");
        }
    }
    // 记录执行前的状态
    wasLogging_ = workspace_->isLoggingForFile(targetFile);
    // 调用工作区方法停止日志记录
    workspace_->stopLoggingForFile(targetFile);
    // 保存实际文件名（用于撤销）
    fileName_ = targetFile;
    workspace_->getOutputService().outputLine("Logging stopped for file: " + targetFile);
}

void LogoffCommand::undo() {
    checkWorkSpace();
    // 如果执行前在记录日志，则重新启动日志记录
    if (wasLogging_) {
        workspace_->startLoggingForFile(fileName_);
        workspace_->getOutputService().outputLine("Logging started for file (undo): " + fileName_);
    }
    // 如果执行前不在记录日志，则不需要做任何事情
}

bool LogoffCommand::isReadOnly() const {
    return false; // 停止日志会修改状态
}

// LogshowCommand实现
LogshowCommand::LogshowCommand(const std::string& fileName) : fileName_(fileName) {}

void LogshowCommand::execute() {
    checkWorkSpace();
    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("No active file to show log");
        }
    }
    workspace_->showLog(targetFile);
}

void LogshowCommand::undo() {
    // log-show是只读命令，不需要撤销操作
}

bool LogshowCommand::isReadOnly() const {
    return true; // 显示日志是只读操作
}