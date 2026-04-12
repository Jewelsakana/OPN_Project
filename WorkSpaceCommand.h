#ifndef WORKSPACECOMMAND_H
#define WORKSPACECOMMAND_H

#include "Command.h"

// WorkSpaceCommand基类：所有工作区命令的基类
// 派生自Command，用于区分工作区命令和编辑器命令
class WorkSpaceCommand : public Command {
public:
    virtual ~WorkSpaceCommand() = default;

    // 可以添加工作区命令特有的方法或属性
    // 例如：获取关联的工作区
};

#endif // WORKSPACECOMMAND_H