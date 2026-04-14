#ifndef LOGGERMANAGER_H
#define LOGGERMANAGER_H

#include "Model.h"
#include "Observe.h"
#include "Logger.h"
#include <string>
#include <memory>
#include <map>
#include <vector>

// 前向声明
class FileSystemService;
class WorkSpace;

// LoggerManager类：负责文件日志记录器的生命周期管理
// 继承自Model基类，提供统一的错误处理接口
class LoggerManager : public Model {
public:
    // 构造函数，接受FileSystemService和WorkSpace的引用
    LoggerManager(FileSystemService& fileSystemService, WorkSpace& workspace);
    ~LoggerManager() override = default;

    // 获取实例名称（重写Model基类方法）
    std::string getName() const override {
        return "LoggerManager";
    }

    // 文件日志管理
    void startLoggingForFile(const std::string& fileName);
    void stopLoggingForFile(const std::string& fileName);
    bool isLoggingForFile(const std::string& fileName) const;
    void showLog(const std::string& fileName);

    // 获取所有正在记录日志的文件
    std::vector<std::string> getLoggedFiles() const;

    // 获取日志记录器数量（用于测试）
    size_t getLoggerCount() const;

private:
    FileSystemService& fileSystemService_;  // 文件系统服务引用（非拥有）
    WorkSpace& workspace_;                  // 工作区引用，用于attach/detach观察者
    std::map<std::string, std::shared_ptr<FileLogger>> fileLoggers_; // 文件日志记录器映射

    // 异常处理（重写Model基类方法）
    void handleException(const std::exception& e) const override;
};

#endif // LOGGERMANAGER_H