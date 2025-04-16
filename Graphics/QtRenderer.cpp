#include "QtRenderer.h"

class QtRenderer::QtRendererPrivate {
public:
    QtRendererPrivate()
        : pointColor(Qt::black)
        , pointSize(4.0f)
        , curveColor(Qt::blue)
        , curveWidth(1.0f)
        , curveStyle(Qt::SolidLine)
        , selectionColor(Qt::red)
        , selectionSize(6.0f)
        , gridColor(Qt::lightGray)
        , gridWidth(0.5f)
        , gridStyle(Qt::DotLine)
        , axesColor(Qt::black)
        , axesWidth(1.0f)
        , controlPointNormalColor(Qt::blue)
        , controlPointSelectedColor(Qt::red)
        , controlPointActiveColor(Qt::green)
        , controlPointSize(5.0f)
        , controlPolygonColor(Qt::darkGray)
        , controlPolygonWidth(1.0f)
        , controlPolygonStyle(Qt::DashLine)
        , drawGrid(true)
        , drawAxes(true)
        , drawControlPolygon(true)
        , painter(nullptr)
        , viewportManager(nullptr)
    {}

    QColor pointColor;
    float pointSize;
    QColor curveColor;
    float curveWidth;
    Qt::PenStyle curveStyle;
    QColor selectionColor;
    float selectionSize;
    QColor gridColor;
    float gridWidth;
    Qt::PenStyle gridStyle;
    QColor axesColor;
    float axesWidth;
    QColor controlPointNormalColor;
    QColor controlPointSelectedColor;
    QColor controlPointActiveColor;
    float controlPointSize;
    QColor controlPolygonColor;
    float controlPolygonWidth;
    Qt::PenStyle controlPolygonStyle;

    bool drawGrid;
    bool drawAxes;
    bool drawControlPolygon;

    QPainter* painter;
    ViewportManager* viewportManager;
};

QtRenderer::QtRenderer()
    : m_dPtr(new QtRendererPrivate())
{}

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

    m_dPtr->painter = painter;
    m_dPtr->viewportManager = viewportManager;

    m_dPtr->painter->save();
    m_dPtr->painter->setTransform(viewportManager->viewTransform());
    m_dPtr->painter->setRenderHint(QPainter::Antialiasing, true);
}

void QtRenderer::endRender()
{
    if (m_dPtr->painter) {
        m_dPtr->painter->restore();

        m_dPtr->painter = nullptr;
        m_dPtr->viewportManager = nullptr;
    }
}

void QtRenderer::renderPoint(const Point& point)
{
    if (!m_dPtr->painter || !m_dPtr->viewportManager || !m_dPtr->painter->isActive()) {
        return;
    }

    QPen pen(Qt::NoPen);
    QBrush brush(m_dPtr->pointColor);
    m_dPtr->painter->setPen(pen);
    m_dPtr->painter->setBrush(brush);

    const double x = point.x(), y = point.y();

    m_dPtr->painter->drawEllipse(QPointF(x, y), m_dPtr->pointSize, m_dPtr->pointSize);
}

void QtRenderer::renderCurve(const Curve& curve)
{
    if (!m_dPtr->painter || !m_dPtr->viewportManager) {
        return;
    }

    QPen pen(m_dPtr->curveColor, m_dPtr->curveWidth, m_dPtr->curveStyle);
    m_dPtr->painter->setPen(pen);
    m_dPtr->painter->setBrush(Qt::NoBrush);

    constexpr int numSegments = 5000;
    QPolygonF points;

    for (int i = 0; i <= numSegments; ++i) {
        double t = static_cast<double>(i) / numSegments;
        Point p = curve.getPointAt(t);
        points.append(QPointF(p.x(), p.y()));
    }

    m_dPtr->painter->drawPolyline(points);
}

