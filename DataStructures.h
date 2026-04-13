#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

#include <string>
#include <vector>
#include <memory>

// 文件信息结构体
struct FileInfo {
    std::string fileName;
    bool isActive;
    bool isModified;

    FileInfo(const std::string& name, bool active, bool modified)
        : fileName(name), isActive(active), isModified(modified) {}
};

// 目录树节点结构体
struct TreeNode {
    std::string name;
    bool isDirectory;
    std::vector<std::shared_ptr<TreeNode>> children;

    TreeNode(const std::string& name, bool isDir)
        : name(name), isDirectory(isDir) {}
};

#endif // DATASTRUCTURES_H