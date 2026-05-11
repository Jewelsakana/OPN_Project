#include "FileCoordinator.h"
#include "FileSystemService.h"
#include "DocumentManager.h"
#include "OutputService.h"
#include "LoggerManager.h"
#include "TextEditor.h"
#include "XmlEditor.h"
#include "Editor.h"
#include <stdexcept>
#include <functional>
#include <algorithm>

FileCoordinator::FileCoordinator(FileSystemService& fs, DocumentManager& dm,
                                 OutputService& out, LoggerManager& lm)
    : fileSystemService_(fs), documentManager_(dm),
      outputService_(out), loggerManager_(lm) {
}

void FileCoordinator::setEditorFactory(std::function<std::shared_ptr<Editor>(const std::string& extension)> factory) {
    createEditorByExtension_ = std::move(factory);
}

std::shared_ptr<Editor> FileCoordinator::createEditorForFile(const std::string& fileName) {
    // 从文件名提取扩展名
    std::string extension;
    auto dotPos = fileName.rfind('.');
    if (dotPos != std::string::npos) {
        extension = fileName.substr(dotPos);
    }
    if (createEditorByExtension_) {
        return createEditorByExtension_(extension);
    }
    return nullptr;
}

static bool isXmlFile(const std::string& fileName) {
    auto dotPos = fileName.rfind('.');
    if (dotPos == std::string::npos) return false;
    std::string ext = fileName.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == ".xml";
}

void FileCoordinator::loadFile(const std::string& fileName) {
    if (documentManager_.isFileOpen(fileName)) {
        documentManager_.setActiveFile(fileName);
        return;
    }

    auto editor = createEditorForFile(fileName);
    if (!editor) {
        throw std::runtime_error("Failed to create editor for: " + fileName);
    }

    bool fileExisted = fileSystemService_.fileExists(fileName);
    bool modified = !fileExisted;

    if (fileExisted) {
        if (isXmlFile(fileName)) {
            auto xmlEditor = dynamic_cast<XmlEditor*>(editor.get());
            if (!xmlEditor) {
                throw std::runtime_error("Expected XmlEditor for XML file");
            }
            xmlEditor->loadFromFile(fileName);
        } else {
            auto textEditor = dynamic_cast<TextEditor*>(editor.get());
            if (!textEditor) {
                throw std::runtime_error("Expected TextEditor for text file");
            }
            auto lines = fileSystemService_.loadFile(fileName);
            textEditor->setLines(lines);
        }
        modified = false;
    } else {
        fileSystemService_.createFileIfNotExists(fileName);
    }

    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, modified);

    if (documentManager_.getActiveFileName().empty()) {
        documentManager_.setActiveFile(fileName);
    }

    if (fileExisted && isXmlFile(fileName)) {
        // XML 文件不检查 #log 头
    } else if (fileExisted) {
        auto textEditor = dynamic_cast<TextEditor*>(editor.get());
        if (textEditor) {
            const auto& lines = textEditor->getLines();
            if (!lines.empty() && lines[0] == "# log") {
                loggerManager_.startLoggingForFile(fileName);
            }
        }
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

    if (auto xmlEditor = dynamic_cast<XmlEditor*>(editor.get())) {
        xmlEditor->saveToFile(fileName);
        documentManager_.setFileModified(fileName, false);
    } else if (auto textEditor = dynamic_cast<TextEditor*>(editor.get())) {
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

    auto editor = createEditorForFile(fileName);
    if (!editor) {
        throw std::runtime_error("Failed to create editor for: " + fileName);
    }

    if (isXmlFile(fileName)) {
        auto xmlEditor = dynamic_cast<XmlEditor*>(editor.get());
        if (!xmlEditor) {
            throw std::runtime_error("Expected XmlEditor for XML file");
        }
        // 生成默认 XML 内容
        std::string xmlContent;
        if (withLog) {
            xmlContent = "# log\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root id=\"root\">\n</root>\n";
        } else {
            xmlContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root id=\"root\">\n</root>\n";
        }
        xmlEditor->loadFromString(xmlContent);
    } else {
        auto textEditor = dynamic_cast<TextEditor*>(editor.get());
        if (!textEditor) {
            throw std::runtime_error("Expected TextEditor for text file");
        }
        if (withLog) {
            textEditor->setLines({ "# log" });
        } else {
            textEditor->clear();
        }
    }

    documentManager_.openFile(fileName, editor);
    documentManager_.setFileModified(fileName, true);
    documentManager_.setActiveFile(fileName);

    if (withLog && !isXmlFile(fileName)) {
        loggerManager_.startLoggingForFile(fileName);
    }
}
