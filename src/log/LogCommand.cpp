#include "LogCommand.h"
#include "WorkSpace.h"
#include "OutputService.h"
#include "CommandFactory.h"
#include <iostream>

// 自注册：日志命令工厂
namespace {
    REGISTER_WS_CMD_FILENAME(WorkSpaceCommandType::Logon, LogonCommand)
    REGISTER_WS_CMD_FILENAME(WorkSpaceCommandType::Logoff, LogoffCommand)
    REGISTER_WS_CMD_FILENAME(WorkSpaceCommandType::Logshow, LogshowCommand)
}

std::string LogCommand::resolveTargetFile(const std::string& fileName) const {
    std::string targetFile = fileName;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("No active file to " + std::string(fileName.empty() ? "perform operation" : "use"));
        }
    }
    return targetFile;
}

// LogonCommand实现
LogonCommand::LogonCommand(const std::string& fileName) : fileName_(fileName) {}

void LogonCommand::execute() {
    checkWorkSpace();
    std::string targetFile = resolveTargetFile(fileName_);
    wasLogging_ = workspace_->isLoggingForFile(targetFile);
    workspace_->startLoggingForFile(targetFile);
    fileName_ = targetFile;
    workspace_->outputLine("Logging started for file: " + targetFile);
}

void LogonCommand::undo() {
    checkWorkSpace();
    if (!wasLogging_) {
        workspace_->stopLoggingForFile(fileName_);
        workspace_->outputLine("Logging stopped for file (undo): " + fileName_);
    }
}

bool LogonCommand::isReadOnly() const {
    return false;
}

// LogoffCommand实现
LogoffCommand::LogoffCommand(const std::string& fileName) : fileName_(fileName) {}

void LogoffCommand::execute() {
    checkWorkSpace();
    std::string targetFile = resolveTargetFile(fileName_);
    wasLogging_ = workspace_->isLoggingForFile(targetFile);
    workspace_->stopLoggingForFile(targetFile);
    fileName_ = targetFile;
    workspace_->outputLine("Logging stopped for file: " + targetFile);
}

void LogoffCommand::undo() {
    checkWorkSpace();
    if (wasLogging_) {
        workspace_->startLoggingForFile(fileName_);
        workspace_->outputLine("Logging started for file (undo): " + fileName_);
    }
}

bool LogoffCommand::isReadOnly() const {
    return false;
}

// LogshowCommand实现
LogshowCommand::LogshowCommand(const std::string& fileName) : fileName_(fileName) {}

void LogshowCommand::execute() {
    checkWorkSpace();
    workspace_->showLog(resolveTargetFile(fileName_));
}

void LogshowCommand::undo() {
    // log-show是只读命令，不需要撤销操作
}

bool LogshowCommand::isReadOnly() const {
    return true; // 显示日志是只读操作
}