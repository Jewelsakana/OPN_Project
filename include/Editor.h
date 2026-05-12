#ifndef EDITOR_H
#define EDITOR_H

#include <memory>
#include <string>
#include <vector>
#include "CommandParser.h"
#include "ISpellChecker.h"

// 前向声明
class Command;

// Editor接口：所有编辑器的基类
class Editor {
public:
    virtual ~Editor() = default;

    // 执行命令
    virtual void executeCommand(std::unique_ptr<Command> command) = 0;

    // Undo/Redo操作（默认空实现，子类可重写）
    virtual void undo() {}
    virtual void redo() {}
    virtual bool canUndo() const { return false; }
    virtual bool canRedo() const { return false; }

    // 检查是否支持某个编辑器命令类型
    virtual bool supportsCommand(EditorCommandType type) const = 0;

    // 文件内容序列化/反序列化（多态方法，消除FileCoordinator中的dynamic_cast分支）
    virtual void loadFromData(const std::string& content) {}
    virtual std::string saveToData() const { return ""; }
    virtual void initContent(bool withLog) {}

    // 查询是否已修改
    virtual bool isModified() const { return false; }
    virtual void setModified(bool modified) {}

    // 获取待检查的文本片段列表（用于拼写检查）
    // TextEditor：每行返回一个 TextSegment
    // XmlEditor：遍历 XML 树，仅提取可检查的文本节点内容
    virtual std::vector<TextSegment> getTextsToCheck() const { return {}; }
};

#endif // EDITOR_H