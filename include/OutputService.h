#ifndef OUTPUTSERVICE_H
#define OUTPUTSERVICE_H

#include "Model.h"
#include "DataStructures.h"
#include <string>
#include <vector>
#include <memory>

// OutputService类：负责统一输出逻辑，继承自Model基类
class OutputService : public Model {
public:
    OutputService();
    ~OutputService() override = default;

    // 获取实例名称
    std::string getName() const override {
        return "OutputService";
    }

    // 输出文件列表（接收结构化数据）
    void outputList(const std::vector<FileInfo>& files);

    // 输出目录树（接收结构化数据）
    void outputTree(const TreeNode& root);

    // 输出错误信息
    void outputError(const std::string& errorMessage);

    // 输出单行信息
    void outputLine(const std::string& message);

    // 输出纯文本（用于向后兼容）
    void outputText(const std::string& text);

private:
    // 递归打印目录树的辅助函数
    void printTreeNode(const TreeNode& node, const std::string& prefix, bool isLast);

    // 异常处理重写
    void handleException(const std::exception& e) const override;
};

#endif // OUTPUTSERVICE_H