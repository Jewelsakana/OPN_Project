#include "CommandController.h"
#include "WorkSpace.h"
#include "TextEditor.h"
#include "WorkSpaceCommand.h"
#include "CommandFactory.h"
#include "OutputService.h"
#include "Event.h"
#include <stdexcept>
#include <memory>

CommandController::CommandController(WorkSpace* workspace)
    : workspace_(workspace) {
    if (!workspace_) {
        throw std::invalid_argument("CommandController requires a valid WorkSpace pointer");
    }
}

void CommandController::parseAndExecuteCommand(const std::string& commandString) {
    try {
        CommandParser parser;
        ParsedCommand parsed = parser.parse(commandString);

        auto command = createCommandFromParsed(parsed);
        if (command) {
            executeCommand(std::move(command));
            std::string targetFileName;
            if (auto* ws = parsed.asWorkSpace()) {
                targetFileName = ws->fileName.value_or("");
            } else {
                targetFileName = workspace_->getActiveFileName();
            }
            Event event(commandString, targetFileName);
            workspace_->notify(event);
        } else {
            throw std::runtime_error("Failed to create command from parsed data");
        }
    } catch (const std::exception& e) {
        workspace_->outputError(e.what());
        throw;
    }
}

std::unique_ptr<Command> CommandController::createCommandFromParsed(const ParsedCommand& parsed) {
    return CommandFactory::createFromParsed(parsed, workspace_, getActiveTextEditor());
}

void CommandController::executeCommand(std::unique_ptr<Command> command) {
    try {
        if (auto* wsCommand = dynamic_cast<WorkSpaceCommand*>(command.get())) {
            wsCommand->execute();
        } else {
            auto activeEditor = workspace_->getActiveEditor();
            if (activeEditor) {
                activeEditor->executeCommand(std::move(command));
            } else {
                throw std::runtime_error("No active editor to execute command");
            }
        }
    } catch (const std::exception& e) {
        workspace_->outputError(e.what());
        throw;
    }
}

WorkSpace* CommandController::getWorkSpace() const {
    return workspace_;
}

TextEditor* CommandController::getActiveTextEditor() const {
    auto activeEditor = workspace_->getActiveEditor();
    if (!activeEditor) {
        return nullptr;
    }
    return dynamic_cast<TextEditor*>(activeEditor.get());
}
