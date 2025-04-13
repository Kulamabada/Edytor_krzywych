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

// Commands for manipulating control points of curves
class AddControlPointCommand : public AbstractCommand {
public:
    AddControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, const Point& point, int index = -1);
    ~AddControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<InterpolatedCurve> m_curve;
    Point m_point;
    int m_index;       // Pozycja do wstawienia lub -1 dla dodania na końcu
    int m_addedIndex;  // Rzeczywisty indeks po dodaniu
};

class MoveControlPointCommand : public AbstractCommand {
public:
    MoveControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, int pointIndex, const Point& newPosition);
    ~MoveControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<InterpolatedCurve> m_curve;
    int m_pointIndex;
    Point m_oldPosition;
    Point m_newPosition;
};

class DeleteControlPointCommand : public AbstractCommand {
public:
    DeleteControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, int pointIndex);
    ~DeleteControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<InterpolatedCurve> m_curve;
    int m_pointIndex;
    Point m_deletedPoint;
};

class BatchControlPointsCommand : public AbstractCommand {
public:
    BatchControlPointsCommand(std::shared_ptr<InterpolatedCurve> curve, const std::vector<Point>& newPoints);
    ~BatchControlPointsCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<InterpolatedCurve> m_curve;
    std::vector<Point> m_oldPoints;
    std::vector<Point> m_newPoints;
};

#endif // COMMAND_H
