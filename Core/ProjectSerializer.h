#pragma once

#include "JsonSerializable.h"
#include <QJsonDocument>
#include <memory>

class Canvas;
class ViewportManager;
class GeometricObject;

class ProjectSerializer {
public:
    static bool saveProject(const QString& filePath, const Canvas* canvas);
    static bool loadProject(const QString& filePath, Canvas* canvas);

private:
    static QJsonObject serializeViewport(const ViewportManager* viewport);
    static bool deserializeViewport(const QJsonObject& json, ViewportManager* viewport);
    static QJsonArray serializeObjects(const Canvas* canvas);
    static bool deserializeObjects(const QJsonArray& json, Canvas* canvas);
    static std::shared_ptr<GeometricObject> createObjectFromJson(const QJsonObject& json);
};
