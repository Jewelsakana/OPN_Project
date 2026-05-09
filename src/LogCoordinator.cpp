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

void LogCoordinator::showLog(const std::string& fileName) {
    loggerManager_.showLog(fileName);
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
