#include "EditorCoordinator.h"
#include "DocumentManager.h"

EditorCoordinator::EditorCoordinator(DocumentManager& dm)
    : documentManager_(dm) {
}

void EditorCoordinator::openFile(const std::string& fileName, std::shared_ptr<Editor> editor) {
    if (!documentManager_.isFileOpen(fileName)) {
        documentManager_.openFile(fileName, editor);
    }
}

void EditorCoordinator::closeFile(const std::string& fileName) {
    documentManager_.closeFile(fileName);
}

void EditorCoordinator::setActiveFile(const std::string& fileName) {
    documentManager_.setActiveFile(fileName);
}

std::shared_ptr<Editor> EditorCoordinator::getActiveEditor() const {
    return documentManager_.getActiveEditor();
}

std::shared_ptr<Editor> EditorCoordinator::getEditor(const std::string& fileName) const {
    return documentManager_.getEditor(fileName);
}

std::vector<std::string> EditorCoordinator::getOpenFiles() const {
    return documentManager_.getOpenFiles();
}

const std::string& EditorCoordinator::getActiveFileName() const {
    return documentManager_.getActiveFileName();
}

bool EditorCoordinator::isFileOpen(const std::string& fileName) const {
    return documentManager_.isFileOpen(fileName);
}

void EditorCoordinator::setFileModified(const std::string& fileName, bool modified) {
    documentManager_.setFileModified(fileName, modified);
}

bool EditorCoordinator::isFileModified(const std::string& fileName) const {
    return documentManager_.isFileModified(fileName);
}

std::vector<FileInfo> EditorCoordinator::getFileInfoList() const {
    return documentManager_.getFileInfoList();
}

bool EditorCoordinator::hasUnsavedFiles() const {
    return documentManager_.hasUnsavedFiles();
}

std::vector<std::string> EditorCoordinator::getUnsavedFiles() const {
    return documentManager_.getUnsavedFiles();
}

const std::map<std::string, bool>& EditorCoordinator::getAllModifiedStates() const {
    return documentManager_.getAllModifiedStates();
}

void EditorCoordinator::clear() {
    documentManager_.clear();
}

DocumentManager& EditorCoordinator::getDocumentManager() {
    return documentManager_;
}
