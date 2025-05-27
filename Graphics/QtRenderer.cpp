#include "QtRenderer.h"
#include "../Geometry/InterpolatedCurve.h"
#include "../Geometry/ImageObject.h"
#include "../Geometry/BezierCurve.h"

QtRenderer::QtRenderer() = default;

QtRenderer::~QtRenderer() = default;

void QtRenderer::beginRender(QPainter* painter, ViewportManager* viewportManager)
{
    if (!painter || !viewportManager) {
        qDebug() << "Nieprawidłowy painter lub viewportManager";
        return;
    }
    if (!painter->isActive()) {
        qWarning() << "QPainter nie jest aktywny w beginRender()";
        return;
    }
    m_painter = painter;
    m_viewportManager = viewportManager;

    m_painter->save();
    m_painter->setTransform(viewportManager->viewTransform());
    m_painter->setRenderHint(QPainter::Antialiasing, true);
}

void QtRenderer::endRender()
{
    if (m_painter) {
        m_painter->restore();

        m_painter = nullptr;
        m_viewportManager = nullptr;
    }
}

void QtRenderer::renderPoint(const Point& point)
{
    if (!m_painter || !m_viewportManager || !m_painter->isActive())
        return;

    QPen pen(Qt::NoPen);
    QBrush brush(m_pointColor);
    m_painter->setPen(pen);
    m_painter->setBrush(brush);

    const double x = point.x(), y = point.y();

    m_painter->drawEllipse(QPointF(x, y), m_pointSize, m_pointSize);
}

void QtRenderer::renderCurve(const Curve& curve) {
    if (!m_painter || !m_viewportManager) return;

    if (const auto interpolatedCurve = dynamic_cast<const InterpolatedCurve*>(&curve)) {

        if (interpolatedCurve->getDrawInterpolatedCurve()) {
            QColor color = interpolatedCurve->getColor();
            double width = interpolatedCurve->getLineWidth();
            Qt::PenStyle style = interpolatedCurve->getLineStyle();

            const QPen pen(color, width, style);
            m_painter->setPen(pen);
            m_painter->setBrush(Qt::NoBrush);

            constexpr int numSegments = 5000;
            QPolygonF points;

            for (int i = 0; i <= numSegments; ++i) {
                double t = static_cast<double>(i) / numSegments;
                Point p = curve.getPointAt(t);
                points.append(QPointF(p.x(), p.y()));
            }

            m_painter->drawPolyline(points);
        }

        if (interpolatedCurve->getDrawPolyline())
            renderPolyline(interpolatedCurve->getPoints());

        if (interpolatedCurve->getDrawConvexHull())
            renderConvexHull(interpolatedCurve->getPoints());
    }


    const auto bezierCurve = dynamic_cast<const BezierCurve*>(&curve);
    if (bezierCurve) {
        // Renderowanie krzywej Béziera
        QColor color = bezierCurve->getColor();
        double width = bezierCurve->getLineWidth();
        Qt::PenStyle style = bezierCurve->getLineStyle();

        const QPen pen(color, width, style);
        m_painter->setPen(pen);
        m_painter->setBrush(Qt::NoBrush);

        if (bezierCurve->getControlPointCount() >= 2) {
            constexpr int numSegments = 1000;
            QPolygonF points;

            for (int i = 0; i <= numSegments; ++i) {
                double t = static_cast<double>(i) / numSegments;
                Point p = curve.getPointAt(t);
                points.append(QPointF(p.x(), p.y()));
            }

            m_painter->drawPolyline(points);
        }

        // Renderowanie wielokąta kontrolnego jeśli włączone
        if (bezierCurve->getShowControlPolygon()) {
            renderBezierControlPolygon(bezierCurve->getControlPoints(),
                                     bezierCurve->getControlPolygonColor(),
                                     bezierCurve->getControlPolygonWidth(),
                                     bezierCurve->getControlPolygonStyle());
        }
    }
}

