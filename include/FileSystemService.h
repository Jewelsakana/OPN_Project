#ifndef FILESYSTEMSERVICE_H
#define FILESYSTEMSERVICE_H

#include "Model.h"
#include "DataStructures.h"
#include <string>
#include <vector>
#include <memory>

// FileSystemService类：负责文件系统操作，继承自Model基类
class FileSystemService : public Model {
public:
    FileSystemService();
    ~FileSystemService() override = default;

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

    // 创建新文件（如果不存在）
    bool createFileIfNotExists(const std::string& fileName);

    // 获取文件大小
    size_t getFileSize(const std::string& fileName) const;

    // 获取文件最后修改时间
    std::string getFileLastModified(const std::string& fileName) const;

    // 读取文件原始内容（用于Editor多态反序列化）
    std::string readFileContent(const std::string& fileName);

    // 写入原始内容到文件（用于Editor多态序列化）
    void writeFileContent(const std::string& fileName, const std::string& content);


private:
    // 异常处理重写
    void handleException(const std::exception& e) const override;
};

#endif // FILESYSTEMSERVICE_H