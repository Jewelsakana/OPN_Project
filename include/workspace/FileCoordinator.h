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
class Editor;

// FileCoordinator：文件加载/保存/初始化/关闭的业务逻辑
class FileCoordinator {
public:
    FileCoordinator(FileSystemService& fs, DocumentManager& dm,
                    OutputService& out, LoggerManager& lm);

    void loadFile(const std::string& fileName);
    void saveFile(const std::string& fileName);
    void saveAllFiles();
    void initFile(const std::string& fileName, bool withLog = false);

    // 设置编辑器工厂（由WorkSpace注入，根据扩展名创建对应的Editor）
    void setEditorFactory(std::function<std::shared_ptr<Editor>(const std::string& extension)> factory);

private:
    FileSystemService& fileSystemService_;
    DocumentManager& documentManager_;
    OutputService& outputService_;
    LoggerManager& loggerManager_;
    std::function<std::shared_ptr<Editor>(const std::string& extension)> createEditorByExtension_;
};

#endif // FILECOORDINATOR_H
