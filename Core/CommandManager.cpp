// File: CommandManager.cpp
#include "CommandManager.h"
#include "AbstractCommand.h"
#include <vector>
#include <stdexcept>

// Implementacja PIMPL
class CommandManager::CommandManagerImpl {
public:
    CommandManagerImpl() = default;
    ~CommandManagerImpl() = default;

    bool executeCommand(std::unique_ptr<AbstractCommand> command) {
        if (!command) {
            return false;
        }

        // Wykonanie komendy
        bool success = command->execute();
        if (!success) {
            return false;
        }

        // Po wykonaniu nowej komendy, czyścimy stos redo
        if (!m_redoStack.empty()) {
            m_redoStack.clear();
        }

        // Dodanie komendy do stosu undo
        m_undoStack.push_back(std::move(command));
        return true;
    }

    bool canUndo() const {
        return !m_undoStack.empty();
    }

    bool canRedo() const {
        return !m_redoStack.empty();
    }

    bool undo() {
        if (!canUndo()) {
            return false;
        }

        // Pobranie ostatniej komendy ze stosu undo
        auto command = std::move(m_undoStack.back());
        m_undoStack.pop_back();

        // Cofnięcie komendy
        bool success = command->undo();

        // Przeniesienie komendy na stos redo
        m_redoStack.push_back(std::move(command));
        return success;
    }

    bool redo() {
        if (!canRedo()) {
            return false;
        }

        // Pobranie ostatniej komendy ze stosu redo
        auto command = std::move(m_redoStack.back());
        m_redoStack.pop_back();

        // Ponowne wykonanie komendy
        bool success = command->redo();

        // Przeniesienie komendy na stos undo
        m_undoStack.push_back(std::move(command));
        return success;
    }

    void clearHistory() {
        m_undoStack.clear();
        m_redoStack.clear();
    }

    int undoStackSize() const {
        return static_cast<int>(m_undoStack.size());
    }

    int redoStackSize() const {
        return static_cast<int>(m_redoStack.size());
    }

private:
    std::vector<std::unique_ptr<AbstractCommand>> m_undoStack;
    std::vector<std::unique_ptr<AbstractCommand>> m_redoStack;
};

// Implementacja CommandManager
CommandManager::CommandManager() : m_pImpl(std::make_unique<CommandManagerImpl>()) {
}

CommandManager::~CommandManager() = default;

bool CommandManager::executeCommand(std::unique_ptr<AbstractCommand> command) {
    return m_pImpl->executeCommand(std::move(command));
}

bool CommandManager::canUndo() const {
    return m_pImpl->canUndo();
}

bool CommandManager::canRedo() const {
    return m_pImpl->canRedo();
}

bool CommandManager::undo() {
    return m_pImpl->undo();
}

bool CommandManager::redo() {
    return m_pImpl->redo();
}

void CommandManager::clearHistory() {
    m_pImpl->clearHistory();
}

int CommandManager::undoStackSize() const {
    return m_pImpl->undoStackSize();
}

int CommandManager::redoStackSize() const {
    return m_pImpl->redoStackSize();
}