void QtRenderer::renderObject(const GeometricObject& object)
{
    if (const auto* point = dynamic_cast<const Point*>(&object)) {
        renderPoint(*point);
        return;
    }

    if (const auto* curve = dynamic_cast<const Curve*>(&object))
        renderCurve(*curve);
}

void QtRenderer::renderGrid(double gridSize) {
    if (!m_drawGrid || !m_painter) return;

    QRectF visibleRect = m_viewportManager->visibleWorldRect();

    QPen smallPen(QColor(230, 230, 230), 0.5, Qt::DotLine);
    m_painter->setPen(smallPen);

    for (double x = std::floor(visibleRect.left()); x <= visibleRect.right(); x += 1.0) {
        m_painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
    }
    for (double y = std::floor(visibleRect.top()); y <= visibleRect.bottom(); y += 1.0) {
        m_painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
    }

    QPen mainPen(QColor(200, 200, 200), 1.0, Qt::SolidLine);
    m_painter->setPen(mainPen);

    for (double x = std::floor(visibleRect.left() / 5) * 5; x <= visibleRect.right(); x += 5.0) {
        m_painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
    }
    for (double y = std::floor(visibleRect.top() / 5) * 5; y <= visibleRect.bottom(); y += 5.0) {
        m_painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
    }
}

void QtRenderer::renderText(const QPointF& position, const QString& text) {
    if (!m_painter) return;
    m_painter->save();
    QTransform current = m_painter->transform();
    m_painter->resetTransform();
    QPointF screenPos = current.map(position);
    m_painter->drawText(screenPos, text);
    m_painter->restore();
}

void QtRenderer::renderAxes()
{
    if (!m_drawAxes || !m_painter || !m_viewportManager) return;

    QPen pen(m_axesColor, m_axesWidth, Qt::SolidLine);
    m_painter->setPen(pen);

    QRectF visibleRect = m_viewportManager->visibleWorldRect();

    m_painter->drawLine(QPointF(visibleRect.left(), 0), QPointF(visibleRect.right(), 0));
    m_painter->drawLine(QPointF(0, visibleRect.top()), QPointF(0, visibleRect.bottom()));

    double arrowSize = 7.0 / m_viewportManager->zoomFactor();

    QPolygonF arrowX;
    arrowX << QPointF(visibleRect.right(), 0)
          << QPointF(visibleRect.right() - arrowSize, arrowSize / 2)
          << QPointF(visibleRect.right() - arrowSize, -arrowSize / 2);
    m_painter->setBrush(m_axesColor);
    m_painter->drawPolygon(arrowX);

    QPolygonF arrowY;
    arrowY << QPointF(0, visibleRect.bottom())
          << QPointF(arrowSize / 2, visibleRect.bottom() - arrowSize)
          << QPointF(-arrowSize / 2, visibleRect.bottom() - arrowSize);
    m_painter->drawPolygon(arrowY);

    renderText(QPointF(visibleRect.right() - 2 * arrowSize, -arrowSize), "x");
    renderText(QPointF(arrowSize, visibleRect.bottom() - 2 * arrowSize), "y");

    renderAxisTicks();
}

