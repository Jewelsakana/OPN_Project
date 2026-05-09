#ifndef FILECOORDINATOR_H
#define FILECOORDINATOR_H

#include <functional>
#include <string>
#include <vector>
#include <memory>

class FileSystemService;
class DocumentManager;
class OutputService;
class LoggerManager;
class TextEditor;

// FileCoordinator：文件加载/保存/初始化/关闭的业务逻辑
class FileCoordinator {
public:
    FileCoordinator(FileSystemService& fs, DocumentManager& dm,
                    OutputService& out, LoggerManager& lm);

    void loadFile(const std::string& fileName);
    void saveFile(const std::string& fileName);
    void saveAllFiles();
    void initFile(const std::string& fileName, bool withLog = false);

    // 设置TextEditor工厂（由WorkSpace注入）
    void setEditorFactory(std::function<std::shared_ptr<TextEditor>()> factory);

private:
    FileSystemService& fileSystemService_;
    DocumentManager& documentManager_;
    OutputService& outputService_;
    LoggerManager& loggerManager_;
    std::function<std::shared_ptr<TextEditor>()> createTextEditor_;
};

#endif // FILECOORDINATOR_H
