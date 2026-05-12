#ifndef SPELLCHECKCOMMAND_H
#define SPELLCHECKCOMMAND_H

#include "WorkSpaceCommand.h"
#include "ISpellChecker.h"
#include <string>
#include <vector>
#include <memory>

// SpellCheckCommand：拼写检查命令
// 格式：spell-check [file]
//   - 不指定参数：检查当前活动文件
//   - file：检查指定文本文件
// 依赖 ISpellChecker 接口（适配器模式），不绑定具体拼写检查产品
class SpellCheckCommand : public WorkSpaceCommand {
public:
    explicit SpellCheckCommand(const std::string& fileName = "");
    void execute() override;
    void undo() override;
    bool isReadOnly() const override;

private:
    std::string fileName_;

    // 检查指定编辑器中的文本
    std::vector<SpellCheckResult> checkEditor(const Editor& editor,
                                               ISpellChecker& checker);
};

#endif // SPELLCHECKCOMMAND_H