void QtRenderer::renderAxisTicks() {
    if (!m_painter || !m_viewportManager || !m_painter->isActive()) return;

    QRectF visibleRect = m_viewportManager->visibleWorldRect();
    double zoomFactor = m_viewportManager->zoomFactor();

    double viewportWidth = visibleRect.width();
    double viewportHeight = visibleRect.height();

    double desiredTickCount = 16.0;
    double optimalSpacingX = viewportWidth / desiredTickCount;
    double optimalSpacingY = viewportHeight / desiredTickCount;

    auto findNiceSpacing = [](double optimalSpacing) -> double {
        double magnitude = pow(10, floor(log10(optimalSpacing)));
        double normalized = optimalSpacing / magnitude;

        if (normalized < 1.2) return 1.0 * magnitude;
        if (normalized < 2.5) return 2.0 * magnitude;
        if (normalized < 7.0) return 5.0 * magnitude;
        return 10.0 * magnitude;
    };

    double tickSpacingX = findNiceSpacing(optimalSpacingX);
    double tickSpacingY = findNiceSpacing(optimalSpacingY);

    double xMin = floor(visibleRect.left() / tickSpacingX) * tickSpacingX;
    double xMax = ceil(visibleRect.right() / tickSpacingX) * tickSpacingX;
    double yMin = floor(visibleRect.top() / tickSpacingY) * tickSpacingY;
    double yMax = ceil(visibleRect.bottom() / tickSpacingY) * tickSpacingY;

    double tickLength = 5.0 / zoomFactor;

    auto determinePrecision = [](double spacing) -> int {
        if (spacing >= 1.0) return 0;
        return std::min(6, static_cast<int>(std::ceil(std::abs(std::log10(spacing)))));
    };

    int xPrecision = determinePrecision(tickSpacingX);
    int yPrecision = determinePrecision(tickSpacingY);

    for (double x = xMin; x <= xMax; x += tickSpacingX) {
        if (std::abs(x) < 1e-6) continue;

        m_painter->drawLine(QPointF(x, -tickLength/2), QPointF(x, tickLength/2));
        QString label = QString::number(x, 'f', xPrecision);
        while (label.contains('.') && label.endsWith('0'))
            label.chop(1);

        if (label.endsWith('.'))
            label.chop(1);

        renderText(QPointF(x, -tickLength - 5.0/zoomFactor), label);
    }

    for (double y = yMin; y <= yMax; y += tickSpacingY) {
        if (std::abs(y) < 1e-6) continue;

        m_painter->drawLine(QPointF(-tickLength/2, y), QPointF(tickLength/2, y));
        QString label = QString::number(y, 'f', yPrecision);
        while (label.contains('.') && label.endsWith('0'))
            label.chop(1);

        if (label.endsWith('.'))
            label.chop(1);

        renderText(QPointF(-tickLength - 15.0/zoomFactor, y), label);
    }
}

void QtRenderer::renderSelectionMarker(const QPointF& position)
{
    if (!m_painter || !m_painter->isActive())
        return;

    m_painter->save();
    m_painter->resetTransform();

    QPen pen(m_selectionColor, 1.0, Qt::SolidLine);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    double size = m_selectionSize;
    m_painter->drawEllipse(position, size, size);

    m_painter->drawLine(position - QPointF(size / 2, 0),
                       position + QPointF(size / 2, 0));
    m_painter->drawLine(position - QPointF(0, size / 2),
                       position + QPointF(0, size / 2));

    m_painter->restore();
}

void QtRenderer::renderControlPoint(const QPointF& position, bool isSelected, bool isActive)
{
    if (!m_painter || !m_painter->isActive()) {
        return;
    }

    m_painter->save();
    m_painter->resetTransform();

    QColor fillColor;
    if (isActive) {
        fillColor = m_controlPointActiveColor;
    } else if (isSelected) {
        fillColor = m_controlPointSelectedColor;
    } else {
        fillColor = m_controlPointNormalColor;
    }

    QPen shadowPen(Qt::black, 1.0);
    QBrush shadowBrush(QColor(0, 0, 0, 50));
    m_painter->setPen(shadowPen);
    m_painter->setBrush(shadowBrush);
    m_painter->drawEllipse(position + QPointF(1, 1), m_controlPointSize + 1, m_controlPointSize + 1);

    QPen pen(Qt::black, 1.0);
    QBrush brush(fillColor);
    m_painter->setPen(pen);
    m_painter->setBrush(brush);
    m_painter->drawEllipse(position, m_controlPointSize, m_controlPointSize);

    if (isSelected) {
        m_painter->setPen(Qt::white);
        QFont font = m_painter->font();
        font.setPointSize(9);
        m_painter->setFont(font);

        QFontMetrics fm(font);
        QString text = QString::number(isActive ? 1 : 0);
        QRect textRect = fm.boundingRect(text);
        QPointF textPos = position - QPointF(textRect.width() / 2.0, -textRect.height() / 4.0);

        m_painter->drawText(textPos, text);
    }

    m_painter->restore();
}

