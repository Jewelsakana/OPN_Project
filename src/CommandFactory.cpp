#include "CommandFactory.h"
#include "WorkSpace.h"
#include "OutputService.h"
#include "Editor.h"
#include <stdexcept>
#include <unordered_map>

namespace {
    std::unordered_map<EditorCommandType, EditorCommandCreator>& editorRegistry() {
        static std::unordered_map<EditorCommandType, EditorCommandCreator> registry;
        return registry;
    }

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

static EditorCommandContext buildEditorContext(Editor* activeEditor, WorkSpace* workspace) {
    EditorCommandContext ctx;
    ctx.outputService = &workspace->getOutputService();
    activeEditor->populateContext(ctx);
    return ctx;
}

std::unique_ptr<Command> CommandFactory::createFromParsed(
    const ParsedCommand& parsed,
    WorkSpace* workspace) {

    if (auto* ws = parsed.asWorkSpace()) {
        auto cmd = createWorkSpaceCommand(*ws, workspace);
        cmd->setWorkSpace(workspace);
        return cmd;
    }

    if (auto* ed = parsed.asEditor()) {
        auto activeEditor = workspace->getActiveEditor();
        if (!activeEditor) {
            throw std::runtime_error("No active editor to execute editor command");
        }

        if (!activeEditor->supportsCommand(ed->editorType)) {
            throw std::runtime_error("Command not supported for current editor type");
        }

        auto ctx = buildEditorContext(activeEditor.get(), workspace);
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
