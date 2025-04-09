#ifndef APPLICATIONCORE_H
#define APPLICATIONCORE_H

#include <memory>
#include "CommandManager.h"

class ApplicationCore {
public:
    // Singleton pattern
    static ApplicationCore& getInstance();

    // Prevent copying and assignment
    ApplicationCore(const ApplicationCore&) = delete;
    ApplicationCore& operator=(const ApplicationCore&) = delete;

    // Core application functionality
    bool initialize();
    void shutdown();

    // Access to managers
    CommandManager& getCommandManager();

private:
    // Private constructor & destructor (Singleton)
    ApplicationCore();
    ~ApplicationCore();

    // PIMPL implementation
    class ApplicationCoreImpl;
    std::unique_ptr<ApplicationCoreImpl> m_pImpl;
};

#endif // APPLICATIONCORE_H
