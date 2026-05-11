#include "OutputService.h"
#include "IXmlDocument.h"
#include <iostream>
#include <vector>
#include <string>
#include <functional>

OutputService::OutputService() {}

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
    std::cout << (isLast ? "└── " : "├── ");
    std::cout << node.name << std::endl;

    // 构建子节点的前缀
    std::string childPrefix = prefix + (isLast ? "    " : "│   ");

    // 递归打印所有子节点
    for (size_t i = 0; i < node.children.size(); ++i) {
        bool childIsLast = (i == node.children.size() - 1);
        printTreeNode(*node.children[i], childPrefix, childIsLast);
    }
}

void OutputService::outputXmlTree(const IXmlDocument& doc) {
    std::string rootId = doc.getRootId();
    if (rootId.empty()) {
        outputLine("(empty)");
        return;
    }

    // 构建属性字符串
    auto formatAttrs = [&](const std::string& id) -> std::string {
        auto attrs = doc.getNodeAttributes(id);
        if (attrs.empty()) return "";
        std::string result;
        for (size_t i = 0; i < attrs.size(); ++i) {
            if (i > 0) result += ", ";
            result += attrs[i].first + "=\"" + attrs[i].second + "\"";
        }
        return " [" + result + "]";
    };

    // 递归打印
    std::function<void(const std::string&, const std::string&, const std::string&)> printNodeRec;
    printNodeRec = [&](const std::string& id, const std::string& prefix, const std::string& recursePrefix) {
        std::string nodeName = doc.getNodeName(id);
        if (nodeName.empty()) return;

        std::cout << prefix << nodeName << formatAttrs(id) << std::endl;

        std::string text = doc.getNodeValue(id);
        auto childIds = doc.getChildIds(id);

        bool hasText = !text.empty();
        size_t totalChildren = childIds.size() + (hasText ? 1 : 0);
        size_t count = 0;

        for (size_t i = 0; i < childIds.size(); ++i) {
            ++count;
            bool isLast = (count == totalChildren);
            std::string nextPrefix = recursePrefix + (isLast ? "└── " : "├── ");
            std::string nextRecurse = recursePrefix + (isLast ? "    " : "│   ");
            printNodeRec(childIds[i], nextPrefix, nextRecurse);
        }

        if (hasText) {
            std::string textPrefix = recursePrefix + "└── ";
            std::cout << textPrefix << "\"" << text << "\"" << std::endl;
        }
    };

    // 打印根节点
    std::string nodeName = doc.getNodeName(rootId);
    std::cout << nodeName << formatAttrs(rootId) << std::endl;

    std::string text = doc.getNodeValue(rootId);
    auto childIds = doc.getChildIds(rootId);
    bool hasText = !text.empty();
    size_t totalChildren = childIds.size() + (hasText ? 1 : 0);
    size_t count = 0;

    for (size_t i = 0; i < childIds.size(); ++i) {
        ++count;
        bool isLast = (count == totalChildren);
        printNodeRec(childIds[i],
                     isLast ? "└── " : "├── ",
                     isLast ? "    " : "│   ");
    }

    if (hasText) {
        std::cout << "└── \"" << text << "\"" << std::endl;
    }
}

void OutputService::handleException(const std::exception& e) const {
    // 基础实现：输出到标准错误
    std::cerr << "OutputService error: " << e.what() << std::endl;
}