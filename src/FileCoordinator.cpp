#include "FileCoordinator.h"
#include "FileSystemService.h"
#include "DocumentManager.h"
#include "OutputService.h"
#include "LoggerManager.h"
#include "Editor.h"
#include <stdexcept>
#include <functional>
#include <algorithm>

namespace {
    std::string getFileExtension(const std::string& fileName) {
        auto dotPos = fileName.rfind('.');
        if (dotPos != std::string::npos) {
            return fileName.substr(dotPos);
        }
        return "";
    }
}

FileCoordinator::FileCoordinator(FileSystemService& fs, DocumentManager& dm,
                                 OutputService& out, LoggerManager& lm)
    : fileSystemService_(fs), documentManager_(dm),
      outputService_(out), loggerManager_(lm) {
}

void FileCoordinator::setEditorFactory(std::function<std::shared_ptr<Editor>(const std::string& extension)> factory) {
    createEditorByExtension_ = std::move(factory);
}

void FileCoordinator::loadFile(const std::string& fileName) {
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    auto editor = createEditorByExtension_(getFileExtension(fileName));
    if (!editor) {
        throw std::runtime_error("Failed to create editor for: " + fileName);
    }

    bool fileExisted = fileSystemService_.fileExists(fileName);

    if (fileExisted) {
        std::string content = fileSystemService_.readFileContent(fileName);
        if (!content.empty() && content.rfind("# log", 0) == 0) {
            loggerManager_.startLoggingForFile(fileName);
        }
        editor->loadFromData(content);
    } else {
        fileSystemService_.createFileIfNotExists(fileName);
    }

    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, !fileExisted);

    if (documentManager_.getActiveFileName().empty()) {
        documentManager_.setActiveFile(fileName);
    }
}

void FileCoordinator::saveFile(const std::string& fileName) {
    if (!documentManager_.isFileOpen(fileName)) {
        throw std::runtime_error("File not open: " + fileName);
    }

    auto editor = documentManager_.getEditor(fileName);
    if (!editor) {
        throw std::runtime_error("Editor not found: " + fileName);
    }

    std::string content = editor->saveToData();
    fileSystemService_.writeFileContent(fileName, content);
    documentManager_.setFileModified(fileName, false);
}

void FileCoordinator::saveAllFiles() {
    auto openFiles = documentManager_.getOpenFiles();
    for (const auto& fileName : openFiles) {
        try {
            saveFile(fileName);
        } catch (const std::exception& e) {
            outputService_.outputError("Failed to save " + fileName + ": " + e.what());
        }
    }
}

void FileCoordinator::initFile(const std::string& fileName, bool withLog) {
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    auto editor = createEditorByExtension_(getFileExtension(fileName));
    if (!editor) {
        throw std::runtime_error("Failed to create editor for: " + fileName);
    }

    editor->initContent(withLog);

    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, true);
    documentManager_.setActiveFile(fileName);

    if (withLog) {
        loggerManager_.startLoggingForFile(fileName);
    }
}
