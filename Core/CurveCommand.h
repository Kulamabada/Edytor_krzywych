#pragma once

#include "AbstractCommand.h"
#include "../Geometry/InterpolatedCurve.h"

class CurveCommand : public AbstractCommand {
protected:
    std::shared_ptr<InterpolatedCurve> m_curve;

public:
    explicit CurveCommand(std::shared_ptr<InterpolatedCurve> curve) : m_curve(curve) {}
    virtual ~CurveCommand() = default;

    bool redo() override {
        return execute();
    }
};
