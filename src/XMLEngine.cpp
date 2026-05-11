#include "XMLEngine.h"
#include "XmlEditor.h"

XMLEngine::XMLEngine(XmlEditor* editor)
    : editor_(editor) {
}

void XMLEngine::validate() const {
}

bool XMLEngine::isValid() const {
    return editor_ != nullptr && editor_->getDocument().isLoaded();
}

bool XMLEngine::hasNodeWithId(const std::string& id) const {
    if (!editor_) {
        return false;
    }
    return editor_->hasNodeWithId(id);
}

std::string XMLEngine::getNodeName(const std::string& id) const {
    if (!editor_) {
        return "";
    }
    return editor_->getNodeName(id);
}

std::string XMLEngine::getNodeValue(const std::string& id) const {
    if (!editor_) {
        return "";
    }
    return editor_->getNodeValue(id);
}

std::string XMLEngine::getNodeAttribute(const std::string& id, const std::string& attrName) const {
    if (!editor_) {
        return "";
    }
    return editor_->getNodeAttribute(id, attrName);
}

bool XMLEngine::isDocumentLoaded() const {
    return editor_ && editor_->getDocument().isLoaded();
}

void XMLEngine::handleException(const std::exception& e) const {
}
