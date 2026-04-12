#include "WorkSpaceCommand.h"
#include "WorkSpace.h"
#include <iostream>

// LoadCommand实现
LoadCommand::LoadCommand(const std::string& fileName) : fileName_(fileName), wasOpen_(false) {}

void LoadCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("LoadCommand: No workspace associated");
    }
    wasOpen_ = workspace_->isFileOpen(fileName_);
    workspace_->loadFile(fileName_);
}

void LoadCommand::undo() {
    if (!workspace_) {
        throw std::runtime_error("LoadCommand: No workspace associated");
    }
    // 如果文件之前未打开，则关闭它
    if (!wasOpen_ && workspace_->isFileOpen(fileName_)) {
        workspace_->closeFile(fileName_);
    }
    // 如果文件之前已打开，我们无法恢复到之前的内容，所以什么都不做
}

bool LoadCommand::isReadOnly() const {
    return false; // 加载文件会修改工作区状态
}

// SaveCommand实现
SaveCommand::SaveCommand(const std::string& target) : target_(target) {}

void SaveCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("SaveCommand: No workspace associated");
    }

    if (target_.empty()) {
        // 保存当前活动文件
        const std::string& activeFile = workspace_->getActiveFileName();
        if (activeFile.empty()) {
            throw std::runtime_error("SaveCommand: No active file to save");
        }
        workspace_->saveFile(activeFile);
    } else if (target_ == "all") {
        // 保存所有文件
        workspace_->saveAllFiles();
    } else {
        // 保存指定文件
        workspace_->saveFile(target_);
    }
}

void SaveCommand::undo() {
    // 保存操作的撤销通常无法实现，因为文件系统状态已改变
    // 可以留空或抛出异常
    throw std::runtime_error("SaveCommand undo not supported");
}

bool SaveCommand::isReadOnly() const {
    return false; // 保存文件可能会修改文件系统状态
}

// InitCommand实现
InitCommand::InitCommand(const std::string& fileName, bool withLog)
    : fileName_(fileName), withLog_(withLog), wasOpen_(false) {}

void InitCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("InitCommand: No workspace associated");
    }
    wasOpen_ = workspace_->isFileOpen(fileName_);
    workspace_->initFile(fileName_, withLog_);
}

void InitCommand::undo() {
    if (!workspace_) {
        throw std::runtime_error("InitCommand: No workspace associated");
    }
    // 如果文件之前未打开，则关闭它
    if (!wasOpen_ && workspace_->isFileOpen(fileName_)) {
        workspace_->closeFile(fileName_);
    }
    // 如果文件之前已打开，我们无法恢复到之前的内容，所以什么都不做
}

bool InitCommand::isReadOnly() const {
    return false; // 初始化缓冲区会修改工作区状态
}

// CloseCommand实现
CloseCommand::CloseCommand(const std::string& fileName) : fileName_(fileName) {}

void CloseCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("CloseCommand: No workspace associated");
    }

    std::string targetFile = fileName_;
    if (targetFile.empty()) {
        // 关闭当前活动文件
        targetFile = workspace_->getActiveFileName();
        if (targetFile.empty()) {
            throw std::runtime_error("CloseCommand: No active file to close");
        }
    }

    // 检查文件是否已打开
    if (!workspace_->isFileOpen(targetFile)) {
        throw std::runtime_error("CloseCommand: File not open: " + targetFile);
    }

    // 检查文件是否已修改
    if (workspace_->isFileModified(targetFile)) {
        // 根据要求，应该提示用户保存
        // 由于无法交互，我们抛出异常，要求用户先保存
        throw std::runtime_error("CloseCommand: File '" + targetFile + "' has been modified. Please save before closing.");
    }

    // 关闭文件
    workspace_->closeFile(targetFile);
}

void CloseCommand::undo() {
    // 关闭操作的撤销需要重新打开文件，但文件内容可能已丢失
    // 暂时不支持撤销关闭操作
    throw std::runtime_error("CloseCommand undo not supported");
}

