#include "Command.h"
#include <algorithm>
#include <cassert>

namespace {
    int addObjectToScene(std::shared_ptr<GeometricObject> object);

    bool removeObjectFromScene(int objectId);

    std::shared_ptr<GeometricObject> getObjectFromScene(int objectId);

    bool transformObject(int objectId, const Eigen::Matrix3d& transform);

    ObjectStyle getObjectStyle(int objectId);

    bool setObjectStyle(int objectId, const ObjectStyle& style);
}

AddObjectCommand::AddObjectCommand(std::shared_ptr<GeometricObject> object)
    : m_object(std::move(object)), m_objectId(-1) {
}

bool AddObjectCommand::execute() {
    if (!m_object) {
        return false;
    }

    m_objectId = addObjectToScene(m_object);
    return m_objectId >= 0;
}

bool AddObjectCommand::undo() {
    if (m_objectId < 0) {
        return false;
    }
    return removeObjectFromScene(m_objectId);
}

bool AddObjectCommand::redo() {
    return execute();
}

RemoveObjectCommand::RemoveObjectCommand(int objectId)
    : m_objectId(objectId), m_removedObject(nullptr) {
}

bool RemoveObjectCommand::execute() {
    m_removedObject = getObjectFromScene(m_objectId);
    if (!m_removedObject) {
        return false;
    }

    return removeObjectFromScene(m_objectId);
}

bool RemoveObjectCommand::undo() {
    if (!m_removedObject) {
        return false;
    }

    m_objectId = addObjectToScene(m_removedObject);
    return m_objectId >= 0;
}

bool RemoveObjectCommand::redo() {
    return execute();
}

TransformObjectCommand::TransformObjectCommand(int objectId, const Eigen::Matrix3d& transformation)
    : m_objectId(objectId), m_forwardTransform(transformation) {
    m_inverseTransform = transformation.inverse();
}

bool TransformObjectCommand::execute() {
    return transformObject(m_objectId, m_forwardTransform);
}

bool TransformObjectCommand::undo() {
    return transformObject(m_objectId, m_inverseTransform);
}

bool TransformObjectCommand::redo() {
    return execute();
}

ModifyObjectStyleCommand::ModifyObjectStyleCommand(int objectId, const ObjectStyle& newStyle)
    : m_objectId(objectId), m_newStyle(newStyle) {
    m_oldStyle = getObjectStyle(m_objectId);
}

bool ModifyObjectStyleCommand::execute() {
    return setObjectStyle(m_objectId, m_newStyle);
}

bool ModifyObjectStyleCommand::undo() {
    return setObjectStyle(m_objectId, m_oldStyle);
}

bool ModifyObjectStyleCommand::redo() {
    return execute();
}

CompoundCommand::CompoundCommand(const std::string& name)
    : m_name(name) {
}

void CompoundCommand::addCommand(std::unique_ptr<AbstractCommand> command) {
    if (command) {
        m_commands.push_back(std::move(command));
    }
}

bool CompoundCommand::execute() {
    bool success = true;
    size_t executedCommands = 0;

    for (size_t i = 0; i < m_commands.size(); ++i) {
        if (!m_commands[i]->execute()) {
            success = false;
            executedCommands = i;
            break;
        }
        executedCommands = i + 1;
    }

    if (!success) {
        for (size_t i = 0; i < executedCommands; ++i) {
            size_t index = executedCommands - i - 1;
            m_commands[index]->undo();
        }
    }

    return success;
}

bool CompoundCommand::undo() {
    bool success = true;

    for (auto it = m_commands.rbegin(); it != m_commands.rend(); ++it) {
        if (!(*it)->undo()) {
            success = false;
            break;
        }
    }

    return success;
}

bool CompoundCommand::redo() {
    bool success = true;

    for (auto& command : m_commands) {
        if (!command->redo()) {
            success = false;
            break;
        }
    }

    return success;
}


AddControlPointCommand::AddControlPointCommand(std::shared_ptr<InterpolatedCurve> curve, const Point& point, int index)
    : m_curve(curve), m_point(point), m_index(index), m_addedIndex(-1) {
}

bool AddControlPointCommand::execute() {
    if (!m_curve) {
        return false;
    }

    if (m_index < 0 || m_index > static_cast<int>(m_curve->getPointCount())) {
        m_curve->addPoint(m_point);
        m_addedIndex = static_cast<int>(m_curve->getPointCount()) - 1;
    } else {
        m_curve->insertPoint(m_index, m_point);
        m_addedIndex = m_index;
    }

    return true;
}

