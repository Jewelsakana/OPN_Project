#include "WorkSpace.h"
#include "TextEditor.h"
#include "TextEngine.h"
#include "WorkSpaceCommand.h"
#include "TextCommands.h"
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>

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
        // 设置TextEngine
        auto textEngine = std::make_shared<TextEngine>();
        editor->setTextEngine(textEngine);

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
        // 设置TextEngine
        auto textEngine = std::make_shared<TextEngine>();
        editor->setTextEngine(textEngine);
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

void WorkSpace::loadFile(const std::string& fileName) {
    // 检查文件是否已经打开，如果是，则直接切换到该文件
    if (isFileOpen(fileName)) {
        setActiveFile(fileName);
        return;
    }

    std::ifstream file(fileName);
    if (file.good()) {
        // 文件存在，读取内容
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();

        // 创建TextEditor并设置内容
        auto editor = std::make_shared<TextEditor>();
        auto textEngine = std::make_shared<TextEngine>();
        editor->setTextEngine(textEngine);
        editor->setLines(lines);
        editor->setModified(false); // 从磁盘加载，未修改

        openFiles_[fileName] = editor;
        fileModifiedStates_[fileName] = false;

        // 设置为活动文件
        if (activeFileName_.empty()) {
            activeFileName_ = fileName;
        } else {
            // 保持当前活动文件不变，除非没有活动文件
        }
    } else {
        // 文件不存在，创建新的空文件
        openFile(fileName); // 使用现有的openFile方法创建空编辑器
        fileModifiedStates_[fileName] = true; // 标记为已修改（新文件）
        // 设置为活动文件
        setActiveFile(fileName);
    }
}

void WorkSpace::saveFile(const std::string& fileName) {
    if (!isFileOpen(fileName)) {
        throw std::runtime_error("文件未打开: " + fileName);
    }

    std::ofstream file(fileName);
    if (!file) {
        throw std::runtime_error("无法写入文件: " + fileName);
    }

    auto editor = getEditor(fileName);
    if (!editor) {
        throw std::runtime_error("编辑器不存在: " + fileName);
    }

    // 将TextEditor内容写入文件
    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);
    if (textEditor) {
        const auto& lines = textEditor->getLines();
        for (size_t i = 0; i < lines.size(); ++i) {
            file << lines[i];
            if (i != lines.size() - 1) {
                file << '\n'; // 行间换行
            }
        }
    } else {
        // 如果是其他类型的Editor，暂时不支持
        throw std::runtime_error("不支持的编辑器类型");
    }

    file.close();
    setFileModified(fileName, false); // 清除修改标记
}

void WorkSpace::saveAllFiles() {
    for (const auto& pair : openFiles_) {
        const std::string& fileName = pair.first;
        try {
            saveFile(fileName);
        } catch (const std::exception& e) {
            // 保存单个文件失败，继续保存其他文件
            // 可以记录错误或重新抛出
            std::cerr << "保存文件 " << fileName << " 失败: " << e.what() << std::endl;
        }
    }
}

void WorkSpace::initFile(const std::string& fileName, bool withLog) {
    // 如果文件已打开，则直接切换到该文件
    if (isFileOpen(fileName)) {
        setActiveFile(fileName);
        return;
    }

    // 创建新的TextEditor
    auto editor = std::make_shared<TextEditor>();
    auto textEngine = std::make_shared<TextEngine>();
    editor->setTextEngine(textEngine);

    if (withLog) {
        // 在第一行添加 "# log"
        editor->setLines({ "# log" });
    } else {
        // 清空编辑器（保持默认空行）
        editor->clear();
    }

    // 标记为已修改（新缓冲区）
    editor->setModified(true);

    openFiles_[fileName] = editor;
    fileModifiedStates_[fileName] = true; // 新文件，已修改

    // 设置为活动文件
    setActiveFile(fileName);
}
