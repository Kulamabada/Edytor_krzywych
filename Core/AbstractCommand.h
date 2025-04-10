// File: AbstractCommand.h
#pragma once

#include <memory>

class AbstractCommand {
public:
    virtual ~AbstractCommand() = default;

    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual bool redo() = 0;
};
