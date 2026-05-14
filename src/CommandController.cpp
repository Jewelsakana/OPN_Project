#include "CommandController.h"
#include "WorkSpace.h"
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
    return CommandFactory::createFromParsed(parsed, workspace_);
}

void CommandController::executeCommand(std::unique_ptr<Command> command) {
    if (command->isWorkSpaceLevel()) {
        command->execute();
    } else {
        auto activeEditor = workspace_->getActiveEditor();
        if (activeEditor) {
            activeEditor->executeCommand(std::move(command));
        } else {
            throw std::runtime_error("No active editor to execute command");
        }
    }
}

WorkSpace* CommandController::getWorkSpace() const {
    return workspace_;
}
