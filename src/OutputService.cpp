#include "OutputService.h"
#include <iostream>
#include <vector>
#include <string>

OutputService::OutputService() {
    // 构造函数
}

void OutputService::outputList(const std::vector<FileInfo>& files) {
    try {
        validate();

        if (files.empty()) {
            std::cout << "No files open in workspace" << std::endl;
            return;
        }

        for (const auto& file : files) {
            // 标记活动文件
            if (file.isActive) {
                std::cout << "* ";
            } else {
                std::cout << "  ";
            }

            // 输出文件名
            std::cout << file.fileName;

            // 标记已修改的文件
            if (file.isModified) {
                std::cout << " [modified]";
            }

            std::cout << std::endl;
        }
    } catch (const std::exception& e) {
        handleException(e);
        throw; // 重新抛出异常，让调用者处理
    }
}

void OutputService::outputTree(const TreeNode& root) {
    try {
        validate();

        // 从根节点开始打印，根节点本身通常是当前目录
        std::cout << root.name << std::endl;

        // 递归打印所有子节点
        for (size_t i = 0; i < root.children.size(); ++i) {
            bool isLast = (i == root.children.size() - 1);
            printTreeNode(*root.children[i], "", isLast);
        }
    } catch (const std::exception& e) {
        handleException(e);
        throw;
    }
}

void OutputService::outputError(const std::string& errorMessage) {
    try {
        validate();
        std::cerr << "Error: " << errorMessage << std::endl;
    } catch (const std::exception& e) {
        // 即使输出错误失败，我们也尝试输出原始错误
        std::cerr << "Error (in error output): " << e.what() << std::endl;
        std::cerr << "Original error: " << errorMessage << std::endl;
    }
}

void OutputService::outputLine(const std::string& message) {
    try {
        validate();
        std::cout << message << std::endl;
    } catch (const std::exception& e) {
        handleException(e);
        throw;
    }
}

void OutputService::outputText(const std::string& text) {
    try {
        validate();
        std::cout << text;
    } catch (const std::exception& e) {
        handleException(e);
        throw;
    }
}

void OutputService::printTreeNode(const TreeNode& node, const std::string& prefix, bool isLast) {
    // 打印当前节点
    std::cout << prefix;
    std::cout << (isLast ? "`-- " : "|-- ");
    std::cout << node.name << std::endl;

    // 构建子节点的前缀
    std::string childPrefix = prefix + (isLast ? "    " : "|   ");

    // 递归打印所有子节点
    for (size_t i = 0; i < node.children.size(); ++i) {
        bool childIsLast = (i == node.children.size() - 1);
        printTreeNode(*node.children[i], childPrefix, childIsLast);
    }
}

void OutputService::handleException(const std::exception& e) const {
    // 基础实现：输出到标准错误
    std::cerr << "OutputService error: " << e.what() << std::endl;
}