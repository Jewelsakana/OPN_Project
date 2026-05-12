#include "EditDurationTracker.h"
#include "Event.h"
#include "StringUtils.h"
#include <iostream>

EditDurationTracker::EditDurationTracker()
    : lastTick_(std::chrono::steady_clock::now()) {
}

void EditDurationTracker::update(const Event& e) {
    try {
        std::string targetFile = e.getTargetFileName();
        std::string cmd = StringUtils::toLower(e.getCommandContent());
        bool isClose = StringUtils::startsWith(cmd, "close");

        if (isClose) {
            // close 命令：记录时长并移除跟踪
            std::string closedFile = targetFile.empty() ? currentFile_ : targetFile;
            if (!closedFile.empty() && durationMap_.find(closedFile) != durationMap_.end()) {
                if (currentFile_ == closedFile) {
                    recordElapsed();
                    currentFile_.clear();
                }
                durationMap_.erase(closedFile);
            }
            return;
        }

        if (targetFile.empty()) return;

        // 检查是否是新文件（首次在事件中出现）
        if (durationMap_.find(targetFile) == durationMap_.end()) {
            durationMap_[targetFile] = 0;
        }

        // 记录上一个文件的时长
        recordElapsed();

        // 切换到新文件
        currentFile_ = targetFile;
        lastTick_ = std::chrono::steady_clock::now();
    } catch (...) {
        std::cerr << "警告: EditDurationTracker::update 失败" << std::endl;
    }
}

void EditDurationTracker::onFileOpened(const std::string& fileName) {
    try {
        if (durationMap_.find(fileName) == durationMap_.end()) {
            durationMap_[fileName] = 0;
        }
    } catch (...) {
        std::cerr << "警告: EditDurationTracker::onFileOpened 失败" << std::endl;
    }
}

void EditDurationTracker::onFileClosed(const std::string& fileName) {
    try {
        if (currentFile_ == fileName) {
            recordElapsed();
            currentFile_.clear();
        }
        durationMap_.erase(fileName);
    } catch (...) {
        std::cerr << "警告: EditDurationTracker::onFileClosed 失败" << std::endl;
    }
}

int EditDurationTracker::getDurationSeconds(const std::string& fileName) const {
    auto it = durationMap_.find(fileName);
    if (it == durationMap_.end()) return 0;

    int total = it->second;
    // 如果是当前活动文件，加上从上次tick到现在的时长
    if (fileName == currentFile_) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastTick_).count();
        total += static_cast<int>(elapsed);
    }
    return total;
}

const std::unordered_map<std::string, int>& EditDurationTracker::getAllDurations() const {
    return durationMap_;
}

void EditDurationTracker::reset() {
    currentFile_.clear();
    durationMap_.clear();
    lastTick_ = std::chrono::steady_clock::now();
}

const std::string& EditDurationTracker::getCurrentFile() const {
    return currentFile_;
}

void EditDurationTracker::recordElapsed() {
    if (currentFile_.empty()) return;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastTick_).count();
    if (elapsed > 0) {
        durationMap_[currentFile_] += static_cast<int>(elapsed);
    }
}
