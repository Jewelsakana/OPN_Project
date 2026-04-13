#include "DocumentManager.h"
#include "TextEditor.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>

DocumentManager::DocumentManager() : activeFileName_("") {
    // 构造函数
}

bool DocumentManager::isFileOpen(const std::string& fileName) const {
    return openFiles_.find(fileName) != openFiles_.end();
}

void DocumentManager::openFile(const std::string& fileName, std::shared_ptr<Editor> editor) {
    safeExecute([this, &fileName, &editor]() {
        if (!validateFileName(fileName)) {
            throw std::invalid_argument("Invalid file name: " + fileName);
        }

        if (isFileOpen(fileName)) {
            // 文件已经打开，更新编辑器引用（如果需要）
            openFiles_[fileName] = editor;
        } else {
            // 新文件
            openFiles_[fileName] = editor;
            fileModifiedStates_[fileName] = false;

            // 如果没有活动文件，设置为活动文件
            if (activeFileName_.empty()) {
                activeFileName_ = fileName;
            }
        }
    });
}

void DocumentManager::closeFile(const std::string& fileName) {
    safeExecute([this, &fileName]() {
        auto it = openFiles_.find(fileName);
        if (it != openFiles_.end()) {
            openFiles_.erase(it);
            fileModifiedStates_.erase(fileName);

            // 如果关闭的是活动文件，需要重新设置活动文件
            if (activeFileName_ == fileName) {
                if (!openFiles_.empty()) {
                    // 设置为第一个打开的文件
                    activeFileName_ = openFiles_.begin()->first;
                } else {
                    activeFileName_.clear();
                }
            }
        }
    });
}

std::vector<std::string> DocumentManager::getOpenFiles() const {
    std::vector<std::string> files;
    for (const auto& pair : openFiles_) {
        files.push_back(pair.first);
    }
    return files;
}

std::shared_ptr<Editor> DocumentManager::getEditor(const std::string& fileName) const {
    auto it = openFiles_.find(fileName);
    if (it != openFiles_.end()) {
        return it->second;
    }
    return nullptr;
}

void DocumentManager::setActiveFile(const std::string& fileName) {
    safeExecute([this, &fileName]() {
        if (openFiles_.find(fileName) != openFiles_.end()) {
            activeFileName_ = fileName;
        } else {
            throw std::runtime_error("File not open: " + fileName);
        }
    });
}

const std::string& DocumentManager::getActiveFileName() const {
    return activeFileName_;
}

std::shared_ptr<Editor> DocumentManager::getActiveEditor() const {
    if (activeFileName_.empty()) {
        return nullptr;
    }
    return getEditor(activeFileName_);
}

void DocumentManager::setFileModified(const std::string& fileName, bool modified) {
    safeExecute([this, &fileName, modified]() {
        if (fileModifiedStates_.find(fileName) != fileModifiedStates_.end()) {
            fileModifiedStates_[fileName] = modified;
        } else {
            throw std::runtime_error("File not found: " + fileName);
        }
    });
}

bool DocumentManager::isFileModified(const std::string& fileName) const {
    auto it = fileModifiedStates_.find(fileName);
    if (it != fileModifiedStates_.end()) {
        return it->second;
    }
    return false;
}

const std::map<std::string, bool>& DocumentManager::getAllModifiedStates() const {
    return fileModifiedStates_;
}

bool DocumentManager::hasUnsavedFiles() const {
    for (const auto& pair : fileModifiedStates_) {
        if (pair.second) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> DocumentManager::getUnsavedFiles() const {
    std::vector<std::string> unsaved;
    for (const auto& pair : fileModifiedStates_) {
        if (pair.second) {
            unsaved.push_back(pair.first);
        }
    }
    return unsaved;
}

void DocumentManager::clear() {
    openFiles_.clear();
    fileModifiedStates_.clear();
    activeFileName_.clear();
}

bool DocumentManager::isValid() const {
    // 验证：所有打开的文件都应该有修改状态记录
    for (const auto& pair : openFiles_) {
        if (fileModifiedStates_.find(pair.first) == fileModifiedStates_.end()) {
            return false;
        }
    }
    // 验证：活动文件应该在打开的文件列表中（除非没有打开的文件）
    if (!activeFileName_.empty() && !isFileOpen(activeFileName_)) {
        return false;
    }
    return true;
}

size_t DocumentManager::getOpenFileCount() const {
    return openFiles_.size();
}

size_t DocumentManager::getModifiedFileCount() const {
    size_t count = 0;
    for (const auto& pair : fileModifiedStates_) {
        if (pair.second) {
            count++;
        }
    }
    return count;
}

void DocumentManager::handleException(const std::exception& e) const {
    // 可以添加文档管理特定的异常处理，如日志记录
    std::cerr << "DocumentManager error: " << e.what() << std::endl;
}

bool DocumentManager::validateFileName(const std::string& fileName) const {
    // 基础文件名验证
    if (fileName.empty()) {
        return false;
    }
    // 可以添加更多验证逻辑，如路径遍历攻击防护
    return true;
}