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

pugi::xml_node XmlDocumentWrapper::root() const {
    return doc_.document_element();
}

pugi::xml_document& XmlDocumentWrapper::getDocument() {
    return doc_;
}

const pugi::xml_document& XmlDocumentWrapper::getDocument() const {
    return doc_;
}

void XmlDocumentWrapper::collectIds(std::unordered_map<std::string, pugi::xml_node>& idMap) const {
    idMap.clear();

    // 递归遍历所有元素节点
    std::function<void(pugi::xml_node)> traverse = [&](pugi::xml_node node) {
        for (pugi::xml_node child : node.children()) {
            if (child.type() == pugi::node_element) {
                pugi::xml_attribute idAttr = child.attribute("id");
                if (idAttr.empty()) {
                    throw MissingIdException(child.name());
                }
                std::string id = idAttr.value();
                if (idMap.find(id) != idMap.end()) {
                    throw DuplicateIdException(id);
                }
                idMap[id] = child;
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
        if (idMap.find(id) != idMap.end()) {
            throw DuplicateIdException(id);
        }
        idMap[id] = rootNode;
        traverse(rootNode);
    }
}

bool XmlDocumentWrapper::isLoaded() const {
    return loaded_;
}

void XmlDocumentWrapper::clear() {
    doc_.reset();
    loaded_ = false;
}
