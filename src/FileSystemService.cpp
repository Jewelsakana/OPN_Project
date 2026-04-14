#include "FileSystemService.h"
#include "WorkSpace.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

// 尝试使用filesystem库
#if __has_include(<filesystem>) && (!defined(__GNUC__) || __GNUC__ >= 9)
// GCC 8的filesystem实现有问题，使用实验版本
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

std::string FileSystemService::getDirectoryTree(const std::string& path) {
    return safeExecute([this, &path]() -> std::string {
#if HAS_FILESYSTEM
        std::stringstream ss;

        fs::path dirPath;
        if (path.empty()) {
            dirPath = fs::current_path();
        } else {
            dirPath = fs::path(path);
        }

        // 检查路径是否存在
        if (!fs::exists(dirPath)) {
            throw std::runtime_error("目录不存在: " + dirPath.string());
        }

        // 检查是否为目录
        if (!fs::is_directory(dirPath)) {
            throw std::runtime_error("路径不是目录: " + dirPath.string());
        }

        ss << dirPath.string() << "\n";
        ss << buildDirectoryTree(dirPath.string(), "", true);
        return ss.str();
#else
        return "Filesystem library not available. Cannot show directory tree.";
#endif
    });
}

std::string FileSystemService::buildDirectoryTree(const std::string& path, const std::string& prefix, bool isLast) {
#if HAS_FILESYSTEM
    std::stringstream ss;

    try {
        // 收集目录项
        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(path)) {
            entries.push_back(entry);
        }

        // 排序：先目录后文件，按名称排序
        std::sort(entries.begin(), entries.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b) {
                bool aIsDir = fs::is_directory(a.path());
                bool bIsDir = fs::is_directory(b.path());
                if (aIsDir != bIsDir) {
                    return aIsDir > bIsDir; // 目录在前
                }
                return a.path().filename().string() < b.path().filename().string();
            });

        // 遍历并构建字符串
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            bool lastItem = (i == entries.size() - 1);

            // 当前行的前缀
            ss << prefix;
            if (isLast) {
                ss << "    ";
            } else {
                ss << "│   ";
            }

            // 连接符号
            if (lastItem) {
                ss << "└── ";
            } else {
                ss << "├── ";
            }

            // 条目名称
            std::string name = entry.path().filename().string();
            if (fs::is_directory(entry.path())) {
                // 目录加斜杠
                ss << name << "/\n";
                // 递归处理子目录
                std::string newPrefix = prefix;
                if (isLast) {
                    newPrefix += "    ";
                } else {
                    newPrefix += "│   ";
                }
                ss << buildDirectoryTree(entry.path().string(), newPrefix, lastItem);
            } else {
                // 文件
                ss << name << "\n";
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("访问目录出错: " + std::string(e.what()));
    }

    return ss.str();
#else
    return "";
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
    // 可以添加文件系统特定的异常处理，如日志记录
    std::cerr << "FileSystemService error: " << e.what() << std::endl;
}

bool FileSystemService::isFilesystemAvailable() const {
    return HAS_FILESYSTEM == 1;
}

std::shared_ptr<TreeNode> FileSystemService::getDirectoryTreeStructure(const std::string& path) {
    return safeExecute([this, &path]() -> std::shared_ptr<TreeNode> {
#if HAS_FILESYSTEM
        fs::path dirPath;
        if (path.empty()) {
            dirPath = fs::current_path();
        } else {
            dirPath = fs::path(path);
        }

        // 检查路径是否存在
        if (!fs::exists(dirPath)) {
            throw std::runtime_error("目录不存在: " + dirPath.string());
        }

        // 检查是否为目录
        if (!fs::is_directory(dirPath)) {
            throw std::runtime_error("路径不是目录: " + dirPath.string());
        }

        // 构建结构化目录树
        return buildDirectoryTreeStructure(dirPath.string());
#else
        throw std::runtime_error("Filesystem library not available. Cannot build directory tree structure.");
#endif
    });
}