bool CloseCommand::isReadOnly() const {
    return false; // 关闭文件会修改工作区状态
}

// EditCommand实现
EditCommand::EditCommand(const std::string& fileName) : fileName_(fileName) {}

void EditCommand::execute() {
    if (!workspace_) {
        throw std::runtime_error("EditCommand: No workspace associated");
    }

    // 检查文件是否已打开
    if (!workspace_->isFileOpen(fileName_)) {
        throw std::runtime_error("文件未打开: " + fileName_);
    }

    // 记录之前的活动文件
    previousActiveFile_ = workspace_->getActiveFileName();

    // 切换活动文件
    workspace_->setActiveFile(fileName_);
}

void EditCommand::undo() {
    if (!workspace_) {
        throw std::runtime_error("EditCommand: No workspace associated");
    }

    // 切换回之前的活动文件（如果仍然打开）
    if (!previousActiveFile_.empty() && workspace_->isFileOpen(previousActiveFile_)) {
        workspace_->setActiveFile(previousActiveFile_);
    }
    // 如果之前的活动文件已关闭，则什么都不做（保持当前活动文件）
}

bool EditCommand::isReadOnly() const {
    return false; // 切换活动文件会修改工作区状态
}

// EditorListCommand实现
EditorListCommand::EditorListCommand() {}

void EditorListCommand::execute() {
    std::cout << "EditorListCommand: Listing all open editors" << std::endl;
    // TODO: 实际的编辑器列表显示逻辑
}

void EditorListCommand::undo() {
    std::cout << "EditorListCommand: Undo listing (nothing to undo)" << std::endl;
    // 显示列表是只读操作，不需要真正的撤销
}

bool EditorListCommand::isReadOnly() const {
    return true; // 显示列表是只读操作
}

// DirTreeCommand实现
DirTreeCommand::DirTreeCommand(const std::string& path) : path_(path) {}

void DirTreeCommand::execute() {
    if (path_.empty()) {
        std::cout << "DirTreeCommand: Showing directory tree of current directory" << std::endl;
    } else {
        std::cout << "DirTreeCommand: Showing directory tree of '" << path_ << "'" << std::endl;
    }
    // TODO: 实际的目录树显示逻辑
}

void DirTreeCommand::undo() {
    std::cout << "DirTreeCommand: Undo showing directory tree (nothing to undo)" << std::endl;
    // 显示目录树是只读操作，不需要真正的撤销
}

bool DirTreeCommand::isReadOnly() const {
    return true; // 显示目录树是只读操作
}

// UndoCommand实现
UndoCommand::UndoCommand() {}

void UndoCommand::execute() {
    std::cout << "UndoCommand: Performing undo" << std::endl;
    // TODO: 实际的撤销逻辑
}

void UndoCommand::undo() {
    std::cout << "UndoCommand: Undo undo (redo) - this should not normally be called" << std::endl;
    // TODO: 实际的撤销撤销逻辑（实际上是重做）
}

bool UndoCommand::isReadOnly() const {
    return false; // 撤销操作会修改状态
}

// RedoCommand实现
RedoCommand::RedoCommand() {}

void RedoCommand::execute() {
    std::cout << "RedoCommand: Performing redo" << std::endl;
    // TODO: 实际的重做逻辑
}

void RedoCommand::undo() {
    std::cout << "RedoCommand: Undo redo (undo) - this should not normally be called" << std::endl;
    // TODO: 实际的撤销重做逻辑（实际上是撤销）
}

bool RedoCommand::isReadOnly() const {
    return false; // 重做操作会修改状态
}

// ExitCommand实现
ExitCommand::ExitCommand() {}

void ExitCommand::execute() {
    std::cout << "ExitCommand: Exiting program" << std::endl;
    // TODO: 实际的退出逻辑（清理资源等）
}

void ExitCommand::undo() {
    std::cout << "ExitCommand: Undo exit - this should not normally be called" << std::endl;
    // 退出程序通常无法撤销
}

bool ExitCommand::isReadOnly() const {
    return false; // 退出程序会修改程序状态
}