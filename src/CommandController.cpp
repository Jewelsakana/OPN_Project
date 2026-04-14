#include "CommandController.h"
#include "WorkSpace.h"
#include "TextEditor.h"
#include "WorkSpaceCommand.h"
#include "LogCommand.h"
#include "TextCommands.h"
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
            // 命令执行成功，记录事件
            std::string targetFileName;
            if (parsed.type == CommandType::WorkSpaceCommand) {
                // 工作区命令：如果有文件名参数则使用，否则为空
                if (parsed.fileName) {
                    targetFileName = *parsed.fileName;
                } else {
                    // 对于没有文件名的工作区命令，使用空字符串
                    targetFileName = "";
                }
            } else { // EditorCommand
                // 编辑器命令：目标文件是当前活动文件
                targetFileName = workspace_->getActiveFileName();
            }
            Event event(commandString, targetFileName);
            workspace_->notify(event);
        } else {
            throw std::runtime_error("Failed to create command from parsed data");
        }
    } catch (const std::exception& e) {
        // 统一错误处理：使用OutputService输出错误信息
        workspace_->getOutputService().outputError(e.what());
        // 重新抛出异常，允许上层处理
        throw;
    }
}

std::unique_ptr<Command> CommandController::createCommandFromParsed(const ParsedCommand& parsed) {
    if (parsed.type == CommandType::WorkSpaceCommand) {
        switch (parsed.workSpaceType) {
            case WorkSpaceCommandType::Load:
                if (parsed.fileName) {
                    return std::unique_ptr<Command>(new LoadCommand(*parsed.fileName));
                }
                break;
            case WorkSpaceCommandType::Save:
                if (parsed.target) {
                    return std::unique_ptr<Command>(new SaveCommand(*parsed.target));
                } else {
                    return std::unique_ptr<Command>(new SaveCommand(""));
                }
                break;
            case WorkSpaceCommandType::Init:
                if (parsed.fileName && parsed.withLog) {
                    return std::unique_ptr<Command>(new InitCommand(*parsed.fileName, *parsed.withLog));
                } else if (parsed.fileName) {
                    return std::unique_ptr<Command>(new InitCommand(*parsed.fileName, false));
                }
                break;
            case WorkSpaceCommandType::Close:
                if (parsed.fileName) {
                    return std::unique_ptr<Command>(new CloseCommand(*parsed.fileName));
                } else {
                    return std::unique_ptr<Command>(new CloseCommand(""));
                }
                break;
            case WorkSpaceCommandType::Edit:
                if (parsed.fileName) {
                    return std::unique_ptr<Command>(new EditCommand(*parsed.fileName));
                }
                break;
            case WorkSpaceCommandType::EditorList:
                return std::unique_ptr<Command>(new EditorListCommand());
            case WorkSpaceCommandType::DirTree:
                if (parsed.path) {
                    return std::unique_ptr<Command>(new DirTreeCommand(*parsed.path));
                } else {
                    return std::unique_ptr<Command>(new DirTreeCommand(""));
                }
                break;
            case WorkSpaceCommandType::Undo:
                return std::unique_ptr<Command>(new UndoCommand());
            case WorkSpaceCommandType::Redo:
                return std::unique_ptr<Command>(new RedoCommand());
            case WorkSpaceCommandType::Exit:
                return std::unique_ptr<Command>(new ExitCommand());
            case WorkSpaceCommandType::Logon:
                if (parsed.fileName) {
                    return std::unique_ptr<Command>(new LogonCommand(*parsed.fileName));
                } else {
                    return std::unique_ptr<Command>(new LogonCommand(""));
                }
            case WorkSpaceCommandType::Logoff:
                if (parsed.fileName) {
                    return std::unique_ptr<Command>(new LogoffCommand(*parsed.fileName));
                } else {
                    return std::unique_ptr<Command>(new LogoffCommand(""));
                }
            case WorkSpaceCommandType::Logshow:
                if (parsed.fileName) {
                    return std::unique_ptr<Command>(new LogshowCommand(*parsed.fileName));
                } else {
                    return std::unique_ptr<Command>(new LogshowCommand(""));
                }
        }
    } else if (parsed.type == CommandType::EditorCommand) {
        // 获取当前活动编辑器
        auto activeEditor = workspace_->getActiveEditor();
        if (!activeEditor) {
            throw std::runtime_error("No active editor to execute editor command");
        }

        // 将Editor转换为TextEditor（我们知道在WorkSpace中存储的是TextEditor实例）
        auto textEditor = dynamic_cast<TextEditor*>(activeEditor.get());
        if (!textEditor) {
            throw std::runtime_error("Active editor is not a TextEditor instance");
        }

        // 获取TextEngine
        auto textEngine = textEditor->getTextEngine();
        if (!textEngine) {
            throw std::runtime_error("TextEditor has no TextEngine");
        }

        // 根据编辑器命令类型创建具体的Command对象
        switch (parsed.editorType) {
            case EditorCommandType::Append:
                if (parsed.text) {
                    return std::unique_ptr<Command>(new AppendCommand(
                        textEditor->getLinesRef(), textEngine.get(), *parsed.text));
                }
                break;
            case EditorCommandType::Insert:
                if (parsed.line && parsed.column && parsed.text) {
                    return std::unique_ptr<Command>(new InsertCommand(
                        textEditor->getLinesRef(), textEngine.get(),
                        *parsed.line, *parsed.column, *parsed.text));
                }
                break;
            case EditorCommandType::Delete:
                if (parsed.line && parsed.column && parsed.length) {
                    return std::unique_ptr<Command>(new DeleteCommand(
                        textEditor->getLinesRef(), textEngine.get(),
                        *parsed.line, *parsed.column, *parsed.length));
                }
                break;
            case EditorCommandType::Replace:
                if (parsed.line && parsed.column && parsed.length && parsed.text) {
                    return std::unique_ptr<Command>(new ReplaceCommand(
                        textEditor->getLinesRef(), textEngine.get(),
                        *parsed.line, *parsed.column, *parsed.length, *parsed.text));
                }
                break;
            case EditorCommandType::Show:
                if (parsed.startLine && parsed.endLine) {
                    return std::unique_ptr<Command>(new ShowCommand(
                        textEditor->getLinesRef(), textEngine.get(),
                        &workspace_->getOutputService(),
                        *parsed.startLine, *parsed.endLine));
                } else {
                    // 如果没有指定范围，使用默认值
                    return std::unique_ptr<Command>(new ShowCommand(
                        textEditor->getLinesRef(), textEngine.get(),
                        &workspace_->getOutputService()));
                }
                break;
        }
    }

    throw std::runtime_error("Invalid parsed command type or missing required parameters");
}

void CommandController::executeCommand(std::unique_ptr<Command> command) {
    try {
        // 判断命令类型：是WorkSpaceCommand还是EditorCommand？
        if (auto* wsCommand = dynamic_cast<WorkSpaceCommand*>(command.get())) {
            // 工作区命令，设置关联的工作区
            wsCommand->setWorkSpace(workspace_);
            // 执行命令
            wsCommand->execute();
        } else {
            // 编辑器命令，传递给活动编辑器
            auto activeEditor = workspace_->getActiveEditor();
            if (activeEditor) {
                activeEditor->executeCommand(std::move(command));
            } else {
                // 没有活动编辑器，抛出异常
                throw std::runtime_error("No active editor to execute command");
            }
        }
    } catch (const std::exception& e) {
        // 统一错误处理：使用OutputService输出错误信息
        workspace_->getOutputService().outputError(e.what());
        // 重新抛出异常，让上层处理
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