#pragma once

#include <string>
#include "AbstractCommand.h"
#include "../Geometry/Curve.h"
#include "../Geometry/Image.h"
#include <algorithm>
#include <cassert>
#include <ranges>

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

struct ObjectStyle {
    QColor color;
    double lineWidth;
    Qt::PenStyle lineStyle;
};

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

class CurveCommand : public AbstractCommand {
protected:
    std::shared_ptr<Curve> m_curve;

public:
    explicit CurveCommand(std::shared_ptr<Curve> curve) : m_curve(curve) {}
    virtual ~CurveCommand() = default;

    bool redo() override {
        return execute();
    }
};

class AddControlPointCommand : public CurveCommand {
public:
    AddControlPointCommand(std::shared_ptr<Curve> curve, const Point& point, int index = -1);
    ~AddControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    Point m_point;
    int m_index;
    int m_addedIndex;
};

class MoveControlPointCommand : public CurveCommand {
public:
    MoveControlPointCommand(std::shared_ptr<Curve> curve, int pointIndex, const Point& newPosition);
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
    DeleteControlPointCommand(std::shared_ptr<Curve> curve, int pointIndex);
    ~DeleteControlPointCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    int m_pointIndex;
    Point m_deletedPoint;
};

class BatchControlPointsCommand : public CurveCommand {
public:
    BatchControlPointsCommand(std::shared_ptr<Curve> curve, const std::vector<Point>& newPoints);
    ~BatchControlPointsCommand() override = default;

    bool execute() override;
    bool undo() override;

private:
    std::vector<Point> m_oldPoints;
    std::vector<Point> m_newPoints;
};

class AddImageCommand : public AbstractCommand {
public:
    explicit AddImageCommand(std::shared_ptr<Image> image);
    ~AddImageCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<Image> m_image;
    int m_imageId{-1};
};

class TransformImageCommand : public AbstractCommand {
public:
    TransformImageCommand(std::shared_ptr<Image> image, const Vector& newPosition,
                         double newWidth, double newHeight);
    ~TransformImageCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<Image> m_image;
    Vector m_oldPosition;
    Vector m_newPosition;
    double m_oldWidth;
    double m_oldHeight;
    double m_newWidth;
    double m_newHeight;
};

class MoveImageCommand : public AbstractCommand {
public:
    MoveImageCommand(std::shared_ptr<Image> image, const Vector& newPosition);
    ~MoveImageCommand() override = default;

    bool execute() override;
    bool undo() override;
    bool redo() override;

private:
    std::shared_ptr<Image> m_image;
    Vector m_oldPosition;
    Vector m_newPosition;
};
