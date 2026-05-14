#include "WorkSpaceCommand.h"
#include "WorkSpace.h"
#include "TextEditor.h"
#include "OutputService.h"
#include "CommandFactory.h"
#include "EditDurationTracker.h"
#include "EditDurationDecorator.h"
#include <iostream>
#include <vector>
#include <algorithm>

// 自注册：工作区命令工厂
namespace {
    REGISTER_WS_CMD_REQ_FILENAME(WorkSpaceCommandType::Load, LoadCommand)
    REGISTER_WS_CMD_TARGET(WorkSpaceCommandType::Save, SaveCommand)
    REGISTER_WS_CMD_INIT(WorkSpaceCommandType::Init, InitCommand)
    REGISTER_WS_CMD_FILENAME(WorkSpaceCommandType::Close, CloseCommand)
    REGISTER_WS_CMD_REQ_FILENAME(WorkSpaceCommandType::Edit, EditCommand)
    REGISTER_WS_CMD_TARGET(WorkSpaceCommandType::EditorList, EditorListCommand)
    REGISTER_WS_CMD_PATH(WorkSpaceCommandType::DirTree, DirTreeCommand)
    REGISTER_WS_CMD_NOARGS(WorkSpaceCommandType::Undo, UndoCommand)
    REGISTER_WS_CMD_NOARGS(WorkSpaceCommandType::Redo, RedoCommand)
    REGISTER_WS_CMD_NOARGS(WorkSpaceCommandType::Exit, ExitCommand)
}

#include "FilesystemCompat.h"

// WorkSpaceCommand protected helpers

void WorkSpaceCommand::checkWorkSpace() const {
    if (!workspace_) {
        throw std::runtime_error("No workspace associated");
    }
}

std::shared_ptr<Editor> WorkSpaceCommand::getActiveEditorOrThrow() const {
    auto editor = workspace_->getActiveEditor();
    if (!editor) {
        throw std::runtime_error("No active editor");
    }
    return editor;
}

// LoadCommand实现
LoadCommand::LoadCommand(const std::string& fileName) : fileName_(fileName), wasOpen_(false) {}

void LoadCommand::execute() {
    checkWorkSpace();
    wasOpen_ = workspace_->isFileOpen(fileName_);
    workspace_->loadFile(fileName_);
}

void LoadCommand::undo() {
    checkWorkSpace();
    if (!wasOpen_ && workspace_->isFileOpen(fileName_)) {
        workspace_->closeFile(fileName_);
    }
}

bool LoadCommand::isReadOnly() const {
    return false; // 加载文件会修改工作区状态
}

// SaveCommand实现
SaveCommand::SaveCommand(const std::string& target) : target_(target) {}

void SaveCommand::execute() {
    checkWorkSpace();

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
    checkWorkSpace();
    wasOpen_ = workspace_->isFileOpen(fileName_);
    workspace_->initFile(fileName_, withLog_);
}

void InitCommand::undo() {
    checkWorkSpace();
    if (!wasOpen_ && workspace_->isFileOpen(fileName_)) {
        workspace_->closeFile(fileName_);
    }
}

bool InitCommand::isReadOnly() const {
    return false; // 初始化缓冲区会修改工作区状态
}

// CloseCommand实现
CloseCommand::CloseCommand(const std::string& fileName) : fileName_(fileName) {}

void CloseCommand::execute() {
    checkWorkSpace();

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

    // 停止日志记录（如果正在记录）
    workspace_->stopLoggingForFile(targetFile);

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
    checkWorkSpace();

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
    checkWorkSpace();

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
EditorListCommand::EditorListCommand(const std::string& mode)
    : treeMode_(mode == "tree") {
}

void EditorListCommand::execute() {
    checkWorkSpace();

    auto fileInfos = workspace_->getFileInfoList();
    EditDurationDecorator decorator(workspace_->getEditDurationTracker());

    if (treeMode_) {
        auto root = std::make_shared<TreeNode>("Open Files", true);
        for (const auto& info : fileInfos) {
            root->children.push_back(decorator.decorateFileNode(info));
        }
        workspace_->outputTree(*root);
    } else {
        std::vector<FileInfo> decoratedInfos;
        decoratedInfos.reserve(fileInfos.size());
        for (const auto& info : fileInfos) {
            decoratedInfos.push_back(decorator.decorateFileInfo(info));
        }
        workspace_->outputList(decoratedInfos);
    }
}

void EditorListCommand::undo() {
    std::cout << "EditorListCommand: Undo listing (nothing to undo)" << std::endl;
}

bool EditorListCommand::isReadOnly() const {
    return true;
}

// DirTreeCommand实现
DirTreeCommand::DirTreeCommand(const std::string& path) : path_(path) {}

void DirTreeCommand::execute() {
    checkWorkSpace();

    // 获取结构化目录树
    auto treeRoot = workspace_->getDirectoryTreeStructure(path_);

    workspace_->outputTree(*treeRoot);
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
    checkWorkSpace();
    auto activeEditor = getActiveEditorOrThrow();

    if (!activeEditor->canUndo()) {
        throw std::runtime_error("UndoCommand: Nothing to undo");
    }

    activeEditor->undo();
}

void UndoCommand::undo() {
    checkWorkSpace();
    auto activeEditor = getActiveEditorOrThrow();
    activeEditor->redo();
}

bool UndoCommand::isReadOnly() const {
    return false; // 撤销操作会修改状态
}

// RedoCommand实现
RedoCommand::RedoCommand() {}

void RedoCommand::execute() {
    checkWorkSpace();
    auto activeEditor = getActiveEditorOrThrow();

    if (!activeEditor->canRedo()) {
        throw std::runtime_error("RedoCommand: Nothing to redo");
    }

    activeEditor->redo();
}

void RedoCommand::undo() {
    checkWorkSpace();
    auto activeEditor = getActiveEditorOrThrow();
    activeEditor->undo();
}

bool RedoCommand::isReadOnly() const {
    return false; // 重做操作会修改状态
}

// ExitCommand实现
ExitCommand::ExitCommand() {}

void ExitCommand::ensureNoUnsavedFiles() {
    std::vector<std::string> unsavedFiles;
    auto openFiles = workspace_->getOpenFiles();
    for (const auto& fileName : openFiles) {
        if (workspace_->isFileModified(fileName)) {
            unsavedFiles.push_back(fileName);
        }
    }

    if (!unsavedFiles.empty()) {
        std::string errorMsg = "ExitCommand: The following files have unsaved changes:\n";
        for (const auto& fileName : unsavedFiles) {
            errorMsg += "  " + fileName + "\n";
        }
        errorMsg += "Please save them before exiting.";
        workspace_->outputError(errorMsg);
        throw std::runtime_error(errorMsg);
    }
}

void ExitCommand::trySaveConfig() {
    try {
        workspace_->saveConfig(".editor_config");
    } catch (const std::exception& e) {
        workspace_->outputError("Warning: Failed to save configuration: " + std::string(e.what()));
    }
}

void ExitCommand::execute() {
    checkWorkSpace();
    ensureNoUnsavedFiles();
    trySaveConfig();
    workspace_->requestExit();
    workspace_->outputLine("ExitCommand: All files saved. Exiting program...");
}

void ExitCommand::undo() {
    std::cout << "ExitCommand: Undo exit - this should not normally be called" << std::endl;
    // 退出程序通常无法撤销
}

bool ExitCommand::isReadOnly() const {
    return false; // 退出程序会修改程序状态
}