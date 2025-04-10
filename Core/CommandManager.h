#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <memory>
#include <vector>
#include "AbstractCommand.h"

class CommandManager {
public:
    CommandManager();
    ~CommandManager();

    // Prevent copying
    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;

    // Command execution
    bool executeCommand(std::unique_ptr<AbstractCommand> command);

    // Undo/Redo operations
    bool canUndo() const;
    bool canRedo() const;
    bool undo();
    bool redo();

    // Stack management
    void clearHistory();
    int undoStackSize() const;
    int redoStackSize() const;

private:
    // PIMPL implementation
    class CommandManagerImpl;
    std::unique_ptr<CommandManagerImpl> m_pImpl;
};

#endif // COMMANDMANAGER_H
