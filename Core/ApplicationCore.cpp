#include "ApplicationCore.h"
#include "CommandManager.h"
#include <stdexcept>

ApplicationCore& ApplicationCore::getInstance() {
    static ApplicationCore instance;
    return instance;
}

ApplicationCore::ApplicationCore()
    : m_initialized(false)
    , m_commandManager(std::make_unique<CommandManager>()) {
}

ApplicationCore::~ApplicationCore() {
    shutdown();
}

bool ApplicationCore::initialize() {
    if (m_initialized)
        return true;

    m_initialized = true;
    return true;
}

void ApplicationCore::shutdown() {
    if (!m_initialized)
        return;

    m_initialized = false;
}

CommandManager& ApplicationCore::getCommandManager() {
    if (!m_initialized) {
        throw std::runtime_error("ApplicationCore nie został zainicjalizowany");
    }
    return *m_commandManager;
}
