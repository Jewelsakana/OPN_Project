#include "XmlEditor.h"
#include "XMLCommand.h"
#include "EditorFactory.h"
#include "XmlDocumentWrapper.h"

namespace {
    REGISTER_EDITOR(".xml", XmlEditor)
}

XmlEditor::XmlEditor()
    : modified_(false) {
    document_ = std::make_unique<XmlDocumentWrapper>();
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

IXmlDocument& XmlEditor::getDocument() {
    return *document_;
}

const IXmlDocument& XmlEditor::getDocument() const {
    return *document_;
}

void XmlEditor::loadFromFile(const std::string& filePath) {
    document_->loadFromFile(filePath);
    document_->collectIds();
    setModified(false);
}

void XmlEditor::loadFromString(const std::string& xmlContent) {
    document_->loadFromString(xmlContent);
    document_->collectIds();
    setModified(false);
}

void XmlEditor::saveToFile(const std::string& filePath) {
    document_->saveToFile(filePath);
    setModified(false);
}

bool XmlEditor::hasNodeWithId(const std::string& id) const {
    return document_->hasNodeWithId(id);
}

std::string XmlEditor::getNodeName(const std::string& id) const {
    return document_->getNodeName(id);
}

std::string XmlEditor::getNodeValue(const std::string& id) const {
    return document_->getNodeValue(id);
}

std::string XmlEditor::getNodeAttribute(const std::string& id, const std::string& attrName) const {
    return document_->getNodeAttribute(id, attrName);
}

std::vector<std::string> XmlEditor::getAllIds() const {
    return document_->getAllIds();
}

bool XmlEditor::supportsCommand(EditorCommandType type) const {
    switch (type) {
        case EditorCommandType::InsertBefore:
        case EditorCommandType::AppendChild:
        case EditorCommandType::EditId:
        case EditorCommandType::EditText_:
        case EditorCommandType::XmlDelete:
        case EditorCommandType::XmlTree:
            return true;
        default:
            return false;
    }
}

void XmlEditor::clear() {
    document_->clear();
    modified_ = false;
}
