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
        , drawGrid(true)
        , drawAxes(true)
        , painter(nullptr)
        , viewportManager(nullptr)
    {}

    // Style rysowania
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

    // Opcje rysowania
    bool drawGrid;
    bool drawAxes;

    // Obiekty używane podczas rysowania
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

    constexpr int numSegments = 100;
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

    // Linie pomocnicze
    QPen smallPen(QColor(230, 230, 230), 0.5, Qt::DotLine);
    m_dPtr->painter->setPen(smallPen);

    for (double x = std::floor(visibleRect.left()); x <= visibleRect.right(); x += 1.0) {
        m_dPtr->painter->drawLine(QPointF(x, visibleRect.top()), QPointF(x, visibleRect.bottom()));
    }
    for (double y = std::floor(visibleRect.top()); y <= visibleRect.bottom(); y += 1.0) {
        m_dPtr->painter->drawLine(QPointF(visibleRect.left(), y), QPointF(visibleRect.right(), y));
    }

    // Linie główne
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

    // Rysowanie głównych osi
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

    // Obliczenie optymalnej odległości między przedziałkami w zależności od szerokości widocznego obszaru
    double viewportWidth = visibleRect.width();
    double viewportHeight = visibleRect.height();

    // Chcemy mieć około 5-10 przedziałek na widocznym obszarze
    double desiredTickCount = 16.0;
    double optimalSpacingX = viewportWidth / desiredTickCount;
    double optimalSpacingY = viewportHeight / desiredTickCount;

    // Funkcja pomocnicza do znajdowania "ładnej" wartości odstępu
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

    // Minimalna i maksymalna wartość dla przedziałek
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
        // Usunięcie zbędnych zer na końcu
        while (label.contains('.') && label.endsWith('0'))
            label.chop(1);

        if (label.endsWith('.'))
            label.chop(1);

        renderText(QPointF(x, -tickLength - 5.0/zoomFactor), label);
    }

    // Rysowanie przedziałek dla osi Y
    for (double y = yMin; y <= yMax; y += tickSpacingY) {
        if (std::abs(y) < 1e-6) continue;

        m_dPtr->painter->drawLine(QPointF(-tickLength/2, y), QPointF(tickLength/2, y));
        QString label = QString::number(y, 'f', yPrecision);
        // Usunięcie zbędnych zer na końcu
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

    // Narysuj krzyżyk w środku
    m_dPtr->painter->drawLine(position - QPointF(size / 2, 0),
                             position + QPointF(size / 2, 0));
    m_dPtr->painter->drawLine(position - QPointF(0, size / 2),
                             position + QPointF(0, size / 2));

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

void QtRenderer::setDrawGrid(bool draw)
{
    m_dPtr->drawGrid = draw;
}

void QtRenderer::setDrawAxes(bool draw)
{
    m_dPtr->drawAxes = draw;
}
