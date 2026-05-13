#include "WorkSpace.h"
#include "TextEditor.h"
#include "TextEngine.h"
#include "XmlEditor.h"
#include "EditorFactory.h"
#include "Logger.h"
#include "EditDurationTracker.h"
#include "MockSpellChecker.h"
#include "HttpSpellCheckerAdapter.h"
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
                                   const std::vector<std::string>& loggedFiles,
                                   const std::string& spellCheckerProduct)
    : openFiles_(openFiles),
      activeFileName_(activeFileName),
      fileModifiedStates_(fileModifiedStates),
      logEnabled_(logEnabled),
      loggedFiles_(loggedFiles),
      spellCheckerProduct_(spellCheckerProduct) {
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

const std::string& WorkspaceMemento::getSpellCheckerProduct() const {
    return spellCheckerProduct_;
}

// WorkSpace实现

WorkSpace::WorkSpace()
    : loggerManager_(fileSystemService_, *this)
    , configManager_(*this, configSerializer_)
    , editorCoordinator_(documentManager_)
    , fileCoordinator_(fileSystemService_, documentManager_, outputService_, loggerManager_)
    , logCoordinator_(loggerManager_, outputService_)
    , exitRequested_(false) {
    // 创建编辑时长统计器并注册为观察者
    durationTracker_ = std::make_shared<EditDurationTracker>();
    attach(durationTracker_);
    // 注入编辑器工厂（根据扩展名创建对应编辑器）
    fileCoordinator_.setEditorFactory([this](const std::string& ext) { return createEditorForExtension(ext); });
    // 先加载配置（可能包含 spellCheckerProduct_，由 restoreFromMemento 解析）
    loadConfig(".editor_config");
    // 根据配置解析拼写检查器，配置缺失时默认使用 http
    spellChecker_ = resolveSpellChecker(spellCheckerProduct_);
}

WorkSpace::~WorkSpace() {
}

// 文件管理（委托给EditorCoordinator）

void WorkSpace::openFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        std::string ext;
        auto dotPos = fileName.rfind('.');
        if (dotPos != std::string::npos) {
            ext = fileName.substr(dotPos);
        }
        auto editor = createEditorForExtension(ext);
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
                                              logCoordinator_.isLogEnabled(), loggedFiles,
                                              spellCheckerProduct_);
}

void WorkSpace::restoreOpenFiles(const WorkspaceMemento& memento) {
    editorCoordinator_.clear();
    const auto& openFiles = memento.getOpenFiles();
    for (const auto& fileName : openFiles) {
        std::string ext;
        auto dotPos = fileName.rfind('.');
        if (dotPos != std::string::npos) {
            ext = fileName.substr(dotPos);
        }
        auto editor = createEditorForExtension(ext);
        editorCoordinator_.openFile(fileName, editor);
    }
}

void WorkSpace::restoreModifiedStates(const WorkspaceMemento& memento) {
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
}

void WorkSpace::restoreLogState(const WorkspaceMemento& memento) {
    logCoordinator_.setLogEnabled(memento.isLogEnabled());

    const auto& loggedFiles = memento.getLoggedFiles();
    std::set<std::string> loggedFilesSet(loggedFiles.begin(), loggedFiles.end());

    for (const auto& fileName : loggedFiles) {
        if (editorCoordinator_.isFileOpen(fileName)) {
            logCoordinator_.startLoggingForFile(fileName);
        }
    }

    const auto& openFiles = memento.getOpenFiles();
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

void WorkSpace::restoreFromMemento(const WorkspaceMemento& memento) {
    restoreOpenFiles(memento);
    restoreModifiedStates(memento);
    restoreLogState(memento);
    spellCheckerProduct_ = memento.getSpellCheckerProduct();
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

// 统一输出接口

void WorkSpace::outputError(const std::string& message) {
    outputService_.outputError(message);
}

void WorkSpace::outputLine(const std::string& message) {
    outputService_.outputLine(message);
}

void WorkSpace::outputList(const std::vector<FileInfo>& files) {
    outputService_.outputList(files);
}

void WorkSpace::outputTree(const TreeNode& root) {
    outputService_.outputTree(root);
}

OutputService& WorkSpace::getOutputService() {
    return outputService_;
}

LoggerManager& WorkSpace::getLoggerManager() {
    return loggerManager_;
}

EditDurationTracker* WorkSpace::getEditDurationTracker() const {
    return durationTracker_.get();
}

void WorkSpace::setSpellChecker(std::shared_ptr<ISpellChecker> checker) {
    spellChecker_ = std::move(checker);
}

std::shared_ptr<ISpellChecker> WorkSpace::getSpellChecker() const {
    return spellChecker_;
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
    logCoordinator_.showLog(fileName, getActiveFileName());
}

std::shared_ptr<Editor> WorkSpace::createEditorForExtension(const std::string& extension) const {
    auto editorPtr = EditorFactory::createEditor(extension);
    if (!editorPtr) {
        // 默认回退到TextEditor
        auto textEditor = std::make_shared<TextEditor>();
        auto textEngine = std::make_shared<TextEngine>();
        textEditor->setTextEngine(textEngine);
        return textEditor;
    }
    // 如果是TextEditor，注入TextEngine
    if (auto textEditor = dynamic_cast<TextEditor*>(editorPtr.get())) {
        auto textEngine = std::make_shared<TextEngine>();
        textEditor->setTextEngine(textEngine);
    }
    return editorPtr;
}

void WorkSpace::notifySessionStart() {
    Event event("session start at " + Event::currentTimestampString(), "");
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
    configManager_.saveConfig(configFile);
}

bool WorkSpace::loadConfig(const std::string& configFile) {
    return configManager_.loadConfig(configFile);
}

std::shared_ptr<ISpellChecker> WorkSpace::resolveSpellChecker(const std::string& product) const {
    static const std::map<std::string, std::function<std::shared_ptr<ISpellChecker>()>> factories = {
        {"mock", []() { return std::make_shared<MockSpellChecker>(); }},
        {"http", []() { return std::make_shared<HttpSpellCheckerAdapter>(); }},
    };

    auto it = factories.find(product);
    if (it != factories.end()) {
        return it->second();
    }
    // 配置缺失或无法识别时默认使用 HttpSpellCheckerAdapter
    return std::make_shared<HttpSpellCheckerAdapter>();
}
