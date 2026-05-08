#include "FileSystemService.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

// 尝试使用filesystem库
#if __has_include(<filesystem>) && (!defined(__GNUC__) || __GNUC__ >= 9)
#  include <filesystem>
   namespace fs = std::filesystem;
#  define HAS_FILESYSTEM 1
#elif __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
   namespace fs = std::experimental::filesystem;
#  define HAS_FILESYSTEM 1
#else
#  define HAS_FILESYSTEM 0
#endif

FileSystemService::FileSystemService() {
    // 构造函数可以初始化一些状态
}

std::vector<std::string> FileSystemService::loadFile(const std::string& fileName) {
    return safeExecute([this, &fileName]() -> std::vector<std::string> {
        std::ifstream file(fileName);
        if (!file) {
            throw std::runtime_error("无法打开文件: " + fileName);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        file.close();
        return lines;
    });
}

void FileSystemService::saveFile(const std::string& fileName, const std::vector<std::string>& lines) {
    safeExecute([this, &fileName, &lines]() {
        std::ofstream file(fileName);
        if (!file) {
            throw std::runtime_error("无法写入文件: " + fileName);
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            file << lines[i];
            if (i != lines.size() - 1) {
                file << '\n'; // 行间换行
            }
        }

        file.close();
    });
}

void FileSystemService::appendToFile(const std::string& fileName, const std::string& content) {
    safeExecute([this, &fileName, &content]() {
        std::ofstream file(fileName, std::ios::app); // 追加模式
        if (!file) {
            throw std::runtime_error("无法追加到文件: " + fileName);
        }
        file << content;
        file.close();
    });
}

bool FileSystemService::fileExists(const std::string& fileName) const {
#if HAS_FILESYSTEM
    return fs::exists(fileName);
#else
    // 回退方案：尝试打开文件
    std::ifstream file(fileName);
    return file.good();
#endif
}

bool FileSystemService::createFileIfNotExists(const std::string& fileName) {
    return safeExecute([this, &fileName]() -> bool {
        if (!fileExists(fileName)) {
            std::ofstream file(fileName);
            if (!file) {
                throw std::runtime_error("无法创建文件: " + fileName);
            }
            file.close();
            return true;
        }
        return false;
    });
}

size_t FileSystemService::getFileSize(const std::string& fileName) const {
#if HAS_FILESYSTEM
    try {
        return fs::file_size(fileName);
    } catch (const fs::filesystem_error&) {
        return 0;
    }
#else
    // 回退方案：打开文件并获取大小
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    if (!file) return 0;
    return file.tellg();
#endif
}

std::string FileSystemService::getFileLastModified(const std::string& fileName) const {
#if HAS_FILESYSTEM
    try {
        auto ftime = fs::last_write_time(fileName);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    } catch (const fs::filesystem_error&) {
        return "Unknown";
    }
#else
    return "Not available";
#endif
}

void FileSystemService::handleException(const std::exception& e) const {
    std::cerr << "FileSystemService error: " << e.what() << std::endl;
}