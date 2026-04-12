#include "WorkSpace.h"
#include "TextEditor.h"
#include "WorkSpaceCommand.h"
#include <algorithm>
#include <stdexcept>

// WorkspaceMemento实现

WorkspaceMemento::WorkspaceMemento(const std::vector<std::string>& openFiles,
                                   const std::string& activeFileName,
                                   const std::map<std::string, bool>& fileModifiedStates,
                                   bool logEnabled)
    : openFiles_(openFiles),
      activeFileName_(activeFileName),
      fileModifiedStates_(fileModifiedStates),
      logEnabled_(logEnabled) {
}

const std::vector<std::string>& WorkspaceMemento::getOpenFiles() const {
    return openFiles_;
}

const std::string& WorkspaceMemento::getActiveFileName() const {
    return activeFileName_;
}

const std::map<std::string, bool>& WorkspaceMemento::getFileModifiedStates() const {
    return fileModifiedStates_;
}

bool WorkspaceMemento::isLogEnabled() const {
    return logEnabled_;
}

// WorkSpace实现

WorkSpace::WorkSpace() : logEnabled_(false) {
}

WorkSpace::~WorkSpace() {
}

// 文件管理
void WorkSpace::openFile(const std::string& fileName) {
    if (openFiles_.find(fileName) == openFiles_.end()) {
        // 创建新的TextEditor实例（暂时固定为TextEditor）
        // TODO: 可能需要根据文件类型创建不同的Editor
        auto editor = std::make_shared<TextEditor>();
        openFiles_[fileName] = editor;
        fileModifiedStates_[fileName] = false;

        // 如果没有活动文件，设置为活动文件
        if (activeFileName_.empty()) {
            activeFileName_ = fileName;
        }

        // 通知观察者文件被打开
        // Event event("open", fileName);
        // notify(event);
    }
}

void WorkSpace::closeFile(const std::string& fileName) {
    auto it = openFiles_.find(fileName);
    if (it != openFiles_.end()) {
        openFiles_.erase(it);
        fileModifiedStates_.erase(fileName);

        // 如果关闭的是活动文件，需要重新设置活动文件
        if (activeFileName_ == fileName) {
            if (!openFiles_.empty()) {
                activeFileName_ = openFiles_.begin()->first;
            } else {
                activeFileName_.clear();
            }
        }

        // 通知观察者文件被关闭
        // Event event("close", fileName);
        // notify(event);
    }
}

void WorkSpace::setActiveFile(const std::string& fileName) {
    if (openFiles_.find(fileName) != openFiles_.end()) {
        activeFileName_ = fileName;

        // 通知观察者活动文件改变
        // Event event("activate", fileName);
        // notify(event);
    }
}

std::shared_ptr<Editor> WorkSpace::getActiveEditor() const {
    if (activeFileName_.empty()) {
        return nullptr;
    }
    auto it = openFiles_.find(activeFileName_);
    if (it != openFiles_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Editor> WorkSpace::getEditor(const std::string& fileName) const {
    auto it = openFiles_.find(fileName);
    if (it != openFiles_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::string> WorkSpace::getOpenFiles() const {
    std::vector<std::string> files;
    for (const auto& pair : openFiles_) {
        files.push_back(pair.first);
    }
    return files;
}

const std::string& WorkSpace::getActiveFileName() const {
    return activeFileName_;
}

bool WorkSpace::isFileOpen(const std::string& fileName) const {
    return openFiles_.find(fileName) != openFiles_.end();
}

void WorkSpace::setFileModified(const std::string& fileName, bool modified) {
    if (fileModifiedStates_.find(fileName) != fileModifiedStates_.end()) {
        fileModifiedStates_[fileName] = modified;

        // 通知观察者文件修改状态变化
        // Event event("modify", fileName);
        // notify(event);
    }
}

bool WorkSpace::isFileModified(const std::string& fileName) const {
    auto it = fileModifiedStates_.find(fileName);
    if (it != fileModifiedStates_.end()) {
        return it->second;
    }
    return false;
}

void WorkSpace::setLogEnabled(bool enabled) {
    logEnabled_ = enabled;

    // 通知观察者日志开关变化
    // Event event("log", "");
    // notify(event);
}

bool WorkSpace::isLogEnabled() const {
    return logEnabled_;
}

std::shared_ptr<WorkspaceMemento> WorkSpace::createMemento() const {
    std::vector<std::string> openFiles;
    for (const auto& pair : openFiles_) {
        openFiles.push_back(pair.first);
    }

    return std::make_shared<WorkspaceMemento>(openFiles, activeFileName_, fileModifiedStates_, logEnabled_);
}

void WorkSpace::restoreFromMemento(const WorkspaceMemento& memento) {
    // 清空当前状态
    openFiles_.clear();
    fileModifiedStates_.clear();

    // 恢复打开的文件（需要重新创建Editor实例）
    const auto& openFiles = memento.getOpenFiles();
    for (const auto& fileName : openFiles) {
        // 创建新的Editor实例（暂时固定为TextEditor）
        auto editor = std::make_shared<TextEditor>();
        openFiles_[fileName] = editor;
    }

    // 恢复文件修改状态
    const auto& modifiedStates = memento.getFileModifiedStates();
    for (const auto& pair : modifiedStates) {
        if (openFiles_.find(pair.first) != openFiles_.end()) {
            fileModifiedStates_[pair.first] = pair.second;
        }
    }

    // 恢复活动文件名
    activeFileName_ = memento.getActiveFileName();

    // 恢复日志开关
    logEnabled_ = memento.isLogEnabled();

    // 通知观察者工作区恢复
    // Event event("restore", "");
    // notify(event);
}

void WorkSpace::attach(std::shared_ptr<Observe> observer) {
    observers_.push_back(observer);
}

void WorkSpace::detach(std::shared_ptr<Observe> observer) {
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void WorkSpace::notify(const Event& event) {
    notifyObservers(event);
}

void WorkSpace::notifyObservers(const Event& event) {
    for (const auto& observer : observers_) {
        observer->update(event);
    }
}

void WorkSpace::executeCommand(std::unique_ptr<Command> command) {
    // 判断命令类型：是WorkSpaceCommand还是EditorCommand？
    if (dynamic_cast<WorkSpaceCommand*>(command.get()) != nullptr) {
        // 工作区命令，在当前WorkSpace中处理
        // TODO: 具体处理逻辑暂不实现
        // 目前只执行命令（但工作区命令需要实现execute方法）
        command->execute();
    } else {
        // 编辑器命令，传递给活动编辑器
        auto activeEditor = getActiveEditor();
        if (activeEditor) {
            activeEditor->executeCommand(std::move(command));
        } else {
            // 没有活动编辑器，抛出异常
            throw std::runtime_error("No active editor to execute command");
        }
    }
}