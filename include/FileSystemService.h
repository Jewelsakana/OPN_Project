#ifndef FILESYSTEMSERVICE_H
#define FILESYSTEMSERVICE_H

#include "Model.h"
#include "DataStructures.h"
#include <string>
#include <vector>
#include <memory>

// 前向声明
class TextEditor;
class WorkspaceMemento;

// FileSystemService类：负责文件系统操作，继承自Model基类
class FileSystemService : public Model {
public:
    FileSystemService();
    ~FileSystemService() override = default;

    // 获取实例名称
    std::string getName() const override {
        return "FileSystemService";
    }

    // 从文件加载内容到TextEditor
    // 参数：fileName - 文件名
    // 返回：读取的行内容向量
    std::vector<std::string> loadFile(const std::string& fileName);

    // 将TextEditor内容保存到文件
    // 参数：fileName - 文件名，lines - 要保存的行内容
    void saveFile(const std::string& fileName, const std::vector<std::string>& lines);

    // 追加内容到文件（用于日志记录）
    // 参数：fileName - 文件名，content - 要追加的内容
    void appendToFile(const std::string& fileName, const std::string& content);

    // 检查文件是否存在
    bool fileExists(const std::string& fileName) const;

    // 获取目录树结构（字符串表示，用于向后兼容）
    // 参数：path - 目录路径，为空时使用当前目录
    // 返回：目录树的字符串表示
    std::string getDirectoryTree(const std::string& path = "");

    // 获取目录树结构（结构化数据）
    // 参数：path - 目录路径，为空时使用当前目录
    // 返回：目录树的根节点
    std::shared_ptr<TreeNode> getDirectoryTreeStructure(const std::string& path = "");

    // 创建新文件（如果不存在）
    bool createFileIfNotExists(const std::string& fileName);

    // 获取文件大小
    size_t getFileSize(const std::string& fileName) const;

    // 获取文件最后修改时间
    std::string getFileLastModified(const std::string& fileName) const;

    // 保存工作区配置到文件（JSON格式）
    void saveWorkspaceConfig(const std::string& fileName, const WorkspaceMemento& memento);

    // 从文件加载工作区配置（JSON格式）
    std::shared_ptr<WorkspaceMemento> loadWorkspaceConfig(const std::string& fileName);

private:
    // 递归构建目录树（字符串）
    std::string buildDirectoryTree(const std::string& path, const std::string& prefix, bool isLast);

    // 递归构建目录树（结构化数据）
    std::shared_ptr<TreeNode> buildDirectoryTreeStructure(const std::string& path);

    // 异常处理重写
    void handleException(const std::exception& e) const override;

    // 检查文件系统库可用性
    bool isFilesystemAvailable() const;
};

#endif // FILESYSTEMSERVICE_H