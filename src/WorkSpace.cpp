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

// WorkspaceMemento实现（保持不变，用于向后兼容）

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

// WorkSpace实现（重构为协调员）

WorkSpace::WorkSpace() : logEnabled_(false), exitRequested_(false), loggerManager_(fileSystemService_, *this) {
    // 构造函数：DocumentManager和FileSystemService会自动初始化
    // LoggerManager需要引用FileSystemService和WorkSpace

    // 在初始化时尝试加载配置文件
    // 如果配置文件损坏或不存在，程序应正常启动为空白状态
    loadConfig(".editor_config");
}

WorkSpace::~WorkSpace() {
}

// 文件管理（委托给DocumentManager）

void WorkSpace::openFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        // 创建新的TextEditor实例
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
    // 通知观察者日志开关变化
    // Event event("log", "");
    // notify(event);
}

bool WorkSpace::isLogEnabled() const {
    return logEnabled_;
}

// 备忘录模式：保存和恢复状态
std::shared_ptr<WorkspaceMemento> WorkSpace::createMemento() const {
    // 从DocumentManager获取当前状态
    auto openFiles = documentManager_.getOpenFiles();
    auto activeFileName = documentManager_.getActiveFileName();
    auto modifiedStates = documentManager_.getAllModifiedStates();

    return std::make_shared<WorkspaceMemento>(openFiles, activeFileName, modifiedStates, logEnabled_);
}

void WorkSpace::restoreFromMemento(const WorkspaceMemento& memento) {
    // 清空当前状态
    documentManager_.clear();

    // 恢复打开的文件（需要重新创建Editor实例）
    const auto& openFiles = memento.getOpenFiles();
    for (const auto& fileName : openFiles) {
        auto editor = createTextEditor();
        documentManager_.openFile(fileName, editor);
    }

    // 恢复文件修改状态
    const auto& modifiedStates = memento.getFileModifiedStates();
    for (const auto& pair : modifiedStates) {
        if (documentManager_.isFileOpen(pair.first)) {
            documentManager_.setFileModified(pair.first, pair.second);
        }
    }

    // 恢复活动文件名
    const std::string& activeFileName = memento.getActiveFileName();
    if (!activeFileName.empty() && documentManager_.isFileOpen(activeFileName)) {
        documentManager_.setActiveFile(activeFileName);
    }

    // 恢复日志开关
    logEnabled_ = memento.isLogEnabled();

    // 通知观察者工作区恢复
    // Event event("restore", "");
    // notify(event);
}

// 观察者模式
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

size_t WorkSpace::getObserverCount() const {
    return observers_.size();
}

// 文件加载和保存功能（使用FileSystemService和DocumentManager）
void WorkSpace::loadFile(const std::string& fileName) {
    // 检查文件是否已经打开，如果是，则直接切换到该文件
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    // 使用FileSystemService加载文件
    std::vector<std::string> lines;
    bool fileExisted = false;

    if (fileSystemService_.fileExists(fileName)) {
        // 文件存在，读取内容
        lines = fileSystemService_.loadFile(fileName);
        fileExisted = true;
    } else {
        // 文件不存在，创建新的空文件
        fileSystemService_.createFileIfNotExists(fileName);
        lines = {}; // 空内容
    }

    // 创建TextEditor并设置内容
    auto editor = createTextEditor();
    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);
    if (textEditor) {
        textEditor->setLines(lines);
        textEditor->setModified(!fileExisted); // 新文件标记为已修改
    }

    // 添加到DocumentManager
    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, !fileExisted);

    // 设置为活动文件
    if (documentManager_.getActiveFileName().empty()) {
        documentManager_.setActiveFile(fileName);
    }

    // 检查文件第一行是否为 "#log"，如果是则自动启动日志记录
    if (fileExisted && !lines.empty() && lines[0] == "# log") {
        startLoggingForFile(fileName);
    }
}

