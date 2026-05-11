#ifndef XMLCOMMAND_H
#define XMLCOMMAND_H

#include "Command.h"
#include <string>

// 前向声明
class IXmlDocument;

// XMLCommand：XML编辑命令基类，直接绑定IXmlDocument
class XMLCommand : public Command {
public:
    explicit XMLCommand(IXmlDocument* doc);
    ~XMLCommand() override = default;

    void execute() override = 0;
    void undo() override = 0;

protected:
    IXmlDocument* doc_;  // 不持有所有权
};

// InsertBeforeCommand：在目标元素前插入新元素
class InsertBeforeCommand : public XMLCommand {
public:
    InsertBeforeCommand(IXmlDocument* doc,
                        const std::string& tagName,
                        const std::string& newId,
                        const std::string& targetId,
                        const std::string& text);
    void execute() override;
    void undo() override;

private:
    std::string tagName_;
    std::string newId_;
    std::string targetId_;
    std::string text_;
    bool executed_ = false;
};

// AppendChildCommand：在父元素内追加子元素
class AppendChildCommand : public XMLCommand {
public:
    AppendChildCommand(IXmlDocument* doc,
                       const std::string& tagName,
                       const std::string& newId,
                       const std::string& parentId,
                       const std::string& text);
    void execute() override;
    void undo() override;

private:
    std::string tagName_;
    std::string newId_;
    std::string parentId_;
    std::string text_;
    bool executed_ = false;
};

// EditIdCommand：修改元素ID
class EditIdCommand : public XMLCommand {
public:
    EditIdCommand(IXmlDocument* doc,
                  const std::string& oldId,
                  const std::string& newId);
    void execute() override;
    void undo() override;

private:
    std::string oldId_;
    std::string newId_;
    bool executed_ = false;
};

// EditTextCommand：修改元素文本
class EditTextCommand : public XMLCommand {
public:
    EditTextCommand(IXmlDocument* doc,
                    const std::string& elementId,
                    const std::string& text);
    void execute() override;
    void undo() override;

private:
    std::string elementId_;
    std::string newText_;
    std::string oldText_;   // 保存修改前的文本
    bool executed_ = false;
};

// XmlDeleteCommand：删除元素
class XmlDeleteCommand : public XMLCommand {
public:
    XmlDeleteCommand(IXmlDocument* doc, const std::string& elementId);
    void execute() override;
    void undo() override;

private:
    std::string elementId_;
    // 保存删除前的快照用于undo
    std::string savedXml_;
    std::string parentId_;
    std::string tagName_;
    std::string oldText_;
    int childIndex_ = -1;
    bool executed_ = false;
};

#endif // XMLCOMMAND_H
