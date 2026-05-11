#ifndef XMLDOCUMENTWRAPPER_H
#define XMLDOCUMENTWRAPPER_H

#include "IXmlDocument.h"
#include "pugixml.hpp"
#include <string>
#include <vector>
#include <unordered_map>

// XmlDocumentWrapper：适配器模式——将PugiXML的xml_document适配为IXmlDocument接口
// ID映射作为内部实现细节，不对外暴露pugi类型
class XmlDocumentWrapper : public IXmlDocument {
public:
    XmlDocumentWrapper();
    ~XmlDocumentWrapper() override = default;

    // IXmlDocument接口实现
    void loadFromString(const std::string& xmlContent) override;
    void loadFromFile(const std::string& filePath) override;
    std::string saveToString() const override;
    void saveToFile(const std::string& filePath) const override;
    bool isLoaded() const override;
    void clear() override;

    // ID映射（内部管理，验证唯一性）
    void collectIds() override;
    bool hasNodeWithId(const std::string& id) const override;
    std::string getNodeName(const std::string& id) const override;
    std::string getNodeValue(const std::string& id) const override;
    std::string getNodeAttribute(const std::string& id, const std::string& attrName) const override;
    std::vector<std::string> getAllIds() const override;

    // 元素操作方法
    void insertBefore(const std::string& tagName, const std::string& newId,
                      const std::string& targetId, const std::string& text) override;
    void appendChild(const std::string& tagName, const std::string& newId,
                     const std::string& parentId, const std::string& text) override;
    void editId(const std::string& oldId, const std::string& newId) override;
    void editText(const std::string& elementId, const std::string& text) override;
    void deleteElement(const std::string& elementId) override;
    bool isRootNode(const std::string& id) const override;

    // undo 辅助
    std::string getNodeXml(const std::string& id) const override;
    std::string getParentId(const std::string& id) const override;
    int getChildIndex(const std::string& id) const override;

    // xml-tree 遍历
    std::string getRootId() const override;
    std::vector<std::string> getChildIds(const std::string& parentId) const override;
    std::vector<std::pair<std::string, std::string>> getNodeAttributes(const std::string& id) const override;

private:
    // 辅助：创建带id和可选文本的新元素，返回新节点
    pugi::xml_node createElement(pugi::xml_node parent, const std::string& tagName,
                                 const std::string& id, const std::string& text,
                                 bool insertBefore, pugi::xml_node targetNode);

    // 辅助：验证并注册单个节点的ID
    void registerNodeId(pugi::xml_node node);

    // 重建ID映射（在增删改操作后调用）
    void rebuildIdMap();

    // 获取底层文档对象（仅限内部使用）
    pugi::xml_document& getPugiDocument();
    const pugi::xml_document& getPugiDocument() const;

    // 获取根节点
    pugi::xml_node root() const;

    pugi::xml_document doc_;
    bool loaded_;

    // ID到节点的快速映射（内部实现细节，不对外暴露）
    std::unordered_map<std::string, pugi::xml_node> idToNodeMap_;
};

#endif // XMLDOCUMENTWRAPPER_H
