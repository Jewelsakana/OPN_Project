#include "CommandFactory.h"
#include "TextEditor.h"
#include "WorkSpace.h"
#include "WorkSpaceCommand.h"
#include "OutputService.h"
#include <stdexcept>
#include <unordered_map>

namespace {
    // 编辑器命令注册表
    std::unordered_map<EditorCommandType, EditorCommandCreator>& editorRegistry() {
        static std::unordered_map<EditorCommandType, EditorCommandCreator> registry;
        return registry;
    }

    // 工作区命令注册表
    std::unordered_map<WorkSpaceCommandType, WorkSpaceCommandCreator>& workSpaceRegistry() {
        static std::unordered_map<WorkSpaceCommandType, WorkSpaceCommandCreator> registry;
        return registry;
    }
}

void CommandFactory::registerEditorCreator(EditorCommandType type, EditorCommandCreator creator) {
    editorRegistry()[type] = std::move(creator);
}

void CommandFactory::registerWorkSpaceCreator(WorkSpaceCommandType type, WorkSpaceCommandCreator creator) {
    workSpaceRegistry()[type] = std::move(creator);
}

std::unique_ptr<Command> CommandFactory::createFromParsed(
    const ParsedCommand& parsed,
    WorkSpace* workspace,
    TextEditor* activeTextEditor) {

    if (auto* ws = parsed.asWorkSpace()) {
        auto cmd = createWorkSpaceCommand(*ws, workspace);
        if (auto* wsCmd = dynamic_cast<WorkSpaceCommand*>(cmd.get())) {
            wsCmd->setWorkSpace(workspace);
        }
        return cmd;
    }

    if (auto* ed = parsed.asEditor()) {
        if (!activeTextEditor) {
            throw std::runtime_error("No active editor to execute editor command");
        }
        auto textEngine = activeTextEditor->getTextEngine();
        if (!textEngine) {
            throw std::runtime_error("TextEditor has no TextEngine");
        }
        EditorCommandContext ctx{
            activeTextEditor->getLinesRef(),
            textEngine.get(),
            &workspace->getOutputService()
        };
        return createEditorCommand(*ed, ctx);
    }

    throw std::runtime_error("Invalid parsed command type");
}

std::unique_ptr<Command> CommandFactory::createEditorCommand(
    const EditorParsedCommand& ed,
    const EditorCommandContext& ctx) {

    auto& registry = editorRegistry();
    auto it = registry.find(ed.editorType);
    if (it != registry.end()) {
        auto cmd = it->second(ed, ctx);
        if (cmd) return cmd;
    }
    throw std::runtime_error("Invalid editor command type or missing parameters");
}

std::unique_ptr<Command> CommandFactory::createWorkSpaceCommand(
    const WorkSpaceParsedCommand& ws,
    WorkSpace* workspace) {

    auto& registry = workSpaceRegistry();
    auto it = registry.find(ws.workSpaceType);
    if (it != registry.end()) {
        return it->second(ws);
    }
    throw std::runtime_error("Invalid workspace command type or missing parameters");
}
