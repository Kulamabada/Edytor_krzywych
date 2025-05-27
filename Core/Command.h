#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include <string>
#include <QColor>
#include <Eigen/Dense>
#include "AbstractCommand.h"
#include "../Geometry/GeometricObject.h"
#include "../Geometry/Point.h"
#include "../Geometry/InterpolatedCurve.h"
#include "../Geometry/BezierCurve.h"
#include "CurveCommand.h"

// Command to add a geometric object to the scene
class AddObjectCommand : public AbstractCommand {
public:
    explicit AddObjectCommand(std::shared_ptr<GeometricObject> object);
    ~AddObjectCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<GeometricObject> m_object;
    int m_objectId{-1};
};

// Command to remove a geometric object from the scene
class RemoveObjectCommand : public AbstractCommand {
public:
    explicit RemoveObjectCommand(int objectId);
    ~RemoveObjectCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    int m_objectId;
    std::shared_ptr<GeometricObject> m_removedObject;
};

// Command to transform a geometric object
class TransformObjectCommand : public AbstractCommand {
public:
    TransformObjectCommand(int objectId, const Eigen::Matrix3d& transformation);
    ~TransformObjectCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    int m_objectId;
    Eigen::Matrix3d m_forwardTransform;
    Eigen::Matrix3d m_inverseTransform;
};

// Object style properties
struct ObjectStyle {
    QColor color;
    double lineWidth;
    Qt::PenStyle lineStyle;
};

// Command to modify object visual properties
class ModifyObjectStyleCommand : public AbstractCommand {
public:
    ModifyObjectStyleCommand(int objectId, const ObjectStyle& newStyle);
    ~ModifyObjectStyleCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    int m_objectId;
    ObjectStyle m_newStyle;
    ObjectStyle m_oldStyle;
};

// Compound command for executing multiple commands as a single operation
class CompoundCommand : public AbstractCommand {
public:
    CompoundCommand(const std::string& name);
    ~CompoundCommand() override = default;

    void addCommand(std::unique_ptr<AbstractCommand> command);
    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::vector<std::unique_ptr<AbstractCommand>> m_commands;
    std::string m_name;
};

class AddControlPointCommand : public CurveCommand {
public:
    AddControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, const Point& point, int index = -1);
    ~AddControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    Point m_point;
    int m_index;       // Pozycja do wstawienia lub -1 dla dodania na końcu
    int m_addedIndex;  // Rzeczywisty indeks po dodaniu
};

class MoveControlPointCommand : public CurveCommand {
public:
    MoveControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, int pointIndex, const Point& newPosition);
    ~MoveControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    int m_pointIndex;
    Point m_oldPosition;
    Point m_newPosition;
};

class DeleteControlPointCommand : public CurveCommand {
public:
    DeleteControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, int pointIndex);
    ~DeleteControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    int m_pointIndex;
    Point m_deletedPoint;
};

class BatchControlPointsCommand : public CurveCommand {
public:
    BatchControlPointsCommand(std::shared_ptr<InterpolatedCurve> curve, const std::vector<Point>& newPoints);
    ~BatchControlPointsCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    std::vector<Point> m_oldPoints;
    std::vector<Point> m_newPoints;
};

// Komenda do dodawania krzywej Béziera
class AddBezierCurveCommand : public AbstractCommand {
public:
    explicit AddBezierCurveCommand(std::shared_ptr<BezierCurve> curve);
    ~AddBezierCurveCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<BezierCurve> m_curve;
    int m_curveId{-1};
};

// Komendy dla edycji punktów kontrolnych krzywej Béziera
class BezierCurveCommand : public AbstractCommand {
protected:
    std::shared_ptr<BezierCurve> m_curve;

public:
    explicit BezierCurveCommand(std::shared_ptr<BezierCurve> curve) : m_curve(curve) {}
    virtual ~BezierCurveCommand() = default;

    bool redo() override {
        return execute();
    }
};

class AddBezierControlPointCommand : public BezierCurveCommand {
public:
    AddBezierControlPointCommand(std::shared_ptr<BezierCurve> curve, const Point& point, int index = -1);
    ~AddBezierControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    Point m_point;
    int m_index;
    int m_addedIndex;
};

class MoveBezierControlPointCommand : public BezierCurveCommand {
public:
    MoveBezierControlPointCommand(std::shared_ptr<BezierCurve> curve, int pointIndex, const Point& newPosition);
    ~MoveBezierControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    int m_pointIndex;
    Point m_oldPosition;
    Point m_newPosition;
};

class DeleteBezierControlPointCommand : public BezierCurveCommand {
public:
    DeleteBezierControlPointCommand(std::shared_ptr<BezierCurve> curve, int pointIndex);
    ~DeleteBezierControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    int m_pointIndex;
    Point m_deletedPoint;
};

#endif // COMMAND_H