void WorkSpace::saveFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        throw std::runtime_error("文件未打开: " + fileName);
    }

    auto editor = documentManager_.getEditor(fileName);
    if (!editor) {
        throw std::runtime_error("编辑器不存在: " + fileName);
    }

    // 将TextEditor内容写入文件
    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);
    if (textEditor) {
        const auto& lines = textEditor->getLines();
        fileSystemService_.saveFile(fileName, lines);
        documentManager_.setFileModified(fileName, false); // 清除修改标记
    } else {
        // 如果是其他类型的Editor，暂时不支持
        throw std::runtime_error("不支持的编辑器类型");
    }
}

void WorkSpace::saveAllFiles() {
    auto openFiles = documentManager_.getOpenFiles();
    for (const auto& fileName : openFiles) {
        try {
            saveFile(fileName);
        } catch (const std::exception& e) {
            // 保存单个文件失败，继续保存其他文件
            std::cerr << "保存文件 " << fileName << " 失败: " << e.what() << std::endl;
        }
    }
}

void WorkSpace::initFile(const std::string& fileName, bool withLog) {
    // 如果文件已打开，则直接切换到该文件
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    // 创建新的TextEditor
    auto editor = createTextEditor();
    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);

    if (withLog) {
        // 在第一行添加 "# log"
        textEditor->setLines({ "# log" });
    } else {
        // 清空编辑器
        textEditor->clear();
    }

    // 标记为已修改（新缓冲区）
    textEditor->setModified(true);

    // 添加到DocumentManager
    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, true);

    // 设置为活动文件
    documentManager_.setActiveFile(fileName);

    // 如果指定了with-log，则自动启动日志记录
    if (withLog) {
        startLoggingForFile(fileName);
    }
}

// 获取目录树（字符串表示）
std::string WorkSpace::getDirectoryTree(const std::string& path) {
    return fileSystemService_.getDirectoryTree(path);
}

// 获取文件信息列表（结构化数据）
std::vector<FileInfo> WorkSpace::getFileInfoList() const {
    return documentManager_.getFileInfoList();
}

// 获取目录树结构（结构化数据）
std::shared_ptr<TreeNode> WorkSpace::getDirectoryTreeStructure(const std::string& path) {
    return fileSystemService_.getDirectoryTreeStructure(path);
}

// 获取服务引用
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

// 检查是否有未保存的文件
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
    // 委托给LoggerManager处理日志显示
    loggerManager_.showLog(targetFile);
}

// 创建TextEditor实例
std::shared_ptr<TextEditor> WorkSpace::createTextEditor() const {
    auto editor = std::make_shared<TextEditor>();
    auto textEngine = std::make_shared<TextEngine>();
    editor->setTextEngine(textEngine);
    return editor;
}

// 会话开始通知
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

// 退出请求管理
void WorkSpace::requestExit() {
    exitRequested_ = true;
}

bool WorkSpace::isExitRequested() const {
    return exitRequested_;
}

// 配置管理
void WorkSpace::saveConfig(const std::string& configFile) {
    try {
        // 创建当前状态的Memento
        auto memento = createMemento();

        // 通过FileSystemService保存配置
        fileSystemService_.saveWorkspaceConfig(configFile, *memento);

        // 通知保存成功
        Event event("config saved", configFile);
        notify(event);
    } catch (const std::exception& e) {
        // 配置文件保存失败，但不应该导致程序崩溃
        // 可以记录错误，但程序继续运行
        outputService_.outputError("Warning: Failed to save configuration: " + std::string(e.what()));
    }
}

bool WorkSpace::loadConfig(const std::string& configFile) {
    try {
        // 通过FileSystemService加载配置
        auto memento = fileSystemService_.loadWorkspaceConfig(configFile);

        if (!memento) {
            // 配置文件不存在，返回false表示加载失败（但这不是错误）
            return false;
        }

        // 从Memento恢复状态
        restoreFromMemento(*memento);

        // 通知配置加载成功
        Event event("config loaded", configFile);
        notify(event);

        return true;
    } catch (const std::exception& e) {
        // 配置文件损坏或格式错误
        // 输出警告，但程序继续运行（返回false表示加载失败）
        outputService_.outputError("Warning: Failed to load configuration (using default): " + std::string(e.what()));
        return false;
    }
}