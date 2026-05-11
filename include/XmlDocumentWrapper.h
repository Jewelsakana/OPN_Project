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

    // 获取底层文档对象（仅限内部使用，谨慎调用）
    pugi::xml_document& getPugiDocument();
    const pugi::xml_document& getPugiDocument() const;

    // 获取根节点
    pugi::xml_node root() const;

private:
    // 递归遍历收集ID
    void traverseForIds(pugi::xml_node node);

    pugi::xml_document doc_;
    bool loaded_;

    // ID到节点的快速映射（内部实现细节，不对外暴露）
    std::unordered_map<std::string, pugi::xml_node> idToNodeMap_;
};

#endif // XMLDOCUMENTWRAPPER_H
