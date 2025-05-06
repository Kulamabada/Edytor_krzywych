#include "CommandManager.h"
#include "AbstractCommand.h"
#include <vector>

bool CommandManager::executeCommand(std::unique_ptr<AbstractCommand> command) {
    if (!command)
        return false;

    if (const bool success = command->execute(); !success)
        return false;

    if (!m_redoStack.empty())
        m_redoStack.clear();

    m_undoStack.push_back(std::move(command));
    return true;
}

bool CommandManager::canUndo() const {
    return !m_undoStack.empty();
}

bool CommandManager::canRedo() const {
    return !m_redoStack.empty();
}

bool CommandManager::undo() {
    if (!canUndo())
        return false;

    auto command = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    const bool success = command->undo();

    m_redoStack.push_back(std::move(command));
    return success;
}

bool CommandManager::redo() {
    if (!canRedo())
        return false;

    auto command = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    const bool success = command->redo();

    m_undoStack.push_back(std::move(command));
    return success;
}

void CommandManager::clearHistory() {
    m_undoStack.clear();
    m_redoStack.clear();
}

int CommandManager::undoStackSize() const {
    return static_cast<int>(m_undoStack.size());
}

int CommandManager::redoStackSize() const {
    return static_cast<int>(m_redoStack.size());
}
