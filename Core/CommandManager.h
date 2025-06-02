#pragma once

#include <vector>
#include "AbstractCommand.h"

class CommandManager {
public:
    CommandManager() = default;
    ~CommandManager() = default;

    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    bool executeCommand(std::unique_ptr<AbstractCommand> command);

    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();

    void clearHistory();
    int undoStackSize() const;
    int redoStackSize() const;

private:
    std::vector<std::unique_ptr<AbstractCommand>> m_undoStack;
    std::vector<std::unique_ptr<AbstractCommand>> m_redoStack;
};
