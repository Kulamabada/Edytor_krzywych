#include "Curve.h"
#include "TransformMatrix.h"

DefaultCurveConfig Curve::createDefaultConfig(CurveType type) {
    DefaultCurveConfig config;

    switch (type) {
        case CurveType::Bezier:
            config.lineWidth = 2.0;
            config.color = Qt::blue;
            config.showControlPoints = true;
            break;

        case CurveType::Interpolated:
            config.lineWidth = 1.0;
            config.color = Qt::blue;
            config.showControlPoints = true;
            break;

        case CurveType::Generic:
        default:
            config.lineWidth = 1.5;
            config.color = Qt::black;
            config.showControlPoints = false;
            break;
    }

    return config;
}

Curve::Curve(CurveType type) : m_type(type) {
    DefaultCurveConfig config = createDefaultConfig(type);
    initializeFromConfig(config);
    initializePrecisionConfig(type);
}

Curve::Curve(const DefaultCurveConfig& config) : m_type(CurveType::Generic) {
    initializeFromConfig(config);
    initializePrecisionConfig(CurveType::Generic);
}

void Curve::initializePrecisionConfig(CurveType type) {
    switch (type) {
        case CurveType::Bezier:
            m_precisionConfig.lengthSamples = 1000;
            m_precisionConfig.tangentEpsilon = 1e-6;
            m_precisionConfig.renderSamples = 1000;
            break;

        case CurveType::Interpolated:
            m_precisionConfig.lengthSamples = 200;
            m_precisionConfig.tangentEpsilon = 1e-6;
            m_precisionConfig.renderSamples = 200;
            break;

        case CurveType::Generic:
        default:
            m_precisionConfig.lengthSamples = 500;
            m_precisionConfig.tangentEpsilon = 1e-6;
            m_precisionConfig.renderSamples = 500;
            break;
    }
}

void Curve::initializeFromConfig(const DefaultCurveConfig& config) {
    m_style.color = config.color;
    m_style.lineWidth = config.lineWidth;
    m_style.lineStyle = config.lineStyle;

    m_controlPointStyle.show = config.showControlPoints;
    m_controlPointStyle.normalColor = config.controlPointNormalColor;
    m_controlPointStyle.selectedColor = config.controlPointSelectedColor;
    m_controlPointStyle.activeColor = config.controlPointActiveColor;
    m_controlPointStyle.size = config.controlPointSize;

    m_drawPolyline = config.drawPolyline;
    m_drawConvexHull = config.drawConvexHull;
    m_drawMainCurve = true;

    m_polylineColor = config.polylineColor;
    m_polylineWidth = config.polylineWidth;
    m_polylineStyle = config.polylineStyle;

    m_convexHullColor = config.convexHullColor;
    m_convexHullWidth = config.convexHullWidth;
    m_convexHullStyle = config.convexHullStyle;
}

double Curve::getLength() const {
    return calculateLengthByDiscretization();
}

std::unique_ptr<Curve> Curve::clone() const {
    auto copy = createClone();
    copy->setControlPoints(m_controlPoints);
    copyStylesTo(copy.get());
    copySpecificPropertiesTo(copy.get());
    return copy;
}

void Curve::addControlPoint(const Point& point) {
    m_controlPoints.push_back(point);
    onControlPointsChanged();
}

void Curve::insertControlPoint(size_t index, const Point& point) {
    if (index <= m_controlPoints.size()) {
        m_controlPoints.insert(m_controlPoints.begin() + index, point);
        onControlPointsChanged();
    }
}

void Curve::removeControlPoint(size_t index) {
    if (index < m_controlPoints.size()) {
        m_controlPoints.erase(m_controlPoints.begin() + index);
        onControlPointsChanged();
    }
}

void Curve::setControlPoints(const std::vector<Point>& points) {
    m_controlPoints = points;
    onControlPointsChanged();
}

void Curve::moveControlPoint(size_t index, const Point& newPosition) {
    if (index < m_controlPoints.size()) {
        m_controlPoints[index] = newPosition;
        onControlPointsChanged();
    }
}

const std::vector<Point>& Curve::getControlPoints() const {
    return m_controlPoints;
}

size_t Curve::getControlPointCount() const {
    return m_controlPoints.size();
}

void Curve::copyStylesTo(Curve* target) const {
    if (!target) return;

    target->setStyle(m_style);
    target->setControlPointStyle(m_controlPointStyle);
    target->setDrawPolyline(m_drawPolyline);
    target->setDrawConvexHull(m_drawConvexHull);
    target->setPolylineColor(m_polylineColor);
    target->setPolylineWidth(m_polylineWidth);
    target->setPolylineStyle(m_polylineStyle);
    target->setConvexHullColor(m_convexHullColor);
    target->setConvexHullWidth(m_convexHullWidth);
    target->setConvexHullStyle(m_convexHullStyle);
    target->setDrawMainCurve(m_drawMainCurve);
    target->setPrecisionConfig(m_precisionConfig);
}