void QtRenderer::renderObject(const GeometricObject& object)
{
    const Point* point = dynamic_cast<const Point*>(&object);
    if (point) {
        renderPoint(*point);
        return;
    }

    const Curve* curve = dynamic_cast<const Curve*>(&object);
    if (curve)
        renderCurve(*curve);
}

void QtRenderer::renderGrid(double gridSize) {
    if (!m_dPtr->drawGrid || !m_dPtr->painter) return;

    QRectF visibleRect = m_dPtr->viewportManager->visibleWorldRect();

    QPen smallPen(QColor(230, 230, 230), 0.5, Qt::DotLine);
    m_dPtr->painter->setPen(smallPen);

    for (double x = std::floor(visibleRect.left()); x <= visibleRect.right(); x += 1.0) {
        m_dPtr->painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
    }
    for (double y = std::floor(visibleRect.top()); y <= visibleRect.bottom(); y += 1.0) {
        m_dPtr->painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
    }

    QPen mainPen(QColor(200, 200, 200), 1.0, Qt::SolidLine);
    m_dPtr->painter->setPen(mainPen);

    for (double x = std::floor(visibleRect.left() / 5) * 5; x <= visibleRect.right(); x += 5.0) {
        m_dPtr->painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
    }
    for (double y = std::floor(visibleRect.top() / 5) * 5; y <= visibleRect.bottom(); y += 5.0) {
        m_dPtr->painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
    }
}

void QtRenderer::renderText(const QPointF& position, const QString& text) {
    if (!m_dPtr->painter) return;
    m_dPtr->painter->save();
    QTransform current = m_dPtr->painter->transform();
    m_dPtr->painter->resetTransform();
    QPointF screenPos = current.map(position);
    m_dPtr->painter->drawText(screenPos, text);
    m_dPtr->painter->restore();
}

void QtRenderer::renderAxes()
{
    if (!m_dPtr->drawAxes || !m_dPtr->painter || !m_dPtr->viewportManager) return;

    QPen pen(m_dPtr->axesColor, m_dPtr->axesWidth, Qt::SolidLine);
    m_dPtr->painter->setPen(pen);

    QRectF visibleRect = m_dPtr->viewportManager->visibleWorldRect();

    m_dPtr->painter->drawLine(QPointF(visibleRect.left(), 0), QPointF(visibleRect.right(), 0));
    m_dPtr->painter->drawLine(QPointF(0, visibleRect.top()), QPointF(0, visibleRect.bottom()));

    double arrowSize = 7.0 / m_dPtr->viewportManager->zoomFactor();

    QPolygonF arrowX;
    arrowX << QPointF(visibleRect.right(), 0)
          << QPointF(visibleRect.right() - arrowSize, arrowSize / 2)
          << QPointF(visibleRect.right() - arrowSize, -arrowSize / 2);
    m_dPtr->painter->setBrush(m_dPtr->axesColor);
    m_dPtr->painter->drawPolygon(arrowX);

    QPolygonF arrowY;
    arrowY << QPointF(0, visibleRect.bottom())
          << QPointF(arrowSize / 2, visibleRect.bottom() - arrowSize)
          << QPointF(-arrowSize / 2, visibleRect.bottom() - arrowSize);
    m_dPtr->painter->drawPolygon(arrowY);

    renderText(QPointF(visibleRect.right() - 2 * arrowSize, -arrowSize), "x");
    renderText(QPointF(arrowSize, visibleRect.bottom() - 2 * arrowSize), "y");

    renderAxisTicks();
}

