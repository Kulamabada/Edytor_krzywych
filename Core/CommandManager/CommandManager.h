#ifndef CORE_COMMANDMANAGER_COMMANDMANAGER_H
#define CORE_COMMANDMANAGER_COMMANDMANAGER_H

#include <memory>
#include <vector>
#include "Core/CommandManager/AbstractCommand.h"

namespace Core {
namespace CommandManager {

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
    std::unique_ptr<CommandManagerImpl> pImpl;
};

} // namespace CommandManager
} // namespace Core

#endif // CORE_COMMANDMANAGER_COMMANDMANAGER_H
