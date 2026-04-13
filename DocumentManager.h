#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include "Model.h"
#include "Editor.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

// 前向声明
class TextEditor;

// DocumentManager类：管理文件状态和编辑器映射，继承自Model基类
class DocumentManager : public Model {
public:
    DocumentManager();
    ~DocumentManager() override = default;

    // 获取实例名称
    std::string getName() const override {
        return "DocumentManager";
    }

    // 文件管理
    bool isFileOpen(const std::string& fileName) const;
    void openFile(const std::string& fileName, std::shared_ptr<Editor> editor);
    void closeFile(const std::string& fileName);
    std::vector<std::string> getOpenFiles() const;

    // 获取编辑器
    std::shared_ptr<Editor> getEditor(const std::string& fileName) const;

    // 活动文件管理
    void setActiveFile(const std::string& fileName);
    const std::string& getActiveFileName() const;
    std::shared_ptr<Editor> getActiveEditor() const;

    // 修改状态管理
    void setFileModified(const std::string& fileName, bool modified);
    bool isFileModified(const std::string& fileName) const;

    // 获取所有修改状态
    const std::map<std::string, bool>& getAllModifiedStates() const;

    // 检查是否有未保存的文件
    bool hasUnsavedFiles() const;
    std::vector<std::string> getUnsavedFiles() const;

    // 清空所有状态
    void clear();

    // 验证状态
    bool isValid() const override;

    // 获取统计信息
    size_t getOpenFileCount() const;
    size_t getModifiedFileCount() const;

private:
    std::map<std::string, std::shared_ptr<Editor>> openFiles_;  // 打开的文件映射
    std::string activeFileName_;                                // 当前活动文件名
    std::map<std::string, bool> fileModifiedStates_;            // 文件修改状态

    // 异常处理重写
    void handleException(const std::exception& e) const override;

    // 验证文件名
    bool validateFileName(const std::string& fileName) const;
};

#endif // DOCUMENTMANAGER_H