void QtRenderer::renderAxisTicks() {
    if (!m_dPtr->painter || !m_dPtr->viewportManager || !m_dPtr->painter->isActive()) return;

    QRectF visibleRect = m_dPtr->viewportManager->visibleWorldRect();
    double zoomFactor = m_dPtr->viewportManager->zoomFactor();

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

        m_dPtr->painter->drawLine(QPointF(x, -tickLength/2), QPointF(x, tickLength/2));
        QString label = QString::number(x, 'f', xPrecision);
        while (label.contains('.') && label.endsWith('0'))
            label.chop(1);

        if (label.endsWith('.'))
            label.chop(1);

        renderText(QPointF(x, -tickLength - 5.0/zoomFactor), label);
    }

    for (double y = yMin; y <= yMax; y += tickSpacingY) {
        if (std::abs(y) < 1e-6) continue;

        m_dPtr->painter->drawLine(QPointF(-tickLength/2, y), QPointF(tickLength/2, y));
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
    if (!m_dPtr->painter || !m_dPtr->painter->isActive())
        return;

    m_dPtr->painter->save();
    m_dPtr->painter->resetTransform();

    QPen pen(m_dPtr->selectionColor, 1.0, Qt::SolidLine);
    m_dPtr->painter->setPen(pen);
    m_dPtr->painter->setBrush(Qt::NoBrush);

    double size = m_dPtr->selectionSize;
    m_dPtr->painter->drawEllipse(position, size, size);

    m_dPtr->painter->drawLine(position - QPointF(size / 2, 0),
                             position + QPointF(size / 2, 0));
    m_dPtr->painter->drawLine(position - QPointF(0, size / 2),
                             position + QPointF(0, size / 2));

    m_dPtr->painter->restore();
}

void QtRenderer::renderControlPoint(const QPointF& position, bool isSelected, bool isActive)
{
    if (!m_dPtr->painter || !m_dPtr->painter->isActive()) {
        return;
    }

    m_dPtr->painter->save();
    m_dPtr->painter->resetTransform();

    QColor fillColor;
    if (isActive) {
        fillColor = m_dPtr->controlPointActiveColor;
    } else if (isSelected) {
        fillColor = m_dPtr->controlPointSelectedColor;
    } else {
        fillColor = m_dPtr->controlPointNormalColor;
    }

    QPen shadowPen(Qt::black, 1.0);
    QBrush shadowBrush(QColor(0, 0, 0, 50));
    m_dPtr->painter->setPen(shadowPen);
    m_dPtr->painter->setBrush(shadowBrush);
    m_dPtr->painter->drawEllipse(position + QPointF(1, 1), m_dPtr->controlPointSize + 1, m_dPtr->controlPointSize + 1);

    QPen pen(Qt::black, 1.0);
    QBrush brush(fillColor);
    m_dPtr->painter->setPen(pen);
    m_dPtr->painter->setBrush(brush);
    m_dPtr->painter->drawEllipse(position, m_dPtr->controlPointSize, m_dPtr->controlPointSize);

    if (isSelected) {
        m_dPtr->painter->setPen(Qt::white);
        QFont font = m_dPtr->painter->font();
        font.setPointSize(9);
        m_dPtr->painter->setFont(font);

        QFontMetrics fm(font);
        QString text = QString::number(isActive ? 1 : 0);
        QRect textRect = fm.boundingRect(text);
        QPointF textPos = position - QPointF(textRect.width() / 2.0, -textRect.height() / 4.0);

        m_dPtr->painter->drawText(textPos, text);
    }

    m_dPtr->painter->restore();
}

