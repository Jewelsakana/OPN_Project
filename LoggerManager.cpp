#include "LoggerManager.h"
#include "FileSystemService.h"
#include "WorkSpace.h"
#include <algorithm>
#include <iostream>

LoggerManager::LoggerManager(FileSystemService& fileSystemService, WorkSpace& workspace)
    : fileSystemService_(fileSystemService), workspace_(workspace) {
    // 构造函数可以初始化状态
}

void LoggerManager::startLoggingForFile(const std::string& fileName) {
    safeExecute([this, &fileName]() {
        if (fileLoggers_.find(fileName) != fileLoggers_.end()) {
            // 日志记录器已存在
            return;
        }
        // 创建新的日志记录器
        auto logger = std::make_shared<FileLogger>(fileName, fileSystemService_);
        fileLoggers_[fileName] = logger;
        workspace_.attach(logger);  // 委托给WorkSpace注册观察者
        // 可选：记录日志启动事件
        // Event event("log-on", fileName);
        // workspace_.notify(event);
    });
}

void LoggerManager::stopLoggingForFile(const std::string& fileName) {
    safeExecute([this, &fileName]() {
        auto it = fileLoggers_.find(fileName);
        if (it != fileLoggers_.end()) {
            workspace_.detach(it->second);  // 委托给WorkSpace注销观察者
            fileLoggers_.erase(it);
            // 可选：记录日志停止事件
            // Event event("log-off", fileName);
            // workspace_.notify(event);
        }
    });
}

bool LoggerManager::isLoggingForFile(const std::string& fileName) const {
    return fileLoggers_.find(fileName) != fileLoggers_.end();
}

std::vector<std::string> LoggerManager::getLoggedFiles() const {
    std::vector<std::string> files;
    files.reserve(fileLoggers_.size());
    for (const auto& pair : fileLoggers_) {
        files.push_back(pair.first);
    }
    return files;
}

size_t LoggerManager::getLoggerCount() const {
    return fileLoggers_.size();
}

void LoggerManager::handleException(const std::exception& e) const {
    // 可以添加日志记录器特定的异常处理，如记录到专用日志
    std::cerr << "LoggerManager error: " << e.what() << std::endl;
}