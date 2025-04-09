#ifndef CORE_APPLICATIONCORE_H
#define CORE_APPLICATIONCORE_H

#include <memory>
#include "Core/CommandManager/CommandManager.h"

namespace Core {

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
    CommandManager::CommandManager& getCommandManager();
    
private:
    // Private constructor & destructor (Singleton)
    ApplicationCore();
    ~ApplicationCore();
    
    // PIMPL implementation
    class ApplicationCoreImpl;
    std::unique_ptr<ApplicationCoreImpl> pImpl;
};

} // namespace Core

#endif // CORE_APPLICATIONCORE_H
