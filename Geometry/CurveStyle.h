#pragma once
#include "../Core/JsonSerializable.h"
#include <QColor>
#include <Qt>

struct CurveStyle : public JsonSerializable {
    QColor color{Qt::blue};
    double lineWidth{1.0};
    Qt::PenStyle lineStyle{Qt::SolidLine};

    CurveStyle() = default;
    CurveStyle(const QColor& c, double w, Qt::PenStyle s)
        : color(c), lineWidth(w), lineStyle(s) {}
    QJsonObject toJson() const override {
        QJsonObject json;
        json["color"] = color.name();
        json["lineWidth"] = lineWidth;
        json["lineStyle"] = static_cast<int>(lineStyle);
        return json;
    }

    bool fromJson(const QJsonObject& json) override {
        if (!json.contains("color")) return false;

        color = QColor(json["color"].toString());
        lineWidth = json["lineWidth"].toDouble(1.0);
        lineStyle = static_cast<Qt::PenStyle>(json["lineStyle"].toInt(Qt::SolidLine));
        return true;
    }
};

struct ControlPointStyle : public JsonSerializable {
    bool show{true};
    QColor normalColor{Qt::blue};
    QColor selectedColor{Qt::red};
    QColor activeColor{Qt::green};
    double size{5.0};

    ControlPointStyle() = default;
    ControlPointStyle(bool s, const QColor& normal, const QColor& selected,
                     const QColor& active, double sz)
        : show(s), normalColor(normal), selectedColor(selected),
          activeColor(active), size(sz) {}

    QJsonObject toJson() const override {
        QJsonObject json;
        json["show"] = show;
        json["normalColor"] = normalColor.name();
        json["selectedColor"] = selectedColor.name();
        json["activeColor"] = activeColor.name();
        json["size"] = size;
        return json;
    }

    bool fromJson(const QJsonObject& json) override {
        show = json["show"].toBool(true);
        normalColor = QColor(json["normalColor"].toString("#0000FF"));
        selectedColor = QColor(json["selectedColor"].toString("#FF0000"));
        activeColor = QColor(json["activeColor"].toString("#00FF00"));
        size = json["size"].toDouble(5.0);
        return true;
    }
};
