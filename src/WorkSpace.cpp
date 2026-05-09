#include "WorkSpace.h"
#include "TextEditor.h"
#include "TextEngine.h"
#include "Logger.h"
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <chrono>
#include <ctime>

// WorkspaceMemento实现

WorkspaceMemento::WorkspaceMemento(const std::vector<std::string>& openFiles,
                                   const std::string& activeFileName,
                                   const std::map<std::string, bool>& fileModifiedStates,
                                   bool logEnabled,
                                   const std::vector<std::string>& loggedFiles)
    : openFiles_(openFiles),
      activeFileName_(activeFileName),
      fileModifiedStates_(fileModifiedStates),
      logEnabled_(logEnabled),
      loggedFiles_(loggedFiles) {
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

const std::vector<std::string>& WorkspaceMemento::getLoggedFiles() const {
    return loggedFiles_;
}

// WorkSpace实现

WorkSpace::WorkSpace()
    : loggerManager_(fileSystemService_, *this)
    , configManager_(*this, configSerializer_)
    , editorCoordinator_(documentManager_)
    , fileCoordinator_(fileSystemService_, documentManager_, outputService_, loggerManager_)
    , logCoordinator_(loggerManager_, outputService_)
    , configCoordinator_(configManager_)
    , exitRequested_(false) {
    // 注入TextEditor工厂
    fileCoordinator_.setEditorFactory([this]() { return createTextEditor(); });
    loadConfig(".editor_config");
}

WorkSpace::~WorkSpace() {
}

// 文件管理（委托给EditorCoordinator）

void WorkSpace::openFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        auto editor = createTextEditor();
        editorCoordinator_.openFile(fileName, editor);
    }
}

void WorkSpace::closeFile(const std::string& fileName) {
    editorCoordinator_.closeFile(fileName);
}

void WorkSpace::setActiveFile(const std::string& fileName) {
    editorCoordinator_.setActiveFile(fileName);
}

std::shared_ptr<Editor> WorkSpace::getActiveEditor() const {
    return editorCoordinator_.getActiveEditor();
}

std::shared_ptr<Editor> WorkSpace::getEditor(const std::string& fileName) const {
    return editorCoordinator_.getEditor(fileName);
}

std::vector<std::string> WorkSpace::getOpenFiles() const {
    return editorCoordinator_.getOpenFiles();
}

const std::string& WorkSpace::getActiveFileName() const {
    return editorCoordinator_.getActiveFileName();
}

bool WorkSpace::isFileOpen(const std::string& fileName) const {
    return editorCoordinator_.isFileOpen(fileName);
}

void WorkSpace::setFileModified(const std::string& fileName, bool modified) {
    editorCoordinator_.setFileModified(fileName, modified);
}

bool WorkSpace::isFileModified(const std::string& fileName) const {
    return editorCoordinator_.isFileModified(fileName);
}

// 日志开关（委托给LogCoordinator）
void WorkSpace::setLogEnabled(bool enabled) {
    logCoordinator_.setLogEnabled(enabled);
}

bool WorkSpace::isLogEnabled() const {
    return logCoordinator_.isLogEnabled();
}

// 观察者通知
void WorkSpace::notify(const Event& event) {
    notifyObservers(event);
}

// 备忘录模式

std::shared_ptr<WorkspaceMemento> WorkSpace::createMemento() const {
    auto openFiles = editorCoordinator_.getOpenFiles();
    auto activeFileName = editorCoordinator_.getActiveFileName();
    auto modifiedStates = editorCoordinator_.getAllModifiedStates();
    auto loggedFiles = logCoordinator_.getLoggedFiles();
    return std::make_shared<WorkspaceMemento>(openFiles, activeFileName, modifiedStates,
                                              logCoordinator_.isLogEnabled(), loggedFiles);
}

