#ifndef IXMLDOCUMENT_H
#define IXMLDOCUMENT_H

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include "ISpellChecker.h"

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

// IXmlReader：XML文档只读查询接口（ISP 微接口）
// 客户端仅需查询节点信息时依赖此接口
class IXmlReader {
public:
    virtual ~IXmlReader() = default;

    virtual bool isLoaded() const = 0;
    virtual bool hasNodeWithId(const std::string& id) const = 0;
    virtual std::string getNodeName(const std::string& id) const = 0;
    virtual std::string getNodeValue(const std::string& id) const = 0;
    virtual std::string getNodeAttribute(const std::string& id, const std::string& attrName) const = 0;
    virtual std::vector<std::string> getAllIds() const = 0;
    virtual bool isRootNode(const std::string& id) const = 0;
};

// IXmlWriter：XML文档元素变更接口（ISP 微接口）
// 客户端仅需修改元素时依赖此接口
class IXmlWriter {
public:
    virtual ~IXmlWriter() = default;

    virtual void insertBefore(const std::string& tagName, const std::string& newId,
                              const std::string& targetId, const std::string& text) = 0;
    virtual void appendChild(const std::string& tagName, const std::string& newId,
                             const std::string& parentId, const std::string& text) = 0;
    virtual void editId(const std::string& oldId, const std::string& newId) = 0;
    virtual void editText(const std::string& elementId, const std::string& text) = 0;
    virtual void deleteElement(const std::string& elementId) = 0;
};

// IXmlNavigator：XML文档生命周期与遍历接口（ISP 微接口）
// 负责加载/保存、初始化、ID映射、结构遍历、undo快照和拼写检查
class IXmlNavigator {
public:
    virtual ~IXmlNavigator() = default;

    // 加载与保存
    virtual void loadFromString(const std::string& xmlContent) = 0;
    virtual void loadFromFile(const std::string& filePath) = 0;
    virtual std::string saveToString() const = 0;
    virtual void saveToFile(const std::string& filePath) const = 0;

    // 状态与生命周期
    virtual void clear() = 0;
    virtual void initContent(bool withLog) = 0;

    // ID映射
    virtual void collectIds() = 0;

    // 结构遍历
    virtual std::string getRootId() const = 0;
    virtual std::vector<std::string> getChildIds(const std::string& parentId) const = 0;
    virtual std::vector<std::pair<std::string, std::string>> getNodeAttributes(const std::string& id) const = 0;

    // Undo 辅助：快照与导航
    virtual std::string getNodeXml(const std::string& id) const = 0;
    virtual std::string getParentId(const std::string& id) const = 0;
    virtual int getChildIndex(const std::string& id) const = 0;

    // 拼写检查：提取文本节点内容
    virtual std::vector<TextSegment> getTextsToCheck() const = 0;
};

// IXmlDocument：XML文档抽象接口（适配器模式目标接口）
// 组合 IXmlReader + IXmlWriter + IXmlNavigator 三个微接口
// 子类实现（如XmlDocumentWrapper）封装特定解析库的细节
class IXmlDocument : public IXmlReader, public IXmlWriter, public IXmlNavigator {
public:
    ~IXmlDocument() override = default;
};

#endif // IXMLDOCUMENT_H
