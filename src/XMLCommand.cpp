#include "XMLCommand.h"
#include "XmlEditor.h"
#include "IXmlDocument.h"
#include "CommandFactory.h"

// 自注册：XML 编辑器命令工厂
namespace {
    REGISTER_XML_CMD(EditorCommandType::InsertBefore, InsertBeforeCommand,
                     ed.tagName && ed.newId && ed.targetId,
                     *ed.tagName, *ed.newId, *ed.targetId, ed.text.value_or(""))
    REGISTER_XML_CMD(EditorCommandType::AppendChild, AppendChildCommand,
                     ed.tagName && ed.newId && ed.targetId,
                     *ed.tagName, *ed.newId, *ed.targetId, ed.text.value_or(""))
    REGISTER_XML_CMD(EditorCommandType::EditId, EditIdCommand,
                     ed.targetId && ed.newId,
                     *ed.targetId, *ed.newId)
    REGISTER_XML_CMD(EditorCommandType::EditText_, EditTextCommand,
                     ed.targetId,
                     *ed.targetId, ed.text.value_or(""))
    REGISTER_XML_CMD(EditorCommandType::XmlDelete, XmlDeleteCommand,
                     ed.targetId,
                     *ed.targetId)
}

// XMLCommand 基类
XMLCommand::XMLCommand(IXmlDocument* doc)
    : doc_(doc) {
}

// ---- InsertBeforeCommand ----

InsertBeforeCommand::InsertBeforeCommand(IXmlDocument* doc,
                                         const std::string& tagName,
                                         const std::string& newId,
                                         const std::string& targetId,
                                         const std::string& text)
    : XMLCommand(doc)
    , tagName_(tagName)
    , newId_(newId)
    , targetId_(targetId)
    , text_(text) {
}

void InsertBeforeCommand::execute() {
    doc_->insertBefore(tagName_, newId_, targetId_, text_);
    executed_ = true;
}

void InsertBeforeCommand::undo() {
    if (!executed_) return;
    doc_->deleteElement(newId_);
    executed_ = false;
}

// ---- AppendChildCommand ----

AppendChildCommand::AppendChildCommand(IXmlDocument* doc,
                                       const std::string& tagName,
                                       const std::string& newId,
                                       const std::string& parentId,
                                       const std::string& text)
    : XMLCommand(doc)
    , tagName_(tagName)
    , newId_(newId)
    , parentId_(parentId)
    , text_(text) {
}

void AppendChildCommand::execute() {
    doc_->appendChild(tagName_, newId_, parentId_, text_);
    executed_ = true;
}

void AppendChildCommand::undo() {
    if (!executed_) return;
    doc_->deleteElement(newId_);
    executed_ = false;
}

// ---- EditIdCommand ----

EditIdCommand::EditIdCommand(IXmlDocument* doc,
                             const std::string& oldId,
                             const std::string& newId)
    : XMLCommand(doc)
    , oldId_(oldId)
    , newId_(newId) {
}

void EditIdCommand::execute() {
    doc_->editId(oldId_, newId_);
    executed_ = true;
}

void EditIdCommand::undo() {
    if (!executed_) return;
    doc_->editId(newId_, oldId_);
    executed_ = false;
}

// ---- EditTextCommand ----

EditTextCommand::EditTextCommand(IXmlDocument* doc,
                                 const std::string& elementId,
                                 const std::string& text)
    : XMLCommand(doc)
    , elementId_(elementId)
    , newText_(text) {
}

void EditTextCommand::execute() {
    oldText_ = doc_->getNodeValue(elementId_);
    doc_->editText(elementId_, newText_);
    executed_ = true;
}

void EditTextCommand::undo() {
    if (!executed_) return;
    doc_->editText(elementId_, oldText_);
    executed_ = false;
}

// ---- XmlDeleteCommand ----

XmlDeleteCommand::XmlDeleteCommand(IXmlDocument* doc, const std::string& elementId)
    : XMLCommand(doc)
    , elementId_(elementId) {
}

void XmlDeleteCommand::execute() {
    savedXml_ = doc_->getNodeXml(elementId_);
    parentId_ = doc_->getParentId(elementId_);
    tagName_ = doc_->getNodeName(elementId_);
    oldText_ = doc_->getNodeValue(elementId_);
    childIndex_ = doc_->getChildIndex(elementId_);

    doc_->deleteElement(elementId_);
    executed_ = true;
}

void XmlDeleteCommand::undo() {
    if (!executed_) return;
    if (!parentId_.empty()) {
        doc_->appendChild(tagName_, elementId_, parentId_, oldText_);
    }
    executed_ = false;
}
