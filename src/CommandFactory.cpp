#include "CommandFactory.h"
#include "TextCommands.h"
#include "WorkSpaceCommand.h"
#include "LogCommand.h"
#include "TextEditor.h"
#include "WorkSpace.h"
#include "OutputService.h"
#include <stdexcept>

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

    switch (ed.editorType) {
        case EditorCommandType::Append:
            if (ed.text)
                return std::make_unique<AppendCommand>(ctx.lines, ctx.textEngine, *ed.text);
            break;
        case EditorCommandType::Insert:
            if (ed.line && ed.column && ed.text)
                return std::make_unique<InsertCommand>(ctx.lines, ctx.textEngine,
                    *ed.line, *ed.column, *ed.text);
            break;
        case EditorCommandType::Delete:
            if (ed.line && ed.column && ed.length)
                return std::make_unique<DeleteCommand>(ctx.lines, ctx.textEngine,
                    *ed.line, *ed.column, *ed.length);
            break;
        case EditorCommandType::Replace:
            if (ed.line && ed.column && ed.length && ed.text)
                return std::make_unique<ReplaceCommand>(ctx.lines, ctx.textEngine,
                    *ed.line, *ed.column, *ed.length, *ed.text);
            break;
        case EditorCommandType::Show:
            if (ed.startLine && ed.endLine)
                return std::make_unique<ShowCommand>(ctx.lines, ctx.textEngine,
                    ctx.outputService, *ed.startLine, *ed.endLine);
            else
                return std::make_unique<ShowCommand>(ctx.lines, ctx.textEngine,
                    ctx.outputService);
    }
    throw std::runtime_error("Invalid editor command type or missing parameters");
}

std::unique_ptr<Command> CommandFactory::createWorkSpaceCommand(
    const WorkSpaceParsedCommand& ws,
    WorkSpace* workspace) {

    switch (ws.workSpaceType) {
        case WorkSpaceCommandType::Load:
            if (ws.fileName)
                return std::make_unique<LoadCommand>(*ws.fileName);
            break;
        case WorkSpaceCommandType::Save:
            return std::make_unique<SaveCommand>(ws.target.value_or(""));
        case WorkSpaceCommandType::Init:
            if (ws.fileName)
                return std::make_unique<InitCommand>(*ws.fileName, ws.withLog.value_or(false));
            break;
        case WorkSpaceCommandType::Close:
            return std::make_unique<CloseCommand>(ws.fileName.value_or(""));
        case WorkSpaceCommandType::Edit:
            if (ws.fileName)
                return std::make_unique<EditCommand>(*ws.fileName);
            break;
        case WorkSpaceCommandType::EditorList:
            return std::make_unique<EditorListCommand>();
        case WorkSpaceCommandType::DirTree:
            return std::make_unique<DirTreeCommand>(ws.path.value_or(""));
        case WorkSpaceCommandType::Undo:
            return std::make_unique<UndoCommand>();
        case WorkSpaceCommandType::Redo:
            return std::make_unique<RedoCommand>();
        case WorkSpaceCommandType::Exit:
            return std::make_unique<ExitCommand>();
        case WorkSpaceCommandType::Logon:
            return std::make_unique<LogonCommand>(ws.fileName.value_or(""));
        case WorkSpaceCommandType::Logoff:
            return std::make_unique<LogoffCommand>(ws.fileName.value_or(""));
        case WorkSpaceCommandType::Logshow:
            return std::make_unique<LogshowCommand>(ws.fileName.value_or(""));
    }
    throw std::runtime_error("Invalid workspace command type or missing parameters");
}
