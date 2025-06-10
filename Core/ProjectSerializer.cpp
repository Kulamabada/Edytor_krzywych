#include "ProjectSerializer.h"
#include "../Graphics/Canvas.h"
#include "../Graphics/ViewportManager.h"
#include "../Geometry/BezierCurve.h"
#include "../Geometry/InterpolatedCurve.h"
#include "../Math/EigenMathAdapter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

bool ProjectSerializer::saveProject(const QString& filePath, const Canvas* canvas) {
    if (!canvas) return false;

    QJsonObject root;
    root["version"] = "1.0";
    root["viewport"] = serializeViewport(canvas->viewportManager());
    root["objects"] = serializeObjects(canvas);

    int selectedIndex = -1;
    auto selectedObj = canvas->selectedObject();
    if (selectedObj) {
        const auto& objects = canvas->objects();
        auto it = std::find(objects.begin(), objects.end(), selectedObj);
        if (it != objects.end()) {
            selectedIndex = std::distance(objects.begin(), it);
        }
    }
    root["selectedObjectIndex"] = selectedIndex;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(doc.toJson());
    return true;
}

bool ProjectSerializer::loadProject(const QString& filePath, Canvas* canvas) {
    if (!canvas) return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) return false;

    QJsonObject root = doc.object();
    if (!root.contains("version") || !root.contains("objects")) return false;

    canvas->clearObjects();

    if (!deserializeViewport(root["viewport"].toObject(), canvas->viewportManager())) {
        return false;
    }

    if (!deserializeObjects(root["objects"].toArray(), canvas)) {
        return false;
    }

    int selectedIndex = root["selectedObjectIndex"].toInt(-1);
    if (selectedIndex >= 0) {
        const auto& objects = canvas->objects();
        if (selectedIndex < static_cast<int>(objects.size())) {
            canvas->selectObject(objects[selectedIndex]);
        }
    }

    canvas->refresh();
    return true;
}

QJsonObject ProjectSerializer::serializeViewport(const ViewportManager* viewport) {
    QJsonObject json;
    if (!viewport) return json;

    json["zoomFactor"] = viewport->zoomFactor();

    QJsonObject panOffset;
    panOffset["x"] = viewport->panOffset().x();
    panOffset["y"] = viewport->panOffset().y();
    json["panOffset"] = panOffset;

    return json;
}

bool ProjectSerializer::deserializeViewport(const QJsonObject& json, ViewportManager* viewport) {
    if (!viewport) return false;

    viewport->setZoomFactor(json["zoomFactor"].toDouble(1.0));

    QJsonObject panOffset = json["panOffset"].toObject();
    viewport->setPanOffset(QPointF(
        panOffset["x"].toDouble(0.0),
        panOffset["y"].toDouble(0.0)
    ));

    return true;
}

QJsonArray ProjectSerializer::serializeObjects(const Canvas* canvas) {
    QJsonArray array;
    if (!canvas) return array;

    for (const auto& obj : canvas->objects()) {
        if (auto serializable = dynamic_cast<const JsonSerializable*>(obj.get())) {
            QJsonObject objJson = serializable->toJson();

            if (dynamic_cast<const BezierCurve*>(obj.get())) {
                objJson["type"] = "BezierCurve";
            } else if (dynamic_cast<const InterpolatedCurve*>(obj.get())) {
                objJson["type"] = "InterpolatedCurve";
            }

            array.append(objJson);
        }
    }

    return array;
}

bool ProjectSerializer::deserializeObjects(const QJsonArray& json, Canvas* canvas) {
    if (!canvas) return false;

    for (const auto& value : json) {
        QJsonObject objJson = value.toObject();
        auto obj = createObjectFromJson(objJson);
        if (obj) {
            canvas->addObject(obj);
        }
    }

    return true;
}

std::shared_ptr<GeometricObject> ProjectSerializer::createObjectFromJson(const QJsonObject& json) {
    QString type = json["type"].toString();

    if (type == "BezierCurve") {
        auto curve = std::make_shared<BezierCurve>();
        if (curve->fromJson(json)) {
            return curve;
        }
    } else if (type == "InterpolatedCurve") {
        auto mathProvider = std::make_shared<EigenMathAdapter>();
        auto curve = std::make_shared<InterpolatedCurve>(mathProvider, InterpolatedCurve::InterpolationType::NIFS3);
        if (curve->fromJson(json)) {
            return curve;
        }
    }

    return nullptr;
}
