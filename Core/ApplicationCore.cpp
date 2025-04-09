// File: ApplicationCore.cpp
#include "ApplicationCore.h"
#include "CommandManager.h"
#include <stdexcept>

// Implementacja PIMPL
class ApplicationCore::ApplicationCoreImpl {
public:
    ApplicationCoreImpl() : m_initialized(false), m_commandManager(std::make_unique<CommandManager>()) {}
    ~ApplicationCoreImpl() = default;

    bool initialize() {
        if (m_initialized) {
            return true; // Już zainicjalizowany
        }

        // Inicjalizacja menedżera komend
        m_initialized = true;
        return true;
    }

    void shutdown() {
        if (!m_initialized) {
            return; // Już zamknięty lub nie zainicjalizowany
        }

        m_initialized = false;
    }

    CommandManager& getCommandManager() {
        if (!m_initialized) {
            throw std::runtime_error("ApplicationCore nie został zainicjalizowany");
        }
        return *m_commandManager;
    }

private:
    bool m_initialized;
    std::unique_ptr<CommandManager> m_commandManager;
};

// Implementacja Singletona
ApplicationCore& ApplicationCore::getInstance() {
    static ApplicationCore instance;
    return instance;
}

// Konstruktor i destruktor
ApplicationCore::ApplicationCore() : m_pImpl(std::make_unique<ApplicationCoreImpl>()) {
}

ApplicationCore::~ApplicationCore() {
    shutdown();
}

// Funkcjonalność główna
bool ApplicationCore::initialize() {
    return m_pImpl->initialize();
}

void ApplicationCore::shutdown() {
    m_pImpl->shutdown();
}

// Dostęp do menedżerów
CommandManager& ApplicationCore::getCommandManager() {
    return m_pImpl->getCommandManager();
}
