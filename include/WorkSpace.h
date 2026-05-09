#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "Command.h"
#include "Observe.h"
#include "Event.h"
#include "Editor.h"
#include "CommandParser.h"
#include "FileSystemService.h"
#include "DocumentManager.h"
#include "OutputService.h"
#include "DataStructures.h"
#include "LoggerManager.h"
#include "ObserverManager.h"
#include "DirectoryService.h"
#include "ConfigSerializer.h"
#include "ConfigManager.h"
#include "FileCoordinator.h"
#include "EditorCoordinator.h"
#include "LogCoordinator.h"
#include "ConfigCoordinator.h"

class LoggerManager;

// WorkspaceMemento类：用于保存和恢复工作区状态
class WorkspaceMemento {
public:
    WorkspaceMemento(const std::vector<std::string>& openFiles,
                     const std::string& activeFileName,
                     const std::map<std::string, bool>& fileModifiedStates,
                     bool logEnabled,
                     const std::vector<std::string>& loggedFiles = {});

    const std::vector<std::string>& getOpenFiles() const;
    const std::string& getActiveFileName() const;
    const std::map<std::string, bool>& getFileModifiedStates() const;
    bool isLogEnabled() const;
    const std::vector<std::string>& getLoggedFiles() const;

private:
    std::vector<std::string> openFiles_;
    std::string activeFileName_;
    std::map<std::string, bool> fileModifiedStates_;
    bool logEnabled_;
    std::vector<std::string> loggedFiles_;
};

// WorkSpace类：作为协调员门面，组合四大协调器
class WorkSpace : public ObserverManager {
public:
    WorkSpace();
    ~WorkSpace();

    // 文件管理（委托给EditorCoordinator）
    void openFile(const std::string& fileName);
    void closeFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);
    std::shared_ptr<Editor> getActiveEditor() const;
    std::shared_ptr<Editor> getEditor(const std::string& fileName) const;
    std::vector<std::string> getOpenFiles() const;
    const std::string& getActiveFileName() const;
    bool isFileOpen(const std::string& fileName) const;
    void setFileModified(const std::string& fileName, bool modified);
    bool isFileModified(const std::string& fileName) const;

    // 日志开关（委托给LogCoordinator）
    void setLogEnabled(bool enabled);
    bool isLogEnabled() const;

    // 文件日志管理（委托给LogCoordinator）
    void startLoggingForFile(const std::string& fileName);
    void stopLoggingForFile(const std::string& fileName);
    bool isLoggingForFile(const std::string& fileName) const;
    void showLog(const std::string& fileName);

    // 会话开始通知
    void notifySessionStart();

    // 备忘录模式：保存和恢复状态
    std::shared_ptr<WorkspaceMemento> createMemento() const;
    void restoreFromMemento(const WorkspaceMemento& memento);

    // 观察者通知（公开接口，委托给ObserverManager）
    void notify(const Event& event);

    // 文件加载和保存（委托给FileCoordinator）
    void loadFile(const std::string& fileName);
    void saveFile(const std::string& fileName);
    void saveAllFiles();
    void initFile(const std::string& fileName, bool withLog = false);

    // 目录树（委托给DirectoryService）
    std::string getDirectoryTree(const std::string& path = "");
    std::shared_ptr<TreeNode> getDirectoryTreeStructure(const std::string& path = "");

    // 文件信息列表（委托给EditorCoordinator）
    std::vector<FileInfo> getFileInfoList() const;

    // 获取服务引用
    DocumentManager& getDocumentManager();
    FileSystemService& getFileSystemService();
    OutputService& getOutputService();
    LoggerManager& getLoggerManager();

    // 检查未保存文件（委托给EditorCoordinator）
    bool hasUnsavedFiles() const;

    // 退出管理
    void requestExit();
    bool isExitRequested() const;

    // 配置管理（委托给ConfigCoordinator）
    void saveConfig(const std::string& configFile = ".editor_config");
    bool loadConfig(const std::string& configFile = ".editor_config");

private:
    // 服务层（按依赖顺序声明）
    DocumentManager documentManager_;
    FileSystemService fileSystemService_;
    OutputService outputService_;
    LoggerManager loggerManager_;
    DirectoryService directoryService_;
    ConfigSerializer configSerializer_;
    ConfigManager configManager_;

    // 协调器层（依赖服务层）
    EditorCoordinator editorCoordinator_;
    FileCoordinator fileCoordinator_;
    LogCoordinator logCoordinator_;
    ConfigCoordinator configCoordinator_;

    // 状态变量
    bool exitRequested_;

    std::shared_ptr<TextEditor> createTextEditor() const;

    // restoreFromMemento 辅助方法
    void restoreOpenFiles(const WorkspaceMemento& memento);
    void restoreModifiedStates(const WorkspaceMemento& memento);
    void restoreLogState(const WorkspaceMemento& memento);
};

#endif // WORKSPACE_H
