#ifndef LOGGER_H
#define LOGGER_H

#include "Observe.h"
#include "Event.h"
#include "FileSystemService.h"
#include <string>
#include <memory>

// FileLogger类：日志观察者，负责将事件记录到日志文件
// 每个FileLogger实例对应一个特定的文件（Editor）
class FileLogger : public Observe {
public:
    // 构造函数，需要关联的文件名和FileSystemService引用
    explicit FileLogger(const std::string& fileName, FileSystemService& fileSystemService);
    ~FileLogger() override = default;

    // 实现Observe接口
    void update(const Event& e) override;

    // 获取关联的文件名
    const std::string& getFileName() const;

    // 检查是否为指定文件的日志记录器
    bool isForFile(const std::string& fileName) const;

    // 写入日志条目（辅助方法）
    void writeLogEntry(const std::string& entry);

private:
    std::string fileName_;                     // 关联的文件名
    FileSystemService& fileSystemService_;     // 文件系统服务引用
    std::string logFileName_;                  // 日志文件名（例如 ".filename.log"）

    // 生成日志文件名
    std::string generateLogFileName() const;

    // 将时间点转换为字符串
    std::string timePointToString(const Event::TimePoint& tp) const;
};

#endif // LOGGER_H