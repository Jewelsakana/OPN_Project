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

// 前向声明
// class Editor; // 已经包含
// class Observe;
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

// WorkSpace类：管理工作区状态
class WorkSpace {
public:
    WorkSpace();
    ~WorkSpace();

    // 文件管理
    void openFile(const std::string& fileName);
    void closeFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);

    // 获取当前活动编辑器
    std::shared_ptr<Editor> getActiveEditor() const;

    // 获取编辑器by文件名
    std::shared_ptr<Editor> getEditor(const std::string& fileName) const;

    // 获取打开的文件列表
    std::vector<std::string> getOpenFiles() const;

    // 获取当前活动文件名
    const std::string& getActiveFileName() const;

    // 检查文件是否已打开
    bool isFileOpen(const std::string& fileName) const;

    // 修改状态管理
    void setFileModified(const std::string& fileName, bool modified);
    bool isFileModified(const std::string& fileName) const;

    // 日志开关
    void setLogEnabled(bool enabled);
    bool isLogEnabled() const;

    // 备忘录模式：保存和恢复状态
    std::shared_ptr<WorkspaceMemento> createMemento() const;
    void restoreFromMemento(const WorkspaceMemento& memento);

    // 观察者模式
    void attach(std::shared_ptr<Observe> observer);
    void detach(std::shared_ptr<Observe> observer);
    void notify(const Event& event);

    // 命令处理相关职责已转移到CommandController类中
    // 使用CommandController进行命令的创建、分发和执行

    // 文件加载和保存功能
    void loadFile(const std::string& fileName);
    void saveFile(const std::string& fileName);
    void saveAllFiles();
    void initFile(const std::string& fileName, bool withLog = false);

private:
    std::map<std::string, std::shared_ptr<Editor>> openFiles_;  // 打开的文件映射
    std::string activeFileName_;                                // 当前活动文件名
    std::map<std::string, bool> fileModifiedStates_;            // 文件修改状态
    bool logEnabled_;                                           // 日志开关
    std::vector<std::shared_ptr<Observe>> observers_;           // 观察者列表

    // 通知观察者
    void notifyObservers(const Event& event);
};

#endif // WORKSPACE_H