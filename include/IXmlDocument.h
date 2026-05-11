#ifndef IXMLDOCUMENT_H
#define IXMLDOCUMENT_H

#include <string>
#include <vector>
#include <utility>
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

// IXmlDocument：XML文档抽象接口（适配器模式目标接口）
// 定义XML文档操作的统一协议，不依赖任何具体的XML解析库
// 子类实现（如XmlDocumentWrapper）封装特定解析库的细节
class IXmlDocument {
public:
    virtual ~IXmlDocument() = default;

    // 加载操作
    virtual void loadFromString(const std::string& xmlContent) = 0;
    virtual void loadFromFile(const std::string& filePath) = 0;

    // 保存操作
    virtual std::string saveToString() const = 0;
    virtual void saveToFile(const std::string& filePath) const = 0;

    // 状态查询
    virtual bool isLoaded() const = 0;
    virtual void clear() = 0;

    // ID映射：收集所有节点ID（验证唯一性）
    virtual void collectIds() = 0;

    // ID查询
    virtual bool hasNodeWithId(const std::string& id) const = 0;
    virtual std::string getNodeName(const std::string& id) const = 0;
    virtual std::string getNodeValue(const std::string& id) const = 0;
    virtual std::string getNodeAttribute(const std::string& id, const std::string& attrName) const = 0;
    virtual std::vector<std::string> getAllIds() const = 0;

    // 元素操作方法（由命令层调用）
    virtual void insertBefore(const std::string& tagName, const std::string& newId,
                              const std::string& targetId, const std::string& text) = 0;
    virtual void appendChild(const std::string& tagName, const std::string& newId,
                             const std::string& parentId, const std::string& text) = 0;
    virtual void editId(const std::string& oldId, const std::string& newId) = 0;
    virtual void editText(const std::string& elementId, const std::string& text) = 0;
    virtual void deleteElement(const std::string& elementId) = 0;
    virtual bool isRootNode(const std::string& id) const = 0;

    // 供 undo 使用：获取元素的序列化快照
    virtual std::string getNodeXml(const std::string& id) const = 0;
    virtual std::string getParentId(const std::string& id) const = 0;
    virtual int getChildIndex(const std::string& id) const = 0;

    // 供 xml-tree 遍历使用
    virtual std::string getRootId() const = 0;
    virtual std::vector<std::string> getChildIds(const std::string& parentId) const = 0;
    virtual std::vector<std::pair<std::string, std::string>> getNodeAttributes(const std::string& id) const = 0;
};

#endif // IXMLDOCUMENT_H