Vector Curve::getPosition() const {
    if (!hasControlPoints() || getControlPointCount() == 0)
        return Vector::Zero();

    const auto& points = getControlPoints();
    Vector sum = Vector::Zero();
    for (const auto& point : points)
        sum += Vector(point.x(), point.y(), point.z());

    return sum / static_cast<double>(points.size());
}

void Curve::setPosition(const Eigen::Vector3d& position) {
    if (!hasControlPoints() || getControlPointCount() == 0)
        return;

    const Vector currentCenter = getPosition();
    Vector delta = position - currentCenter;

    auto points = getControlPoints();
    for (auto& point : points) {
        point.setX(point.x() + delta.x());
        point.setY(point.y() + delta.y());
        point.setZ(point.z() + delta.z());
    }
    setControlPoints(points);
}

void Curve::transform(const TransformMatrix& matrix) {
    if (!hasControlPoints())
        return;

    auto points = getControlPoints();
    for (auto& point : points)
        point.transform(matrix);
    setControlPoints(points);
}

double Curve::calculateLengthByDiscretization(int numSamples) const {
    if (m_controlPoints.size() < 2)
        return 0.0;

    if (numSamples <= 0)
        numSamples = getDefaultLengthSamples();

    double length = 0.0;
    Point prevPoint = getPointAt(0.0);

    for (int i = 1; i <= numSamples; ++i) {
        double t = static_cast<double>(i) / numSamples;
        Point currentPoint = getPointAt(t);
        length += prevPoint.distanceTo(currentPoint);
        prevPoint = currentPoint;
    }

    return length;
}

bool Curve::validateParameterRange(double& t) const {
    t = std::max(0.0, std::min(1.0, t));
    return t >= 0.0 && t <= 1.0;
}

Point Curve::interpolateLinear(const Point& p1, const Point& p2, double t) const {
    const double oneMinusT = 1.0 - t;
    return Point(
        oneMinusT * p1.x() + t * p2.x(),
        oneMinusT * p1.y() + t * p2.y(),
        oneMinusT * p1.z() + t * p2.z()
    );
}

double Curve::calculateParameterForSegment(size_t segmentIndex, size_t totalSegments, double localT) const {
    double segmentLength = 1.0 / totalSegments;
    return segmentIndex * segmentLength + localT * segmentLength;
}

QJsonObject Curve::serializeCurveData() const {
    QJsonObject json;

    QJsonArray pointsArray;
    for (const auto& point : m_controlPoints) {
        pointsArray.append(point.toJson());
    }
    json["controlPoints"] = pointsArray;

    json["style"] = m_style.toJson();
    json["controlPointStyle"] = m_controlPointStyle.toJson();
    json["drawPolyline"] = m_drawPolyline;
    json["drawConvexHull"] = m_drawConvexHull;
    json["drawMainCurve"] = m_drawMainCurve;

    QJsonObject polylineStyle;
    polylineStyle["color"] = m_polylineColor.name();
    polylineStyle["width"] = m_polylineWidth;
    polylineStyle["style"] = static_cast<int>(m_polylineStyle);
    json["polylineStyle"] = polylineStyle;

    QJsonObject convexHullStyle;
    convexHullStyle["color"] = m_convexHullColor.name();
    convexHullStyle["width"] = m_convexHullWidth;
    convexHullStyle["style"] = static_cast<int>(m_convexHullStyle);
    json["convexHullStyle"] = convexHullStyle;

    return json;
}

bool Curve::deserializeCurveData(const QJsonObject& json) {
    if (!json.contains("controlPoints")) return false;

    QJsonArray pointsArray = json["controlPoints"].toArray();
    std::vector<Point> points;

    for (const auto& value : pointsArray) {
        Point point;
        if (point.fromJson(value.toObject())) {
            points.push_back(point);
        }
    }

    setControlPoints(points);

    if (json.contains("style")) {
        m_style.fromJson(json["style"].toObject());
    }

    if (json.contains("controlPointStyle")) {
        m_controlPointStyle.fromJson(json["controlPointStyle"].toObject());
    }

    m_drawPolyline = json["drawPolyline"].toBool(false);
    m_drawConvexHull = json["drawConvexHull"].toBool(false);
    m_drawMainCurve = json["drawMainCurve"].toBool(true);

    if (json.contains("polylineStyle")) {
        QJsonObject polylineStyle = json["polylineStyle"].toObject();
        m_polylineColor = QColor(polylineStyle["color"].toString());
        m_polylineWidth = polylineStyle["width"].toDouble(1.0);
        m_polylineStyle = static_cast<Qt::PenStyle>(polylineStyle["style"].toInt(Qt::DashLine));
    }

    if (json.contains("convexHullStyle")) {
        QJsonObject convexHullStyle = json["convexHullStyle"].toObject();
        m_convexHullColor = QColor(convexHullStyle["color"].toString());
        m_convexHullWidth = convexHullStyle["width"].toDouble(1.0);
        m_convexHullStyle = static_cast<Qt::PenStyle>(convexHullStyle["style"].toInt(Qt::DotLine));
    }

    return true;
}
