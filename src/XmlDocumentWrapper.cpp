#include "XmlDocumentWrapper.h"
#include <fstream>
#include <sstream>
#include <functional>
#include <algorithm>

XmlDocumentWrapper::XmlDocumentWrapper()
    : loaded_(false) {
}

void XmlDocumentWrapper::loadFromString(const std::string& xmlContent) {
    pugi::xml_parse_result result = doc_.load_string(xmlContent.c_str());
    if (!result) {
        throw XmlParseException(result.description());
    }
    loaded_ = true;
}

void XmlDocumentWrapper::loadFromFile(const std::string& filePath) {
    pugi::xml_parse_result result = doc_.load_file(filePath.c_str());
    if (!result) {
        throw XmlParseException(std::string(result.description()) + " (file: " + filePath + ")");
    }
    loaded_ = true;
}

std::string XmlDocumentWrapper::saveToString() const {
    std::ostringstream oss;
    doc_.save(oss, "  ");
    return oss.str();
}

void XmlDocumentWrapper::saveToFile(const std::string& filePath) const {
    if (!doc_.save_file(filePath.c_str(), "  ")) {
        throw XmlDocumentException("Failed to save XML to file: " + filePath);
    }
}

pugi::xml_document& XmlDocumentWrapper::getPugiDocument() {
    return doc_;
}

const pugi::xml_document& XmlDocumentWrapper::getPugiDocument() const {
    return doc_;
}

pugi::xml_node XmlDocumentWrapper::root() const {
    return doc_.document_element();
}

bool XmlDocumentWrapper::isLoaded() const {
    return loaded_;
}

void XmlDocumentWrapper::clear() {
    doc_.reset();
    idToNodeMap_.clear();
    loaded_ = false;
}

// ---- ID映射实现（内部管理 pugi::xml_node，对外只返回字符串） ----

void XmlDocumentWrapper::collectIds() {
    idToNodeMap_.clear();

    std::function<void(pugi::xml_node)> traverse = [&](pugi::xml_node node) {
        for (pugi::xml_node child : node.children()) {
            if (child.type() == pugi::node_element) {
                pugi::xml_attribute idAttr = child.attribute("id");
                if (idAttr.empty()) {
                    throw MissingIdException(child.name());
                }
                std::string id = idAttr.value();
                if (idToNodeMap_.find(id) != idToNodeMap_.end()) {
                    throw DuplicateIdException(id);
                }
                idToNodeMap_[id] = child;
                traverse(child);
            }
        }
    };

    pugi::xml_node rootNode = root();
    if (rootNode && rootNode.type() == pugi::node_element) {
        pugi::xml_attribute idAttr = rootNode.attribute("id");
        if (idAttr.empty()) {
            throw MissingIdException(rootNode.name());
        }
        std::string id = idAttr.value();
        if (idToNodeMap_.find(id) != idToNodeMap_.end()) {
            throw DuplicateIdException(id);
        }
        idToNodeMap_[id] = rootNode;
        traverse(rootNode);
    }
}

bool XmlDocumentWrapper::hasNodeWithId(const std::string& id) const {
    return idToNodeMap_.find(id) != idToNodeMap_.end();
}

std::string XmlDocumentWrapper::getNodeName(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it != idToNodeMap_.end()) {
        return it->second.name();
    }
    return "";
}

std::string XmlDocumentWrapper::getNodeValue(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it != idToNodeMap_.end()) {
        return it->second.child_value();
    }
    return "";
}

std::string XmlDocumentWrapper::getNodeAttribute(const std::string& id, const std::string& attrName) const {
    auto it = idToNodeMap_.find(id);
    if (it != idToNodeMap_.end()) {
        pugi::xml_attribute attr = it->second.attribute(attrName.c_str());
        if (!attr.empty()) {
            return attr.value();
        }
    }
    return "";
}

std::vector<std::string> XmlDocumentWrapper::getAllIds() const {
    std::vector<std::string> ids;
    ids.reserve(idToNodeMap_.size());
    for (const auto& pair : idToNodeMap_) {
        ids.push_back(pair.first);
    }
    return ids;
}

// ---- 辅助方法 ----

void XmlDocumentWrapper::rebuildIdMap() {
    // 完全重建ID映射（操作后调用collectIds重新验证唯一性）
    collectIds();
}

// ---- 元素操作方法 ----

void XmlDocumentWrapper::insertBefore(const std::string& tagName, const std::string& newId,
                                       const std::string& targetId, const std::string& text) {
    auto it = idToNodeMap_.find(targetId);
    if (it == idToNodeMap_.end()) {
        throw XmlDocumentException("目标元素不存在: " + targetId);
    }

    pugi::xml_node targetNode = it->second;
    if (targetNode == root()) {
        throw XmlDocumentException("不能在根元素前插入元素");
    }

    if (hasNodeWithId(newId)) {
        throw XmlDocumentException("元素ID已存在: " + newId);
    }

    pugi::xml_node newNode = targetNode.parent().insert_child_before(tagName.c_str(), targetNode);
    newNode.append_attribute("id") = newId.c_str();
    if (!text.empty()) {
        newNode.append_child(pugi::node_pcdata).set_value(text.c_str());
    }

    rebuildIdMap();
}

