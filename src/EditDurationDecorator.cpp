#include "EditDurationDecorator.h"
#include "EditDurationTracker.h"
#include "StringUtils.h"

EditDurationDecorator::EditDurationDecorator(const EditDurationTracker* tracker)
    : tracker_(tracker) {
}

std::string EditDurationDecorator::decorateFileName(const std::string& fileName) const {
    int seconds = tracker_ ? tracker_->getDurationSeconds(fileName) : 0;
    return fileName + " (" + StringUtils::formatDuration(seconds) + ")";
}

FileInfo EditDurationDecorator::decorateFileInfo(const FileInfo& info) const {
    return FileInfo(decorateFileName(info.fileName), info.isActive, info.isModified);
}

std::shared_ptr<TreeNode> EditDurationDecorator::decorateFileNode(const FileInfo& info) const {
    std::string name = appendStatusMarks(decorateFileName(info.fileName),
                                         info.isActive, info.isModified);
    return std::make_shared<TreeNode>(name, false);
}

std::string EditDurationDecorator::appendStatusMarks(const std::string& decoratedName,
                                                      bool isActive, bool isModified) {
    std::string result = decoratedName;
    if (isActive)  result += " [*]";
    if (isModified) result += " [+]";
    return result;
}
