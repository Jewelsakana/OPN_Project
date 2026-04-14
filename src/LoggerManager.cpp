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

void LoggerManager::showLog(const std::string& fileName) {
    safeExecute([this, &fileName]() {
        std::string targetFile = fileName;
        if (targetFile.empty()) {
            // 需要从WorkSpace获取当前活动文件，但LoggerManager没有直接访问
            // 这里我们暂时假设调用者已经提供了文件名
            // 在实际使用中，WorkSpace的showLog方法会处理空文件名情况
            throw std::runtime_error("LoggerManager::showLog requires a file name");
        }

        // 生成日志文件名（与Logger类一致）
        std::string logFileName = "." + targetFile + ".log";

        try {
            // 使用FileSystemService读取日志文件
            auto logLines = fileSystemService_.loadFile(logFileName);
            if (logLines.empty()) {
                // 通过WorkSpace的OutputService输出
                workspace_.getOutputService().outputLine("Log file is empty: " + logFileName);
            } else {
                workspace_.getOutputService().outputLine("Log content for file: " + targetFile);
                for (const auto& line : logLines) {
                    workspace_.getOutputService().outputLine(line);
                }
            }
        } catch (const std::exception& e) {
            // 文件不存在或其他错误
            workspace_.getOutputService().outputLine("Cannot show log for file " + targetFile + ": " + e.what());
        }
    });
}

void LoggerManager::handleException(const std::exception& e) const {
    // 可以添加日志记录器特定的异常处理，如记录到专用日志
    std::cerr << "LoggerManager error: " << e.what() << std::endl;
}