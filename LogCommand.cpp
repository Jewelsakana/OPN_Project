#include "LogCommand.h"
#include "WorkSpace.h"
#include "OutputService.h"
#include <iostream>

// LogonCommand实现
LogonCommand::LogonCommand(const std::string& fileName) : fileName_(fileName) {}

void LogonCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("LogonCommand: No workspace associated");
    }
    // 获取目标文件名（如果为空则使用当前活动文件）
    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("No active file to start logging");
        }
    }
    // 调用工作区方法启动日志记录
    workspace_->startLoggingForFile(targetFile);
    workspace_->getOutputService().outputLine("Logging started for file: " + targetFile);
}

void LogonCommand::undo() {
    // TODO: 实现撤销
    // 暂时输出信息
    if (workspace_) {
        workspace_->getOutputService().outputLine("LogonCommand undo (not implemented)");
    }
}

bool LogonCommand::isReadOnly() const {
    return false; // 启动日志会修改状态
}

// LogoffCommand实现
LogoffCommand::LogoffCommand(const std::string& fileName) : fileName_(fileName) {}

void LogoffCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("LogoffCommand: No workspace associated");
    }
    // 获取目标文件名（如果为空则使用当前活动文件）
    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("No active file to stop logging");
        }
    }
    // 调用工作区方法停止日志记录
    workspace_->stopLoggingForFile(targetFile);
    workspace_->getOutputService().outputLine("Logging stopped for file: " + targetFile);
}

void LogoffCommand::undo() {
    // TODO: 实现撤销
    if (workspace_) {
        workspace_->getOutputService().outputLine("LogoffCommand undo (not implemented)");
    }
}

bool LogoffCommand::isReadOnly() const {
    return false; // 停止日志会修改状态
}

// LogshowCommand实现
LogshowCommand::LogshowCommand(const std::string& fileName) : fileName_(fileName) {}

void LogshowCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("LogshowCommand: No workspace associated");
    }
    // TODO: 实现显示日志
    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        targetFile = workspace_->getActiveFileName();
    }
    // workspace_->showLog(targetFile);

    workspace_->getOutputService().outputLine("Showing log for file: " + targetFile);
}

void LogshowCommand::undo() {
    // TODO: 实现撤销
    if (workspace_) {
        workspace_->getOutputService().outputLine("LogshowCommand undo (not implemented)");
    }
}

bool LogshowCommand::isReadOnly() const {
    return true; // 显示日志是只读操作
}