void QtRenderer::renderControlPoints(const std::vector<Point>& points, int selectedIndex, int draggingIndex)
{
    if (!m_painter || !m_viewportManager || points.empty())
        return;

    if (m_drawControlPolygon && points.size() >= 2)
        renderControlPolygon(points);

    std::vector<QPointF> screenPositions;
    screenPositions.reserve(points.size());

    for (const auto& point : points)
        screenPositions.push_back(m_viewportManager->worldToScreen(QPointF(point.x(), point.y())));

    for (size_t i = 0; i < points.size(); ++i)
        if (static_cast<int>(i) != selectedIndex && static_cast<int>(i) != draggingIndex)
            renderControlPoint(screenPositions[i], false, false);

    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(points.size()))
        renderControlPoint(screenPositions[selectedIndex], true, false);

    if (draggingIndex >= 0 && draggingIndex < static_cast<int>(points.size()) && draggingIndex != selectedIndex)
        renderControlPoint(screenPositions[draggingIndex], false, true);


    m_painter->save();
    m_painter->resetTransform();
    QPen textPen(Qt::black);
    m_painter->setPen(textPen);

    QFont font = m_painter->font();
    font.setPointSize(8);
    m_painter->setFont(font);

    for (size_t i = 0; i < points.size(); ++i) {
        QString label = QString::number(i);
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(label);
        QPointF textPos = screenPositions[i] + QPointF(m_controlPointSize + 2,
                                                  m_controlPointSize / 2.0);

        QRectF bgRect = QRectF(textPos.x() - 1, textPos.y() - textRect.height(),
                              textRect.width() + 2, textRect.height() + 2);
        m_painter->fillRect(bgRect, QColor(255, 255, 255, 180));

        m_painter->drawText(textPos, label);
    }

    m_painter->restore();
}

void QtRenderer::renderControlPolygon(const std::vector<Point>& points)
{
    if (!m_painter || !m_viewportManager || points.size() < 2)
        return;

    m_painter->save();
    m_painter->resetTransform();

    QPen pen(m_controlPolygonColor, m_controlPolygonWidth, m_controlPolygonStyle);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPolygonF polygon;
    for (const auto& point : points) {
        QPointF screenPos = m_viewportManager->worldToScreen(QPointF(point.x(), point.y()));
        polygon << screenPos;
    }

    m_painter->drawPolyline(polygon);

    m_painter->restore();
}

void QtRenderer::setPointStyle(const QColor& color, float size)
{
    m_pointColor = color;
    m_pointSize = size;
}

void QtRenderer::setCurveStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_curveColor = color;
    m_curveWidth = width;
    m_curveStyle = style;
}

void QtRenderer::setSelectionStyle(const QColor& color, float size)
{
    m_selectionColor = color;
    m_selectionSize = size;
}

void QtRenderer::setGridStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_gridColor = color;
    m_gridWidth = width;
    m_gridStyle = style;
}

void QtRenderer::setAxesStyle(const QColor& color, float width)
{
    m_axesColor = color;
    m_axesWidth = width;
}

void QtRenderer::setControlPointStyle(const QColor& normalColor, const QColor& selectedColor,
                                const QColor& activeColor, float size)
{
    m_controlPointNormalColor = normalColor;
    m_controlPointSelectedColor = selectedColor;
    m_controlPointActiveColor = activeColor;
    m_controlPointSize = size;
}

void QtRenderer::setControlPolygonStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_controlPolygonColor = color;
    m_controlPolygonWidth = width;
    m_controlPolygonStyle = style;
}

void QtRenderer::setDrawGrid(bool draw)
{
    m_drawGrid = draw;
}

void QtRenderer::setDrawAxes(bool draw)
{
    m_drawAxes = draw;
}

void QtRenderer::setDrawControlPolygon(bool draw)
{
    m_drawControlPolygon = draw;
}

