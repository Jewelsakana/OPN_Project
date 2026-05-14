#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <chrono>

// 事件模型Event：封装执行时间戳，命令内容和目标文件名，用于日志记录
class Event {
public:
    using TimePoint = std::chrono::system_clock::time_point;

    // 构造函数
    Event(const std::string& commandContent, const std::string& targetFileName);

    // 获取时间戳
    TimePoint getTimestamp() const;

    // 获取命令内容
    std::string getCommandContent() const;

    // 获取目标文件名
    std::string getTargetFileName() const;

    // 获取当前时间戳字符串（用于日志和事件）
    static std::string currentTimestampString();

private:
    TimePoint timestamp;           // 执行时间戳
    std::string commandContent;    // 命令内容
    std::string targetFileName;    // 目标文件名
};

#endif // EVENT_H