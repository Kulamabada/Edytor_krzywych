#pragma once

#include <QColor>
#include <Qt>

struct CurveStyle {
    QColor color{Qt::blue};
    double lineWidth{1.0};
    Qt::PenStyle lineStyle{Qt::SolidLine};

    CurveStyle() = default;
    CurveStyle(const QColor& c, double w, Qt::PenStyle s)
        : color(c), lineWidth(w), lineStyle(s) {}
};

struct ControlPointStyle {
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
};
