#include "XmlEditor.h"
#include "XMLEngine.h"
#include "XMLCommand.h"
#include "EditorFactory.h"

namespace {
    REGISTER_EDITOR(".xml", XmlEditor)
}

XmlEditor::XmlEditor()
    : modified_(false) {
    xmlEngine_ = std::make_shared<XMLEngine>(this);
}

XmlEditor::~XmlEditor() {
}

void XmlEditor::executeCommand(std::unique_ptr<Command> command) {
    try {
        commandManager_.executeCommand(std::move(command));
        setModified(true);
    } catch (const std::exception& e) {
        throw;
    }
}

void XmlEditor::undo() {
    commandManager_.undo();
}

void XmlEditor::redo() {
    commandManager_.redo();
    setModified(true);
}

bool XmlEditor::canUndo() const {
    return commandManager_.canUndo();
}

bool XmlEditor::canRedo() const {
    return commandManager_.canRedo();
}

bool XmlEditor::isModified() const {
    return modified_;
}

void XmlEditor::setModified(bool modified) {
    this->modified_ = modified;
}

XMLEngine* XmlEditor::getXMLEngine() const {
    return xmlEngine_.get();
}

XmlDocumentWrapper& XmlEditor::getDocument() {
    return document_;
}

const XmlDocumentWrapper& XmlEditor::getDocument() const {
    return document_;
}

void XmlEditor::loadFromFile(const std::string& filePath) {
    document_.loadFromFile(filePath);
    buildIdMapping();
    setModified(false);
}

void XmlEditor::loadFromString(const std::string& xmlContent) {
    document_.loadFromString(xmlContent);
    buildIdMapping();
    setModified(false);
}

void XmlEditor::saveToFile(const std::string& filePath) {
    document_.saveToFile(filePath);
    setModified(false);
}

pugi::xml_node XmlEditor::findNodeById(const std::string& id) const {
    auto it = idToNodeMap_.find(id);
    if (it != idToNodeMap_.end()) {
        return it->second;
    }
    return pugi::xml_node();  // 返回空节点
}

bool XmlEditor::hasNodeWithId(const std::string& id) const {
    return idToNodeMap_.find(id) != idToNodeMap_.end();
}

std::vector<std::string> XmlEditor::getAllIds() const {
    std::vector<std::string> ids;
    ids.reserve(idToNodeMap_.size());
    for (const auto& pair : idToNodeMap_) {
        ids.push_back(pair.first);
    }
    return ids;
}

void XmlEditor::rebuildIdMapping() {
    buildIdMapping();
}

void XmlEditor::clear() {
    document_.clear();
    idToNodeMap_.clear();
    modified_ = false;
}

void XmlEditor::buildIdMapping() {
    document_.collectIds(idToNodeMap_);
}