void WorkSpace::restoreFromMemento(const WorkspaceMemento& memento) {
    editorCoordinator_.clear();

    const auto& openFiles = memento.getOpenFiles();
    for (const auto& fileName : openFiles) {
        auto editor = createTextEditor();
        editorCoordinator_.openFile(fileName, editor);
    }

    const auto& modifiedStates = memento.getFileModifiedStates();
    for (const auto& pair : modifiedStates) {
        if (editorCoordinator_.isFileOpen(pair.first)) {
            editorCoordinator_.setFileModified(pair.first, pair.second);
        }
    }

    const std::string& activeFileName = memento.getActiveFileName();
    if (!activeFileName.empty() && editorCoordinator_.isFileOpen(activeFileName)) {
        editorCoordinator_.setActiveFile(activeFileName);
    }

    logCoordinator_.setLogEnabled(memento.isLogEnabled());

    const auto& loggedFiles = memento.getLoggedFiles();
    std::set<std::string> loggedFilesSet(loggedFiles.begin(), loggedFiles.end());

    for (const auto& fileName : loggedFiles) {
        if (editorCoordinator_.isFileOpen(fileName)) {
            logCoordinator_.startLoggingForFile(fileName);
        }
    }

    for (const auto& fileName : openFiles) {
        if (loggedFilesSet.find(fileName) != loggedFilesSet.end()) {
            continue;
        }
        if (fileSystemService_.fileExists(fileName)) {
            try {
                auto lines = fileSystemService_.loadFile(fileName);
                if (!lines.empty() && lines[0] == "# log") {
                    logCoordinator_.startLoggingForFile(fileName);
                }
            } catch (const std::exception& e) {
                outputService_.outputError("Warning: Failed to check #log in " + fileName + ": " + e.what());
            }
        }
    }
}

// 文件加载和保存（委托给FileCoordinator）

void WorkSpace::loadFile(const std::string& fileName) {
    fileCoordinator_.loadFile(fileName);
}

void WorkSpace::saveFile(const std::string& fileName) {
    fileCoordinator_.saveFile(fileName);
}

void WorkSpace::saveAllFiles() {
    fileCoordinator_.saveAllFiles();
}

void WorkSpace::initFile(const std::string& fileName, bool withLog) {
    fileCoordinator_.initFile(fileName, withLog);
}

// 目录树（委托给DirectoryService）
std::string WorkSpace::getDirectoryTree(const std::string& path) {
    return directoryService_.getDirectoryTree(path);
}

std::shared_ptr<TreeNode> WorkSpace::getDirectoryTreeStructure(const std::string& path) {
    return directoryService_.getDirectoryTreeStructure(path);
}

std::vector<FileInfo> WorkSpace::getFileInfoList() const {
    return editorCoordinator_.getFileInfoList();
}

// 服务引用
DocumentManager& WorkSpace::getDocumentManager() {
    return documentManager_;
}

FileSystemService& WorkSpace::getFileSystemService() {
    return fileSystemService_;
}

OutputService& WorkSpace::getOutputService() {
    return outputService_;
}

LoggerManager& WorkSpace::getLoggerManager() {
    return loggerManager_;
}

bool WorkSpace::hasUnsavedFiles() const {
    return editorCoordinator_.hasUnsavedFiles();
}

// 文件日志管理（委托给LogCoordinator）
void WorkSpace::startLoggingForFile(const std::string& fileName) {
    logCoordinator_.startLoggingForFile(fileName);
}

void WorkSpace::stopLoggingForFile(const std::string& fileName) {
    logCoordinator_.stopLoggingForFile(fileName);
}

bool WorkSpace::isLoggingForFile(const std::string& fileName) const {
    return logCoordinator_.isLoggingForFile(fileName);
}

void WorkSpace::showLog(const std::string& fileName) {
    std::string targetFile = fileName;
    if (targetFile.empty()) {
        targetFile = getActiveFileName();
        if (targetFile.empty()) {
            outputService_.outputLine("Error: No active file to show log");
            return;
        }
    }
    logCoordinator_.showLog(targetFile);
}

std::shared_ptr<TextEditor> WorkSpace::createTextEditor() const {
    auto editor = std::make_shared<TextEditor>();
    auto textEngine = std::make_shared<TextEngine>();
    editor->setTextEngine(textEngine);
    return editor;
}

void WorkSpace::notifySessionStart() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = ss.str();
    Event event("session start at " + timestamp, "");
    notify(event);
}

void WorkSpace::requestExit() {
    exitRequested_ = true;
}

bool WorkSpace::isExitRequested() const {
    return exitRequested_;
}

// 配置管理（委托给ConfigCoordinator）
void WorkSpace::saveConfig(const std::string& configFile) {
    configCoordinator_.saveConfig(configFile);
}

bool WorkSpace::loadConfig(const std::string& configFile) {
    return configCoordinator_.loadConfig(configFile);
}
