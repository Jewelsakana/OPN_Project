#include "LogCoordinator.h"
#include "LoggerManager.h"
#include "OutputService.h"

LogCoordinator::LogCoordinator(LoggerManager& lm, OutputService& out)
    : loggerManager_(lm), outputService_(out), logEnabled_(false) {
}

void LogCoordinator::startLoggingForFile(const std::string& fileName) {
    loggerManager_.startLoggingForFile(fileName);
}

void LogCoordinator::stopLoggingForFile(const std::string& fileName) {
    loggerManager_.stopLoggingForFile(fileName);
}

bool LogCoordinator::isLoggingForFile(const std::string& fileName) const {
    return loggerManager_.isLoggingForFile(fileName);
}

void LogCoordinator::showLog(const std::string& fileName, const std::string& activeFileName) {
    std::string targetFile = fileName;
    if (targetFile.empty()) {
        targetFile = activeFileName;
        if (targetFile.empty()) {
            outputService_.outputLine("Error: No active file to show log");
            return;
        }
    }
    loggerManager_.showLog(targetFile);
}

std::vector<std::string> LogCoordinator::getLoggedFiles() const {
    return loggerManager_.getLoggedFiles();
}

void LogCoordinator::setLogEnabled(bool enabled) {
    logEnabled_ = enabled;
}

bool LogCoordinator::isLogEnabled() const {
    return logEnabled_;
}
