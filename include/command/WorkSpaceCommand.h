#ifndef WORKSPACECOMMAND_H
#define WORKSPACECOMMAND_H

#include "Command.h"
#include "Editor.h"
#include <string>
#include <memory>
#include <stdexcept>

// 前向声明
class WorkSpace;
class TextEditor;

// WorkSpaceCommand基类：所有工作区命令的基类
// 派生自Command，用于区分工作区命令和编辑器命令
class WorkSpaceCommand : public Command {
public:
    virtual ~WorkSpaceCommand() = default;

    // 设置关联的工作区
    void setWorkSpace(WorkSpace* workspace) override { workspace_ = workspace; }

    // 工作区命令是工作区级别
    bool isWorkSpaceLevel() const override { return true; }

protected:
    WorkSpace* workspace_ = nullptr; // 关联的工作区实例

    // 检查workspace是否已关联，未关联则抛出异常
    void checkWorkSpace() const;

    // 获取活动编辑器，未打开或非TextEditor则抛出异常
    std::shared_ptr<Editor> getActiveEditorOrThrow() const;
};

// 加载文件命令
class LoadCommand : public WorkSpaceCommand {
public:
    explicit LoadCommand(const std::string& fileName);
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_;
    bool wasOpen_ = false; // 执行前文件是否已打开
};

// 保存文件命令
class SaveCommand : public WorkSpaceCommand {
public:
    explicit SaveCommand(const std::string& target = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string target_; // 空字符串表示保存所有，否则保存指定文件
};

// 创建新缓冲区命令
class InitCommand : public WorkSpaceCommand {
public:
    explicit InitCommand(const std::string& fileName, bool withLog = false);
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_;
    bool withLog_;
    bool wasOpen_ = false; // 执行前文件是否已打开
};

// 关闭文件命令
class CloseCommand : public WorkSpaceCommand {
public:
    explicit CloseCommand(const std::string& fileName = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_; // 空字符串表示关闭当前活动文件
};

// 切换活动文件命令
class EditCommand : public WorkSpaceCommand {
public:
    explicit EditCommand(const std::string& fileName);
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string fileName_;
    std::string previousActiveFile_; // 执行前的活动文件
};

// 显示文件列表命令
class EditorListCommand : public WorkSpaceCommand {
public:
    explicit EditorListCommand(const std::string& mode = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    bool treeMode_;
};

// 显示目录树命令
class DirTreeCommand : public WorkSpaceCommand {
public:
    explicit DirTreeCommand(const std::string& path = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
private:
    std::string path_; // 空字符串表示当前目录
};

// 撤销命令
class UndoCommand : public WorkSpaceCommand {
public:
    UndoCommand();
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
};

// 重做命令
class RedoCommand : public WorkSpaceCommand {
public:
    RedoCommand();
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;
};

// 退出程序命令
class ExitCommand : public WorkSpaceCommand {
public:
    ExitCommand();
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;

private:
    void ensureNoUnsavedFiles();
    void trySaveConfig();
};

#endif // WORKSPACECOMMAND_H