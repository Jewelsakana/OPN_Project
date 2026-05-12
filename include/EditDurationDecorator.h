#ifndef EDITDURATIONDECORATOR_H
#define EDITDURATIONDECORATOR_H

#include "DataStructures.h"
#include <string>
#include <memory>

class EditDurationTracker;

// EditDurationDecorator：装饰器模式——为文件名附加编辑时长和状态标记
// 将"如何格式化时长"和"如何拼接字符串"从命令类中解耦
class EditDurationDecorator {
public:
    explicit EditDurationDecorator(const EditDurationTracker* tracker);

    // 为文件名附加时长信息，返回 "filename (X分钟)" 格式
    std::string decorateFileName(const std::string& fileName) const;

    // 为 FileInfo 装饰：文件名附加时长，保留 isActive/isModified
    FileInfo decorateFileInfo(const FileInfo& info) const;

    // 为树形显示创建装饰后的 TreeNode
    std::shared_ptr<TreeNode> decorateFileNode(const FileInfo& info) const;

private:
    // 附加状态标记（[*] 表示活动，[+] 表示已修改）
    static std::string appendStatusMarks(const std::string& decoratedName,
                                         bool isActive, bool isModified);

    const EditDurationTracker* tracker_;
};

#endif // EDITDURATIONDECORATOR_H
