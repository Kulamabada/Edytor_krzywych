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
{
}

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

    double x = point.x();
    double y = point.y();

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

    const int numSegments = 100;
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

    if (!m_dPtr->painter || !m_dPtr->painter->isActive()) {
        return;
    }

    const Point* point = dynamic_cast<const Point*>(&object);
    if (point) {
        renderPoint(*point);
        return;
    }

    const Curve* curve = dynamic_cast<const Curve*>(&object);
    if (curve) {
        renderCurve(*curve);
        return;
    }
}

void QtRenderer::renderGrid(double gridSize)
{
    if (!m_dPtr->drawGrid || !m_dPtr->painter || !m_dPtr->viewportManager) {
        return;
    }

    QPen pen(m_dPtr->gridColor, m_dPtr->gridWidth, m_dPtr->gridStyle);
    m_dPtr->painter->setPen(pen);

    QRectF visibleRect = m_dPtr->viewportManager->visibleWorldRect();

    double xMin = std::floor(visibleRect.left() / gridSize) * gridSize;
    double xMax = std::ceil(visibleRect.right() / gridSize) * gridSize;
    double yMin = std::floor(visibleRect.top() / gridSize) * gridSize;
    double yMax = std::ceil(visibleRect.bottom() / gridSize) * gridSize;

    // Rysuj linie pionowe
    for (double x = xMin; x <= xMax; x += gridSize) {
        m_dPtr->painter->drawLine(QPointF(x, yMin), QPointF(x, yMax));
    }

    // Rysuj linie poziome
    for (double y = yMin; y <= yMax; y += gridSize) {
        m_dPtr->painter->drawLine(QPointF(xMin, y), QPointF(xMax, y));
    }
}

void QtRenderer::renderAxes()
{
    if (!m_dPtr->drawAxes || !m_dPtr->painter || !m_dPtr->viewportManager) {
        return;
    }

    QPen pen(m_dPtr->axesColor, m_dPtr->axesWidth, Qt::SolidLine);
    m_dPtr->painter->setPen(pen);

    QRectF visibleRect = m_dPtr->viewportManager->visibleWorldRect();

    // Rysuj oś X
    m_dPtr->painter->drawLine(QPointF(visibleRect.left(), 0),
                             QPointF(visibleRect.right(), 0));

    // Rysuj oś Y
    m_dPtr->painter->drawLine(QPointF(0, visibleRect.top()),
                             QPointF(0, visibleRect.bottom()));

    // Dodaj strzałki wskazujące kierunek osi
    double arrowSize = 10.0 / m_dPtr->viewportManager->zoomFactor();

    // Strzałka dla osi X
    QPolygonF arrowX;
    arrowX << QPointF(visibleRect.right(), 0)
          << QPointF(visibleRect.right() - arrowSize, arrowSize / 2)
          << QPointF(visibleRect.right() - arrowSize, -arrowSize / 2);
    m_dPtr->painter->setBrush(m_dPtr->axesColor);
    m_dPtr->painter->drawPolygon(arrowX);

    // Strzałka dla osi Y
    QPolygonF arrowY;
    arrowY << QPointF(0, visibleRect.top())
          << QPointF(arrowSize / 2, visibleRect.top() + arrowSize)
          << QPointF(-arrowSize / 2, visibleRect.top() + arrowSize);
    m_dPtr->painter->drawPolygon(arrowY);

    // Dodaj etykiety osi
    m_dPtr->painter->drawText(QPointF(visibleRect.right() - arrowSize, -arrowSize), "x");
    m_dPtr->painter->drawText(QPointF(arrowSize, visibleRect.top() + arrowSize), "y");
}

void QtRenderer::renderSelectionMarker(const QPointF& position)
{
    if (!m_dPtr->painter || !m_dPtr->painter->isActive()) {
        return;
    }

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