bool AddControlPointCommand::undo() {
    if (!m_curve || m_addedIndex < 0 ||
        m_addedIndex >= static_cast<int>(m_curve->getPointCount())) {
        return false;
    }

    m_curve->removePoint(m_addedIndex);
    return true;
}

bool AddControlPointCommand::redo() {
    if (!m_curve) {
        return false;
    }

    if (m_addedIndex < 0) {
        return execute();
    }

    if (m_addedIndex > static_cast<int>(m_curve->getPointCount())) {
        m_curve->addPoint(m_point);
    } else {
        m_curve->insertPoint(m_addedIndex, m_point);
    }

    return true;
}

MoveControlPointCommand::MoveControlPointCommand(std::shared_ptr<InterpolatedCurve> curve,
                                               int pointIndex,
                                               const Point& newPosition)
    : m_curve(curve), m_pointIndex(pointIndex), m_newPosition(newPosition) {

    if (m_curve && m_pointIndex >= 0 && m_pointIndex < static_cast<int>(m_curve->getPointCount())) {
        const auto& points = m_curve->getPoints();
        m_oldPosition = points[m_pointIndex];
    }
}

bool MoveControlPointCommand::execute() {
    if (!m_curve || m_pointIndex < 0 ||
        m_pointIndex >= static_cast<int>(m_curve->getPointCount())) {
        return false;
    }

    m_curve->movePoint(m_pointIndex, m_newPosition);
    return true;
}

bool MoveControlPointCommand::undo() {
    if (!m_curve || m_pointIndex < 0 ||
        m_pointIndex >= static_cast<int>(m_curve->getPointCount())) {
        return false;
    }

    m_curve->movePoint(m_pointIndex, m_oldPosition);
    return true;
}

bool MoveControlPointCommand::redo() {
    return execute();
}

DeleteControlPointCommand::DeleteControlPointCommand(std::shared_ptr<InterpolatedCurve> curve,
                                                  int pointIndex)
    : m_curve(curve), m_pointIndex(pointIndex) {

    if (m_curve && m_pointIndex >= 0 && m_pointIndex < static_cast<int>(m_curve->getPointCount())) {
        const auto& points = m_curve->getPoints();
        m_deletedPoint = points[m_pointIndex];
    }
}

bool DeleteControlPointCommand::execute() {
    if (!m_curve || m_pointIndex < 0 ||
        m_pointIndex >= static_cast<int>(m_curve->getPointCount())) {
        return false;
    }

    if (m_curve->getPointCount() < 3) {
        return false;
    }

    m_curve->removePoint(m_pointIndex);
    return true;
}

bool DeleteControlPointCommand::undo() {
    if (!m_curve) {
        return false;
    }

    if (m_pointIndex >= static_cast<int>(m_curve->getPointCount())) {
        m_curve->addPoint(m_deletedPoint);
    } else {
        m_curve->insertPoint(m_pointIndex, m_deletedPoint);
    }

    return true;
}

bool DeleteControlPointCommand::redo() {
    return execute();
}

BatchControlPointsCommand::BatchControlPointsCommand(std::shared_ptr<InterpolatedCurve> curve,
                                                   const std::vector<Point>& newPoints)
    : m_curve(curve), m_newPoints(newPoints) {

    if (m_curve) {
        m_oldPoints = m_curve->getPoints();
    }
}

bool BatchControlPointsCommand::execute() {
    if (!m_curve) {
        return false;
    }

    if (m_newPoints.size() < 2) {
        return false;
    }

    m_curve->setPoints(m_newPoints);
    return true;
}

bool BatchControlPointsCommand::undo() {
    if (!m_curve) {
        return false;
    }

    m_curve->setPoints(m_oldPoints);
    return true;
}

bool BatchControlPointsCommand::redo() {
    return execute();
}

namespace {
    int addObjectToScene(std::shared_ptr<GeometricObject> object) {
        static int nextId = 0;
        return ++nextId;
    }

    bool removeObjectFromScene(int objectId) {
        return objectId > 0;
    }

    std::shared_ptr<GeometricObject> getObjectFromScene(int objectId) {
        return nullptr;
    }

    bool transformObject(int objectId, const Eigen::Matrix3d& transform) {
        return objectId > 0;
    }

    ObjectStyle getObjectStyle(int objectId) {
        return ObjectStyle{QColor(0, 0, 0), 1.0, Qt::SolidLine};
    }

    bool setObjectStyle(int objectId, const ObjectStyle& style) {
        return objectId > 0;
    }
}
