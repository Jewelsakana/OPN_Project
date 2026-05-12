#include "SpellCheckCommand.h"
#include "WorkSpace.h"
#include "OutputService.h"
#include "CommandFactory.h"
#include <stdexcept>

// 自注册：命令工厂
namespace {
    REGISTER_WS_CMD_FILENAME(WorkSpaceCommandType::SpellCheck, SpellCheckCommand)
}

SpellCheckCommand::SpellCheckCommand(const std::string& fileName)
    : fileName_(fileName) {}

void SpellCheckCommand::execute() {
    checkWorkSpace();

    auto checker = workspace_->getSpellChecker();
    if (!checker) {
        throw std::runtime_error("SpellCheckCommand: No spell checker configured");
    }

    std::shared_ptr<Editor> editor;
    if (!fileName_.empty()) {
        // 检查指定文件
        editor = workspace_->getEditor(fileName_);
        if (!editor) {
            throw std::runtime_error("SpellCheckCommand: File not open: " + fileName_);
        }
    } else {
        // 检查当前活动文件
        editor = getActiveEditorOrThrow();
    }

    auto results = checkEditor(*editor, *checker);
    workspace_->getOutputService().outputSpellCheckResults(results);
}

void SpellCheckCommand::undo() {
    // 拼写检查是只读操作，不支持撤销
}

bool SpellCheckCommand::isReadOnly() const {
    return true; // 拼写检查不修改任何状态
}

std::vector<SpellCheckResult> SpellCheckCommand::checkEditor(
    const Editor& editor, ISpellChecker& checker) {

    std::vector<SpellCheckResult> allResults;
    auto segments = editor.getTextsToCheck();

    for (const auto& seg : segments) {
        auto results = checker.checkText(seg);
        allResults.insert(allResults.end(), results.begin(), results.end());
    }

    return allResults;
}
