#include "DirectoryService.h"
#include "FilesystemCompat.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>

#if HAS_FILESYSTEM
namespace {

bool compareDirectoryEntries(const fs::directory_entry& a, const fs::directory_entry& b) {
    bool aIsDir = fs::is_directory(a.path());
    bool bIsDir = fs::is_directory(b.path());
    if (aIsDir != bIsDir) {
        return aIsDir > bIsDir;
    }
    return a.path().filename().string() < b.path().filename().string();
}

// 解析并验证目录路径（消除 getDirectoryTree / getDirectoryTreeStructure 的重复）
fs::path resolveDirPath(const std::string& path) {
    fs::path dirPath;
    if (path.empty()) {
        dirPath = fs::current_path();
    } else {
        dirPath = fs::path(path);
    }
    if (!fs::exists(dirPath)) {
        throw std::runtime_error("Directory does not exist: " + dirPath.string());
    }
    if (!fs::is_directory(dirPath)) {
        throw std::runtime_error("Path is not a directory: " + dirPath.string());
    }
    return dirPath;
}

// 获取排序后的目录条目列表（消除 buildDirectoryTree / buildDirectoryTreeStructure 的重复）
std::vector<fs::directory_entry> getSortedEntries(const std::string& path) {
    std::vector<fs::directory_entry> entries;
    for (const auto& entry : fs::directory_iterator(path)) {
        entries.push_back(entry);
    }
    std::sort(entries.begin(), entries.end(), compareDirectoryEntries);
    return entries;
}

} // anonymous namespace
#endif

std::string DirectoryService::getDirectoryTree(const std::string& path) {
    return safeExecute([this, &path]() -> std::string {
#if HAS_FILESYSTEM
        fs::path dirPath = resolveDirPath(path);

        std::stringstream ss;
        ss << dirPath.string() << "\n";
        ss << buildDirectoryTree(dirPath.string(), "", true);
        return ss.str();
#else
        return "Filesystem library not available. Cannot show directory tree.";
#endif
    });
}

std::string DirectoryService::buildDirectoryTree(const std::string& path, const std::string& prefix, bool isLast) {
#if HAS_FILESYSTEM
    std::stringstream ss;

    try {
        auto entries = getSortedEntries(path);

        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            bool lastItem = (i == entries.size() - 1);

            ss << prefix;
            if (isLast) {
                ss << "    ";
            } else {
                ss << "│   ";
            }

            if (lastItem) {
                ss << "└── ";
            } else {
                ss << "├── ";
            }

            std::string name = entry.path().filename().string();
            if (fs::is_directory(entry.path())) {
                ss << name << "/\n";
                std::string newPrefix = prefix;
                if (isLast) {
                    newPrefix += "    ";
                } else {
                    newPrefix += "│   ";
                }
                ss << buildDirectoryTree(entry.path().string(), newPrefix, lastItem);
            } else {
                ss << name << "\n";
            }
        }
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Error accessing directory: " + std::string(e.what()));
    }

    return ss.str();
#else
    return "";
#endif
}

std::shared_ptr<TreeNode> DirectoryService::getDirectoryTreeStructure(const std::string& path) {
    return safeExecute([this, &path]() -> std::shared_ptr<TreeNode> {
#if HAS_FILESYSTEM
        fs::path dirPath = resolveDirPath(path);
        return buildDirectoryTreeStructure(dirPath.string());
#else
        throw std::runtime_error("Filesystem library not available. Cannot build directory tree structure.");
#endif
    });
}

std::shared_ptr<TreeNode> DirectoryService::buildDirectoryTreeStructure(const std::string& path) {
#if HAS_FILESYSTEM
    try {
        fs::path currentPath(path);
        std::string name = currentPath.filename().string();
        if (name.empty()) {
            name = currentPath.string();
        }

        auto node = std::make_shared<TreeNode>(name, true);

        auto entries = getSortedEntries(path);

        for (const auto& entry : entries) {
            std::string entryName = entry.path().filename().string();

            if (fs::is_directory(entry.path())) {
                auto childNode = buildDirectoryTreeStructure(entry.path().string());
                node->children.push_back(childNode);
            } else {
                auto fileNode = std::make_shared<TreeNode>(entryName, false);
                node->children.push_back(fileNode);
            }
        }

        return node;
    } catch (const fs::filesystem_error& e) {
        throw std::runtime_error("Error accessing directory: " + std::string(e.what()));
    }
#else
    return nullptr;
#endif
}

bool DirectoryService::isFilesystemAvailable() const {
    return HAS_FILESYSTEM == 1;
}

void DirectoryService::handleException(const std::exception& e) const {
    std::cerr << "DirectoryService error: " << e.what() << std::endl;
}
