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

// 前向声明
// class Editor; // 已经包含
// class Observe;
class LoggerManager;
// class Event;

// WorkspaceMemento类：用于保存和恢复工作区状态
class WorkspaceMemento {
public:
    // 构造函数，保存状态
    WorkspaceMemento(const std::vector<std::string>& openFiles,
                     const std::string& activeFileName,
                     const std::map<std::string, bool>& fileModifiedStates,
                     bool logEnabled);

    // 获取打开的文件列表（文件名）
    const std::vector<std::string>& getOpenFiles() const;

    // 获取活动文件名
    const std::string& getActiveFileName() const;

    // 获取文件修改状态
    const std::map<std::string, bool>& getFileModifiedStates() const;

    // 获取日志开关状态
    bool isLogEnabled() const;

private:
    std::vector<std::string> openFiles_;           // 打开的文件名列表
    std::string activeFileName_;                   // 活动文件名
    std::map<std::string, bool> fileModifiedStates_; // 文件修改状态
    bool logEnabled_;                              // 日志开关状态
};

// WorkSpace类：作为协调员，管理工作区状态，委托给DocumentManager和FileSystemService
class WorkSpace {
public:
    WorkSpace();
    ~WorkSpace();

    // 文件管理（委托给DocumentManager）
    void openFile(const std::string& fileName);
    void closeFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);

    // 获取当前活动编辑器（委托给DocumentManager）
    std::shared_ptr<Editor> getActiveEditor() const;

    // 获取编辑器by文件名（委托给DocumentManager）
    std::shared_ptr<Editor> getEditor(const std::string& fileName) const;

    // 获取打开的文件列表（委托给DocumentManager）
    std::vector<std::string> getOpenFiles() const;

    // 获取当前活动文件名（委托给DocumentManager）
    const std::string& getActiveFileName() const;

    // 检查文件是否已打开（委托给DocumentManager）
    bool isFileOpen(const std::string& fileName) const;

    // 修改状态管理（委托给DocumentManager）
    void setFileModified(const std::string& fileName, bool modified);
    bool isFileModified(const std::string& fileName) const;

    // 日志开关
    void setLogEnabled(bool enabled);
    bool isLogEnabled() const;

    // 文件日志管理
    void startLoggingForFile(const std::string& fileName);
    void stopLoggingForFile(const std::string& fileName);
    bool isLoggingForFile(const std::string& fileName) const;

    // 会话开始通知
    void notifySessionStart();

    // 备忘录模式：保存和恢复状态
    std::shared_ptr<WorkspaceMemento> createMemento() const;
    void restoreFromMemento(const WorkspaceMemento& memento);

    // 观察者模式
    void attach(std::shared_ptr<Observe> observer);
    void detach(std::shared_ptr<Observe> observer);
    void notify(const Event& event);
    size_t getObserverCount() const;  // 获取观察者数量（用于测试）

    // 文件加载和保存功能（使用FileSystemService和DocumentManager）
    void loadFile(const std::string& fileName);
    void saveFile(const std::string& fileName);
    void saveAllFiles();
    void initFile(const std::string& fileName, bool withLog = false);

    // 获取目录树（使用FileSystemService，返回字符串表示）
    std::string getDirectoryTree(const std::string& path = "");

    // 获取文件信息列表（结构化数据）
    std::vector<FileInfo> getFileInfoList() const;

    // 获取目录树结构（结构化数据）
    std::shared_ptr<TreeNode> getDirectoryTreeStructure(const std::string& path = "");

    // 获取服务引用（用于测试和特殊操作）
    DocumentManager& getDocumentManager();
    FileSystemService& getFileSystemService();
    OutputService& getOutputService();
    LoggerManager& getLoggerManager();

    // 检查是否有未保存的文件
    bool hasUnsavedFiles() const;

private:
    DocumentManager documentManager_;              // 文档管理器
    FileSystemService fileSystemService_;          // 文件系统服务
    OutputService outputService_;                  // 输出服务
    LoggerManager loggerManager_;                  // 日志管理器
    bool logEnabled_;                              // 日志开关
    std::vector<std::shared_ptr<Observe>> observers_; // 观察者列表

    // 通知观察者
    void notifyObservers(const Event& event);

    // 创建TextEditor实例
    std::shared_ptr<TextEditor> createTextEditor() const;
};

#endif // WORKSPACE_H