#include "Logger.h"
#include <iomanip>
#include <sstream>
#include <chrono>

FileLogger::FileLogger(const std::string& fileName, FileSystemService& fileSystemService)
    : fileName_(fileName), fileSystemService_(fileSystemService) {
    logFileName_ = generateLogFileName();
}

void FileLogger::update(const Event& e) {
    // 检查事件是否针对本文件
    std::string targetFile = e.getTargetFileName();
    if (!targetFile.empty() && targetFile != fileName_) {
        // 事件针对其他文件，忽略
        return;
    }
    // 目标文件名为空或与当前文件名匹配，记录事件
    std::string entry = timePointToString(e.getTimestamp()) + " - " + e.getCommandContent();
    writeLogEntry(entry);
}

const std::string& FileLogger::getFileName() const {
    return fileName_;
}

bool FileLogger::isForFile(const std::string& fileName) const {
    return fileName_ == fileName;
}

void FileLogger::writeLogEntry(const std::string& entry) {
    try {
        fileSystemService_.appendToFile(logFileName_, entry + "\n");
    } catch (const std::exception& e) {
        // 日志写入失败只警告，不中断程序
        // 输出到标准错误流
        std::cerr << "警告: 无法写入日志文件 " << logFileName_ << ": " << e.what() << std::endl;
    }
}

std::string FileLogger::generateLogFileName() const {
    // 生成日志文件名：".filename.log"
    // 如果文件名包含路径，只取文件名部分？还是保留路径？
    // 简单处理：在原文件名前加"."，后加".log"
    // 例如 "test.txt" -> ".test.txt.log"
    // 但需要注意路径分隔符
    // 暂时直接处理
    return "." + fileName_ + ".log";
}

std::string FileLogger::timePointToString(const Event::TimePoint& tp) const {
    auto time = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}