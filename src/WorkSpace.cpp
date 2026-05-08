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
    , logEnabled_(false)
    , exitRequested_(false) {
    loadConfig(".editor_config");
}

WorkSpace::~WorkSpace() {
}

// 文件管理（委托给DocumentManager）

void WorkSpace::openFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        auto editor = createTextEditor();
        documentManager_.openFile(fileName, editor);
    }
}

void WorkSpace::closeFile(const std::string& fileName) {
    documentManager_.closeFile(fileName);
}

void WorkSpace::setActiveFile(const std::string& fileName) {
    documentManager_.setActiveFile(fileName);
}

std::shared_ptr<Editor> WorkSpace::getActiveEditor() const {
    return documentManager_.getActiveEditor();
}

std::shared_ptr<Editor> WorkSpace::getEditor(const std::string& fileName) const {
    return documentManager_.getEditor(fileName);
}

std::vector<std::string> WorkSpace::getOpenFiles() const {
    return documentManager_.getOpenFiles();
}

const std::string& WorkSpace::getActiveFileName() const {
    return documentManager_.getActiveFileName();
}

bool WorkSpace::isFileOpen(const std::string& fileName) const {
    return documentManager_.isFileOpen(fileName);
}

void WorkSpace::setFileModified(const std::string& fileName, bool modified) {
    documentManager_.setFileModified(fileName, modified);
}

bool WorkSpace::isFileModified(const std::string& fileName) const {
    return documentManager_.isFileModified(fileName);
}

// 日志开关
void WorkSpace::setLogEnabled(bool enabled) {
    logEnabled_ = enabled;
}

bool WorkSpace::isLogEnabled() const {
    return logEnabled_;
}

// 观察者通知
void WorkSpace::notify(const Event& event) {
    notifyObservers(event);
}

// 备忘录模式
std::shared_ptr<WorkspaceMemento> WorkSpace::createMemento() const {
    auto openFiles = documentManager_.getOpenFiles();
    auto activeFileName = documentManager_.getActiveFileName();
    auto modifiedStates = documentManager_.getAllModifiedStates();
    auto loggedFiles = loggerManager_.getLoggedFiles();
    return std::make_shared<WorkspaceMemento>(openFiles, activeFileName, modifiedStates, logEnabled_, loggedFiles);
}

void WorkSpace::restoreFromMemento(const WorkspaceMemento& memento) {
    documentManager_.clear();

    const auto& openFiles = memento.getOpenFiles();
    for (const auto& fileName : openFiles) {
        auto editor = createTextEditor();
        documentManager_.openFile(fileName, editor);
    }

    const auto& modifiedStates = memento.getFileModifiedStates();
    for (const auto& pair : modifiedStates) {
        if (documentManager_.isFileOpen(pair.first)) {
            documentManager_.setFileModified(pair.first, pair.second);
        }
    }

    const std::string& activeFileName = memento.getActiveFileName();
    if (!activeFileName.empty() && documentManager_.isFileOpen(activeFileName)) {
        documentManager_.setActiveFile(activeFileName);
    }

    logEnabled_ = memento.isLogEnabled();

    const auto& loggedFiles = memento.getLoggedFiles();
    std::set<std::string> loggedFilesSet(loggedFiles.begin(), loggedFiles.end());

    for (const auto& fileName : loggedFiles) {
        if (documentManager_.isFileOpen(fileName)) {
            startLoggingForFile(fileName);
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
                    startLoggingForFile(fileName);
                }
            } catch (const std::exception& e) {
                outputService_.outputError("Warning: Failed to check #log in " + fileName + ": " + e.what());
            }
        }
    }
}

// 文件加载和保存

void WorkSpace::loadFile(const std::string& fileName) {
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    std::vector<std::string> lines;
    bool fileExisted = false;

    if (fileSystemService_.fileExists(fileName)) {
        lines = fileSystemService_.loadFile(fileName);
        fileExisted = true;
    } else {
        fileSystemService_.createFileIfNotExists(fileName);
        lines = {};
    }

    auto editor = createTextEditor();
    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);
    if (textEditor) {
        textEditor->setLines(lines);
        textEditor->setModified(!fileExisted);
    }

    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, !fileExisted);

    if (documentManager_.getActiveFileName().empty()) {
        documentManager_.setActiveFile(fileName);
    }

    if (fileExisted && !lines.empty() && lines[0] == "# log") {
        startLoggingForFile(fileName);
    }
}

void WorkSpace::saveFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        throw std::runtime_error("File not open: " + fileName);
    }

    auto editor = documentManager_.getEditor(fileName);
    if (!editor) {
        throw std::runtime_error("Editor not found: " + fileName);
    }

    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);
    if (textEditor) {
        const auto& lines = textEditor->getLines();
        fileSystemService_.saveFile(fileName, lines);
        documentManager_.setFileModified(fileName, false);
    } else {
        throw std::runtime_error("Unsupported editor type");
    }
}

void WorkSpace::saveAllFiles() {
    auto openFiles = documentManager_.getOpenFiles();
    for (const auto& fileName : openFiles) {
        try {
            saveFile(fileName);
        } catch (const std::exception& e) {
            std::cerr << "Failed to save " << fileName << ": " << e.what() << std::endl;
        }
    }
}

void WorkSpace::initFile(const std::string& fileName, bool withLog) {
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    auto editor = createTextEditor();
    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);

    if (withLog) {
        textEditor->setLines({ "# log" });
    } else {
        textEditor->clear();
    }

    textEditor->setModified(true);
    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, true);
    documentManager_.setActiveFile(fileName);

    if (withLog) {
        startLoggingForFile(fileName);
    }
}

// 目录树（委托给DirectoryService）
std::string WorkSpace::getDirectoryTree(const std::string& path) {
    return directoryService_.getDirectoryTree(path);
}

std::shared_ptr<TreeNode> WorkSpace::getDirectoryTreeStructure(const std::string& path) {
    return directoryService_.getDirectoryTreeStructure(path);
}

std::vector<FileInfo> WorkSpace::getFileInfoList() const {
    return documentManager_.getFileInfoList();
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
    return documentManager_.hasUnsavedFiles();
}

// 文件日志管理
void WorkSpace::startLoggingForFile(const std::string& fileName) {
    loggerManager_.startLoggingForFile(fileName);
}

void WorkSpace::stopLoggingForFile(const std::string& fileName) {
    loggerManager_.stopLoggingForFile(fileName);
}

bool WorkSpace::isLoggingForFile(const std::string& fileName) const {
    return loggerManager_.isLoggingForFile(fileName);
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
    loggerManager_.showLog(targetFile);
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

// 配置管理（委托给ConfigManager）
void WorkSpace::saveConfig(const std::string& configFile) {
    configManager_.saveConfig(configFile);
}

bool WorkSpace::loadConfig(const std::string& configFile) {
    return configManager_.loadConfig(configFile);
}