void QtRenderer::renderControlPoints(const std::vector<Point>& points, int selectedIndex, int activeIndex)
{
    if (!m_dPtr->painter || !m_dPtr->viewportManager || points.empty()) {
        return;
    }

    if (m_dPtr->drawControlPolygon && points.size() >= 2) {
        renderControlPolygon(points);
    }

    std::vector<QPointF> screenPositions;
    screenPositions.reserve(points.size());

    for (const auto& point : points) {
        screenPositions.push_back(
            m_dPtr->viewportManager->worldToScreen(QPointF(point.x(), point.y()))
        );
    }

    for (size_t i = 0; i < points.size(); ++i) {
        if (static_cast<int>(i) != selectedIndex && static_cast<int>(i) != activeIndex) {
            renderControlPoint(screenPositions[i], false, false);
        }
    }

    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(points.size())) {
        renderControlPoint(screenPositions[selectedIndex], true, false);
    }

    if (activeIndex >= 0 && activeIndex < static_cast<int>(points.size()) && activeIndex != selectedIndex) {
        renderControlPoint(screenPositions[activeIndex], false, true);
    }

    m_dPtr->painter->save();
    m_dPtr->painter->resetTransform();
    QPen textPen(Qt::black);
    m_dPtr->painter->setPen(textPen);

    QFont font = m_dPtr->painter->font();
    font.setPointSize(8);
    m_dPtr->painter->setFont(font);

    for (size_t i = 0; i < points.size(); ++i) {
        QString label = QString::number(i);
        QFontMetrics fm(font);
        QRect textRect = fm.boundingRect(label);

        QPointF textPos = screenPositions[i] + QPointF(m_dPtr->controlPointSize + 2,
                                                      m_dPtr->controlPointSize / 2.0);

        QRectF bgRect = QRectF(textPos.x() - 1, textPos.y() - textRect.height(),
                              textRect.width() + 2, textRect.height() + 2);
        m_dPtr->painter->fillRect(bgRect, QColor(255, 255, 255, 180));

        m_dPtr->painter->drawText(textPos, label);
    }

    m_dPtr->painter->restore();
}

void QtRenderer::renderControlPolygon(const std::vector<Point>& points)
{
    if (!m_dPtr->painter || !m_dPtr->viewportManager || points.size() < 2) {
        return;
    }

    m_dPtr->painter->save();
    m_dPtr->painter->resetTransform();

    QPen pen(m_dPtr->controlPolygonColor, m_dPtr->controlPolygonWidth, m_dPtr->controlPolygonStyle);
    m_dPtr->painter->setPen(pen);
    m_dPtr->painter->setBrush(Qt::NoBrush);

    QPolygonF polygon;
    for (const auto& point : points) {
        QPointF screenPos = m_dPtr->viewportManager->worldToScreen(QPointF(point.x(), point.y()));
        polygon << screenPos;
    }

    m_dPtr->painter->drawPolyline(polygon);

    m_dPtr->painter->restore();
}

void QtRenderer::setPointStyle(const QColor& color, float size)
{
    m_dPtr->pointColor = color;
    m_dPtr->pointSize = size;
}

void QtRenderer::setCurveStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_dPtr->curveColor = color;
    m_dPtr->curveWidth = width;
    m_dPtr->curveStyle = style;
}

void QtRenderer::setSelectionStyle(const QColor& color, float size)
{
    m_dPtr->selectionColor = color;
    m_dPtr->selectionSize = size;
}

void QtRenderer::setGridStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_dPtr->gridColor = color;
    m_dPtr->gridWidth = width;
    m_dPtr->gridStyle = style;
}

void QtRenderer::setAxesStyle(const QColor& color, float width)
{
    m_dPtr->axesColor = color;
    m_dPtr->axesWidth = width;
}

void QtRenderer::setControlPointStyle(const QColor& normalColor, const QColor& selectedColor,
                                    const QColor& activeColor, float size)
{
    m_dPtr->controlPointNormalColor = normalColor;
    m_dPtr->controlPointSelectedColor = selectedColor;
    m_dPtr->controlPointActiveColor = activeColor;
    m_dPtr->controlPointSize = size;
}

void QtRenderer::setControlPolygonStyle(const QColor& color, float width, Qt::PenStyle style)
{
    m_dPtr->controlPolygonColor = color;
    m_dPtr->controlPolygonWidth = width;
    m_dPtr->controlPolygonStyle = style;
}

void QtRenderer::setDrawGrid(bool draw)
{
    m_dPtr->drawGrid = draw;
}

void QtRenderer::setDrawAxes(bool draw)
{
    m_dPtr->drawAxes = draw;
}

void QtRenderer::setDrawControlPolygon(bool draw)
{
    m_dPtr->drawControlPolygon = draw;
}
