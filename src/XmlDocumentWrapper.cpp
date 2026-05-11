#include "XmlDocumentWrapper.h"
#include <fstream>
#include <sstream>
#include <functional>

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
