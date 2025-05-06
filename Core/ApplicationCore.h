#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <memory>
#include "CommandManager.h"

class ApplicationCore {
public:
    static ApplicationCore& getInstance();

    ApplicationCore(const ApplicationCore&) = delete;
    ApplicationCore& operator=(const ApplicationCore&) = delete;

    bool initialize();
    void shutdown();

    CommandManager& getCommandManager();

private:
    ApplicationCore();
    ~ApplicationCore();

    bool m_initialized{false};
    std::unique_ptr<CommandManager> m_commandManager;
};

#endif // APPLICATIONCORE_H
