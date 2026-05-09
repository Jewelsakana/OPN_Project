#include "WorkSpaceCommand.h"
#include "WorkSpace.h"
#include "TextEditor.h"
#include "OutputService.h"
#include "CommandFactory.h"
#include <iostream>
#include <vector>
#include <algorithm>

// 自注册：工作区命令工厂
namespace {
    static bool _reg_load = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Load,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                if (ws.fileName)
                    return std::make_unique<LoadCommand>(*ws.fileName);
                return nullptr;
            });
        return true;
    }();

    static bool _reg_save = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Save,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                return std::make_unique<SaveCommand>(ws.target.value_or(""));
            });
        return true;
    }();

    static bool _reg_init = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Init,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                if (ws.fileName)
                    return std::make_unique<InitCommand>(*ws.fileName, ws.withLog.value_or(false));
                return nullptr;
            });
        return true;
    }();

    static bool _reg_close = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Close,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                return std::make_unique<CloseCommand>(ws.fileName.value_or(""));
            });
        return true;
    }();

    static bool _reg_edit = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Edit,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                if (ws.fileName)
                    return std::make_unique<EditCommand>(*ws.fileName);
                return nullptr;
            });
        return true;
    }();

    static bool _reg_editorlist = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::EditorList,
            [](const WorkSpaceParsedCommand&) -> std::unique_ptr<Command> {
                return std::make_unique<EditorListCommand>();
            });
        return true;
    }();

    static bool _reg_dirtree = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::DirTree,
            [](const WorkSpaceParsedCommand& ws) -> std::unique_ptr<Command> {
                return std::make_unique<DirTreeCommand>(ws.path.value_or(""));
            });
        return true;
    }();

    static bool _reg_undo = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Undo,
            [](const WorkSpaceParsedCommand&) -> std::unique_ptr<Command> {
                return std::make_unique<UndoCommand>();
            });
        return true;
    }();

    static bool _reg_redo = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Redo,
            [](const WorkSpaceParsedCommand&) -> std::unique_ptr<Command> {
                return std::make_unique<RedoCommand>();
            });
        return true;
    }();

    static bool _reg_exit = []() {
        CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType::Exit,
            [](const WorkSpaceParsedCommand&) -> std::unique_ptr<Command> {
                return std::make_unique<ExitCommand>();
            });
        return true;
    }();
}

// 尝试使用filesystem库
#if __has_include(<filesystem>) && (!defined(__GNUC__) || __GNUC__ >= 9)
// GCC 8的filesystem实现有问题，使用实验版本
#  include <filesystem>
   namespace fs = std::filesystem;
#  define HAS_FILESYSTEM 1
#elif __has_include(<experimental/filesystem>)
#  include <experimental/filesystem>
   namespace fs = std::experimental::filesystem;
#  define HAS_FILESYSTEM 1
#else
#  define HAS_FILESYSTEM 0
#endif

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
EditorListCommand::EditorListCommand() {}

void EditorListCommand::execute() {
    checkWorkSpace();

    // 获取结构化文件信息列表
    auto fileInfos = workspace_->getFileInfoList();

    // 通过OutputService输出
    workspace_->getOutputService().outputList(fileInfos);
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
    checkWorkSpace();

    // 获取结构化目录树
    auto treeRoot = workspace_->getDirectoryTreeStructure(path_);

    // 通过OutputService输出
    workspace_->getOutputService().outputTree(*treeRoot);
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

void ExitCommand::execute() {
    checkWorkSpace();

    auto& outputService = workspace_->getOutputService();

    // 检查是否有未保存的文件
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

        // 使用OutputService输出错误信息
        outputService.outputError(errorMsg);
        throw std::runtime_error(errorMsg);
    }

    // 所有文件已保存，可以退出
    // 首先保存工作区配置到文件
    try {
        workspace_->saveConfig(".editor_config");
    } catch (const std::exception& e) {
        // 配置保存失败不应该阻止退出，但记录警告
        outputService.outputError("Warning: Failed to save configuration: " + std::string(e.what()));
    }

    // 设置退出标志，由上层调用者处理实际退出
    workspace_->requestExit();
    outputService.outputLine("ExitCommand: All files saved. Exiting program...");
}

void ExitCommand::undo() {
    std::cout << "ExitCommand: Undo exit - this should not normally be called" << std::endl;
    // 退出程序通常无法撤销
}

bool ExitCommand::isReadOnly() const {
    return false; // 退出程序会修改程序状态
}