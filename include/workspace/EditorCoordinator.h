#ifndef EDITORCOORDINATOR_H
#define EDITORCOORDINATOR_H

#include "DataStructures.h"
#include "Editor.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

class DocumentManager;

// EditorCoordinator：编辑器切换和查询（委托给DocumentManager）
class EditorCoordinator {
public:
    explicit EditorCoordinator(DocumentManager& dm);

    void openFile(const std::string& fileName, std::shared_ptr<Editor> editor);
    void closeFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);
    std::shared_ptr<Editor> getActiveEditor() const;
    std::shared_ptr<Editor> getEditor(const std::string& fileName) const;
    std::vector<std::string> getOpenFiles() const;
    const std::string& getActiveFileName() const;
    bool isFileOpen(const std::string& fileName) const;
    void setFileModified(const std::string& fileName, bool modified);
    bool isFileModified(const std::string& fileName) const;

    std::vector<FileInfo> getFileInfoList() const;
    bool hasUnsavedFiles() const;
    std::vector<std::string> getUnsavedFiles() const;
    const std::map<std::string, bool>& getAllModifiedStates() const;
    void clear();

    // 获取底层DocumentManager引用（供其他协调器使用）
    DocumentManager& getDocumentManager();

private:
    DocumentManager& documentManager_;
};

#endif // EDITORCOORDINATOR_H
