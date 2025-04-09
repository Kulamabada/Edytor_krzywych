// File: AbstractCommand.h
#pragma once

#include <memory>

namespace Core {
    namespace CommandManager {

        class AbstractCommand {
        public:
            virtual ~AbstractCommand() = default;

            virtual void execute() = 0;
            virtual void undo() = 0;
            virtual void redo() = 0;
        };

    } // namespace CommandManager
} // namespace Core
