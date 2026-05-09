#include "FileCoordinator.h"
#include "FileSystemService.h"
#include "DocumentManager.h"
#include "OutputService.h"
#include "LoggerManager.h"
#include "TextEditor.h"
#include <stdexcept>
#include <functional>

FileCoordinator::FileCoordinator(FileSystemService& fs, DocumentManager& dm,
                                 OutputService& out, LoggerManager& lm)
    : fileSystemService_(fs), documentManager_(dm),
      outputService_(out), loggerManager_(lm) {
}

void FileCoordinator::setEditorFactory(std::function<std::shared_ptr<TextEditor>()> factory) {
    createTextEditor_ = std::move(factory);
}

void FileCoordinator::loadFile(const std::string& fileName) {
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

    auto editor = createTextEditor_();
    if (editor) {
        editor->setLines(lines);
        editor->setModified(!fileExisted);
    }

    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, !fileExisted);

    if (documentManager_.getActiveFileName().empty()) {
        documentManager_.setActiveFile(fileName);
    }

    if (fileExisted && !lines.empty() && lines[0] == "# log") {
        loggerManager_.startLoggingForFile(fileName);
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

    auto textEditor = std::dynamic_pointer_cast<TextEditor>(editor);
    if (textEditor) {
        const auto& lines = textEditor->getLines();
        fileSystemService_.saveFile(fileName, lines);
        documentManager_.setFileModified(fileName, false);
    } else {
        throw std::runtime_error("Unsupported editor type");
    }
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

    auto editor = createTextEditor_();

    if (withLog) {
        editor->setLines({ "# log" });
    } else {
        editor->clear();
    }

    editor->setModified(true);
    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, true);
    documentManager_.setActiveFile(fileName);

    if (withLog) {
        loggerManager_.startLoggingForFile(fileName);
    }
}
