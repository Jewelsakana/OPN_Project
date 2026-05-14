#ifndef EDITOR_H
#define EDITOR_H

#include <memory>
#include <string>
#include <vector>
#include "CommandParser.h"
#include "TextSegment.h"

// 前向声明
class Command;
struct EditorCommandContext;

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

    // 检查是否支持某个编辑器命令类型（CommandTypeId 支持插件运行时注册的类型）
    virtual bool supportsCommand(CommandTypeId type) const = 0;

    // 文件内容序列化/反序列化（多态方法，消除FileCoordinator中的dynamic_cast分支）
    virtual void loadFromData(const std::string& content) {}
    virtual std::string saveToData() const { return ""; }
    virtual void initContent(bool withLog) {}

    // 查询是否已修改
    virtual bool isModified() const { return false; }
    virtual void setModified(bool modified) {}

    // 获取待检查的文本片段列表（用于拼写检查）
    virtual std::vector<TextSegment> getTextsToCheck() const { return {}; }

    // 填充编辑器命令上下文（由子类实现，消除CommandFactory中的dynamic_cast分支）
    virtual void populateContext(EditorCommandContext& ctx) {}

    // 初始化编辑器内部组件（由子类实现，消除WorkSpace中的dynamic_cast分支）
    virtual void initialize() {}
};

#endif // EDITOR_H