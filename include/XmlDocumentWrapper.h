#ifndef XMLDOCUMENTWRAPPER_H
#define XMLDOCUMENTWRAPPER_H

#include "pugixml.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

// XML文档异常基类
class XmlDocumentException : public std::runtime_error {
public:
    explicit XmlDocumentException(const std::string& message)
        : std::runtime_error(message) {}
};

// 解析错误异常
class XmlParseException : public XmlDocumentException {
public:
    explicit XmlParseException(const std::string& message)
        : XmlDocumentException("XML parse error: " + message) {}
};

// 重复ID异常
class DuplicateIdException : public XmlDocumentException {
public:
    explicit DuplicateIdException(const std::string& id)
        : XmlDocumentException("Duplicate ID found: '" + id + "'") {}
};

// 缺少ID异常
class MissingIdException : public XmlDocumentException {
public:
    explicit MissingIdException(const std::string& nodeName)
        : XmlDocumentException("Missing required 'id' attribute on element: '" + nodeName + "'") {}
};

// XmlDocumentWrapper：适配器模式封装PugiXML的xml_document
// 提供统一的XML文档操作接口，便于后续替换底层XML解析库
class XmlDocumentWrapper {
public:
    XmlDocumentWrapper();
    ~XmlDocumentWrapper() = default;

    // 从字符串加载XML
    void loadFromString(const std::string& xmlContent);

    // 从文件加载XML
    void loadFromFile(const std::string& filePath);

    // 将XML保存为字符串
    std::string saveToString() const;

    // 将XML保存到文件
    void saveToFile(const std::string& filePath) const;

    // 获取根节点
    pugi::xml_node root() const;

    // 获取底层文档对象（用于高级操作）
    pugi::xml_document& getDocument();
    const pugi::xml_document& getDocument() const;

    // 遍历所有元素节点，收集ID到节点的映射
    // 返回 map<id, node>，如发现重复ID则抛出DuplicateIdException
    void collectIds(std::unordered_map<std::string, pugi::xml_node>& idMap) const;

    // 检查是否已加载文档
    bool isLoaded() const;

    // 清空文档
    void clear();

private:
    pugi::xml_document doc_;
    bool loaded_;
};

#endif // XMLDOCUMENTWRAPPER_H