std::shared_ptr<TreeNode> FileSystemService::buildDirectoryTreeStructure(const std::string& path) {
#if HAS_FILESYSTEM
    try {
        // 创建当前节点
        fs::path currentPath(path);
        std::string name = currentPath.filename().string();
        if (name.empty()) {
            // 如果是根目录，使用路径本身作为名称
            name = currentPath.string();
        }

        auto node = std::make_shared<TreeNode>(name, true); // 目录节点

        // 收集子项
        std::vector<fs::directory_entry> entries;
        for (const auto& entry : fs::directory_iterator(path)) {
            entries.push_back(entry);
        }

        // 排序：先目录后文件，按名称排序
        std::sort(entries.begin(), entries.end(),
            [](const fs::directory_entry& a, const fs::directory_entry& b) {
                bool aIsDir = fs::is_directory(a.path());
                bool bIsDir = fs::is_directory(b.path());
                if (aIsDir != bIsDir) {
                    return aIsDir > bIsDir; // 目录在前
                }
                return a.path().filename().string() < b.path().filename().string();
            });

        // 递归处理子项
        for (const auto& entry : entries) {
            std::string entryName = entry.path().filename().string();

            if (fs::is_directory(entry.path())) {
                // 递归处理子目录
                auto childNode = buildDirectoryTreeStructure(entry.path().string());
                node->children.push_back(childNode);
            } else {
                // 文件节点
                auto fileNode = std::make_shared<TreeNode>(entryName, false);
                node->children.push_back(fileNode);
            }
        }

        return node;
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("访问目录出错: " + std::string(e.what()));
    }
#else
    return nullptr;
#endif
}

// 辅助函数：分割字符串
namespace {
    std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // 辅助函数：修剪字符串
    std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        size_t end = str.find_last_not_of(" \t\r\n");
        if (start == std::string::npos || end == std::string::npos) return "";
        return str.substr(start, end - start + 1);
    }
}

// 保存工作区配置到文件（简单文本格式）
void FileSystemService::saveWorkspaceConfig(const std::string& fileName, const WorkspaceMemento& memento) {
    safeExecute([this, &fileName, &memento]() {
        std::ofstream file(fileName);
        if (!file) {
            throw std::runtime_error("无法写入配置文件: " + fileName);
        }

        // 写入openFiles（格式：openFiles: file1,file2,file3）
        const auto& openFiles = memento.getOpenFiles();
        file << "openFiles:";
        for (size_t i = 0; i < openFiles.size(); ++i) {
            file << openFiles[i];
            if (i != openFiles.size() - 1) file << ",";
        }
        file << "\n";

        // 写入activeFileName
        file << "activeFileName: " << memento.getActiveFileName() << "\n";

        // 写入fileModifiedStates（格式：fileModifiedStates: file1=true,file2=false）
        const auto& modifiedStates = memento.getFileModifiedStates();
        file << "fileModifiedStates:";
        size_t count = 0;
        for (const auto& pair : modifiedStates) {
            file << pair.first << "=" << (pair.second ? "true" : "false");
            if (++count < modifiedStates.size()) file << ",";
        }
        file << "\n";

        // 写入logEnabled
        file << "logEnabled: " << (memento.isLogEnabled() ? "true" : "false") << "\n";

        file.close();
    });
}

// 从文件加载工作区配置（简单文本格式）
std::shared_ptr<WorkspaceMemento> FileSystemService::loadWorkspaceConfig(const std::string& fileName) {
    return safeExecute([this, &fileName]() -> std::shared_ptr<WorkspaceMemento> {
        // 如果文件不存在，返回空指针
        if (!fileExists(fileName)) {
            return nullptr;
        }

        std::ifstream file(fileName);
        if (!file) {
            throw std::runtime_error("无法打开配置文件: " + fileName);
        }

        std::vector<std::string> openFiles;
        std::string activeFileName;
        std::map<std::string, bool> fileModifiedStates;
        bool logEnabled = false;

        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;

            // 查找冒号位置
            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) {
                // 无效行，跳过
                continue;
            }

            std::string key = trim(line.substr(0, colonPos));
            std::string value = trim(line.substr(colonPos + 1));

            if (key == "openFiles") {
                // 解析逗号分隔的文件列表
                if (!value.empty()) {
                    auto files = splitString(value, ',');
                    for (const auto& file : files) {
                        std::string trimmedFile = trim(file);
                        if (!trimmedFile.empty()) {
                            openFiles.push_back(trimmedFile);
                        }
                    }
                }
            } else if (key == "activeFileName") {
                activeFileName = value;
            } else if (key == "fileModifiedStates") {
                // 解析逗号分隔的键值对（file=true格式）
                if (!value.empty()) {
                    auto pairs = splitString(value, ',');
                    for (const auto& pair : pairs) {
                        std::string trimmedPair = trim(pair);
                        size_t equalPos = trimmedPair.find('=');
                        if (equalPos != std::string::npos) {
                            std::string fileKey = trim(trimmedPair.substr(0, equalPos));
                            std::string boolValue = trim(trimmedPair.substr(equalPos + 1));
                            if (boolValue == "true") {
                                fileModifiedStates[fileKey] = true;
                            } else if (boolValue == "false") {
                                fileModifiedStates[fileKey] = false;
                            }
                        }
                    }
                }
            } else if (key == "logEnabled") {
                logEnabled = (value == "true");
            }
        }

        file.close();

        // 创建Memento
        return std::make_shared<WorkspaceMemento>(openFiles, activeFileName, fileModifiedStates, logEnabled);
    });
}