void XmlDocumentWrapper::appendChild(const std::string& tagName, const std::string& newId,
                                      const std::string& parentId, const std::string& text) {
    auto it = idToNodeMap_.find(parentId);
    if (it == idToNodeMap_.end()) {
        throw XmlDocumentException("父元素不存在: " + parentId);
    }

    if (hasNodeWithId(newId)) {
        throw XmlDocumentException("元素ID已存在: " + newId);
    }

    pugi::xml_node parentNode = it->second;
    pugi::xml_node newNode = parentNode.append_child(tagName.c_str());
    newNode.append_attribute("id") = newId.c_str();
    if (!text.empty()) {
        newNode.append_child(pugi::node_pcdata).set_value(text.c_str());
    }

    rebuildIdMap();
}

void XmlDocumentWrapper::editId(const std::string& oldId, const std::string& newId) {
    auto it = idToNodeMap_.find(oldId);
    if (it == idToNodeMap_.end()) {
        throw XmlDocumentException("元素不存在: " + oldId);
    }

    if (isRootNode(oldId)) {
        throw XmlDocumentException("不建议修改根元素ID");
    }

    if (hasNodeWithId(newId)) {
        throw XmlDocumentException("目标ID已存在: " + newId);
    }

    pugi::xml_attribute idAttr = it->second.attribute("id");
    idAttr.set_value(newId.c_str());

    rebuildIdMap();
}

void XmlDocumentWrapper::editText(const std::string& elementId, const std::string& text) {
    auto it = idToNodeMap_.find(elementId);
    if (it == idToNodeMap_.end()) {
        throw XmlDocumentException("元素不存在: " + elementId);
    }

    pugi::xml_node node = it->second;

    // 移除所有PCDATA子节点
    pugi::xml_node child = node.first_child();
    while (child) {
        pugi::xml_node next = child.next_sibling();
        if (child.type() == pugi::node_pcdata) {
            node.remove_child(child);
        }
        child = next;
    }

    // 如果新文本非空，添加PCDATA子节点
    if (!text.empty()) {
        node.append_child(pugi::node_pcdata).set_value(text.c_str());
    }

    // 编辑文本不改变ID映射
}

void XmlDocumentWrapper::deleteElement(const std::string& elementId) {
    auto it = idToNodeMap_.find(elementId);
    if (it == idToNodeMap_.end()) {
        throw XmlDocumentException("元素不存在: " + elementId);
    }

    if (isRootNode(elementId)) {
        throw XmlDocumentException("不能删除根元素");
    }

    pugi::xml_node node = it->second;
    pugi::xml_node parent = node.parent();
    parent.remove_child(node);

    rebuildIdMap();
}

bool XmlDocumentWrapper::isRootNode(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it == idToNodeMap_.end()) {
        return false;
    }
    pugi::xml_node rootNode = root();
    return it->second == rootNode;
}

std::string XmlDocumentWrapper::getNodeXml(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it == idToNodeMap_.end()) {
        return "";
    }

    std::ostringstream oss;
    it->second.print(oss, "");
    return oss.str();
}

std::string XmlDocumentWrapper::getParentId(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it == idToNodeMap_.end()) {
        return "";
    }

    pugi::xml_node parent = it->second.parent();
    if (!parent) {
        return "";
    }

    pugi::xml_attribute idAttr = parent.attribute("id");
    if (idAttr.empty()) {
        return "";
    }
    return idAttr.value();
}

int XmlDocumentWrapper::getChildIndex(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it == idToNodeMap_.end()) {
        return -1;
    }

    pugi::xml_node node = it->second;
    int index = 0;
    pugi::xml_node parent = node.parent();
    for (pugi::xml_node child : parent.children()) {
        if (child.type() == pugi::node_element) {
            if (child == node) {
                return index;
            }
            ++index;
        }
    }
    return -1;
}

std::string XmlDocumentWrapper::getRootId() const {
    pugi::xml_node rootNode = root();
    if (!rootNode) return "";
    pugi::xml_attribute idAttr = rootNode.attribute("id");
    return idAttr.empty() ? "" : std::string(idAttr.value());
}

std::vector<std::string> XmlDocumentWrapper::getChildIds(const std::string& parentId) const {
    std::vector<std::string> childIds;
    auto it = idToNodeMap_.find(parentId);
    if (it == idToNodeMap_.end()) return childIds;

    for (pugi::xml_node child : it->second.children()) {
        if (child.type() == pugi::node_element) {
            pugi::xml_attribute idAttr = child.attribute("id");
            if (!idAttr.empty()) {
                childIds.push_back(idAttr.value());
            }
        }
    }
    return childIds;
}

std::vector<std::pair<std::string, std::string>> XmlDocumentWrapper::getNodeAttributes(const std::string& id) const {
    std::vector<std::pair<std::string, std::string>> attrs;
    auto it = idToNodeMap_.find(id);
    if (it == idToNodeMap_.end()) return attrs;

    for (pugi::xml_attribute attr : it->second.attributes()) {
        attrs.emplace_back(attr.name(), attr.value());
    }
    return attrs;
}