void QtRenderer::renderPolyline(const std::vector<Point>& points)
{
    if (!m_painter || !m_viewportManager || points.size() < 2)
        return;

    const QPen pen(m_polylineColor, m_polylineWidth, m_polylineStyle);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPolygonF polygon;
    for (const auto& point : points)
        polygon << QPointF(point.x(), point.y());

    m_painter->drawPolyline(polygon);
}

void QtRenderer::renderConvexHull(const std::vector<Point>& points)
{
    if (!m_painter || !m_viewportManager || points.size() < 3) return;

    const std::vector<Point> hull = computeConvexHull(points);

    if (hull.size() < 3) return;

    const QPen pen(m_convexHullColor, m_convexHullWidth, m_convexHullStyle);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPolygonF polygon;
    for (const auto& point : hull)
        polygon << QPointF(point.x(), point.y());

    if (!hull.empty())
        polygon << QPointF(hull[0].x(), hull[0].y());

    m_painter->drawPolyline(polygon);
}

void QtRenderer::setPolylineStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_polylineColor = color;
    m_polylineWidth = width;
    m_polylineStyle = style;
}

void QtRenderer::setConvexHullStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_convexHullColor = color;
    m_convexHullWidth = width;
    m_convexHullStyle = style;
}

std::vector<Point> QtRenderer::computeConvexHull(const std::vector<Point>& points) const
{
    if (points.size() < 3) return points;

    std::vector<Point> pts = points;

    const auto pivotIt = std::ranges::min_element(pts,
                                                  [](const Point& a, const Point& b) {
                                                      if (std::abs(a.y() - b.y()) < 1e-9)
                                                          return a.x() < b.x();
                                                      return a.y() < b.y();
                                                  });
    Point pivot = *pivotIt;
    std::swap(*pivotIt, pts[0]);

    std::sort(pts.begin() + 1, pts.end(),
        [this, &pivot](const Point& a, const Point& b) {
            const double angleA = polarAngle(pivot, a), angleB = polarAngle(pivot, b);

            if (std::abs(angleA - angleB) < 1e-9)
                return distanceSquared(pivot, a) < distanceSquared(pivot, b);

            return angleA < angleB;
        });

    std::vector<Point> filtered;
    filtered.push_back(pts[0]);

    for (size_t i = 1; i < pts.size(); ++i) {
        for (;i < pts.size() - 1 &&
            std::abs(polarAngle(pivot, pts[i]) - polarAngle(pivot, pts[i + 1])) < 1e-9;
            i++) {}
        filtered.push_back(pts[i]);
    }

    if (filtered.size() < 3) return filtered;

    std::vector<Point> hull;
    hull.push_back(filtered[0]);
    hull.push_back(filtered[1]);

    for (size_t i = 2; i < filtered.size(); ++i) {
        while (hull.size() > 1 && crossProduct(hull[hull.size()-2], hull[hull.size()-1], filtered[i]) <= 0)
            hull.pop_back();
        hull.push_back(filtered[i]);
    }

    return hull;
}

double QtRenderer::crossProduct(const Point& O, const Point& A, const Point& B)
{
    return (A.x() - O.x()) * (B.y() - O.y()) - (A.y() - O.y()) * (B.x() - O.x());
}

double QtRenderer::polarAngle(const Point& pivot, const Point& point)
{
    return std::atan2(point.y() - pivot.y(), point.x() - pivot.x());
}

double QtRenderer::distanceSquared(const Point& a, const Point& b)
{
    const double dx = a.x() - b.x(), dy = a.y() - b.y();
    return dx * dx + dy * dy;
}

void QtRenderer::renderBezierControlPolygon(const std::vector<Point>& points,
                                          const QColor& color,
                                          double width,
                                          Qt::PenStyle style) {
    if (!m_painter || !m_viewportManager || points.size() < 2)
        return;

    QPen pen(color, width, style);
    m_painter->setPen(pen);
    m_painter->setBrush(Qt::NoBrush);

    QPolygonF polygon;
    for (const auto& point : points) {
        polygon << QPointF(point.x(), point.y());
    }

    m_painter->drawPolyline(polygon);
}
