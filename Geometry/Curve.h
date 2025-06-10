#pragma once

#include "GeometricObject.h"
#include "CurveStyle.h"
#include <vector>
#include "Point.h"

enum class CurveType {
    Bezier,
    Interpolated,
    Generic
};

struct DefaultCurveConfig {
    double lineWidth = 2.0;
    QColor color = Qt::blue;
    Qt::PenStyle lineStyle = Qt::SolidLine;
    bool showControlPoints = true;
    double controlPointSize = 5.0;
    QColor controlPointNormalColor = Qt::blue;
    QColor controlPointSelectedColor = Qt::red;
    QColor controlPointActiveColor = Qt::green;

    bool drawPolyline = false;
    bool drawConvexHull = false;
    QColor polylineColor = Qt::darkGreen;
    double polylineWidth = 1.0;
    Qt::PenStyle polylineStyle = Qt::DashLine;

    QColor convexHullColor = Qt::red;
    double convexHullWidth = 1.0;
    Qt::PenStyle convexHullStyle = Qt::DotLine;
};

struct CurvePrecisionConfig {
    int lengthSamples = 1000;
    double tangentEpsilon = 1e-6;
    int renderSamples = 1000;
};

class Curve : public GeometricObject {
public:
    explicit Curve(CurveType type = CurveType::Generic);
    explicit Curve(const DefaultCurveConfig& config);
    ~Curve() override = default;

    static DefaultCurveConfig createDefaultConfig(CurveType type);

    CurveType getType() const { return m_type; }
    bool isType(CurveType type) const { return m_type == type; }

    virtual double getLength() const;
    virtual Point getPointAt(double t) const = 0;
    std::unique_ptr<Curve> clone() const;

    virtual bool hasControlPoints() const { return true; }
    virtual void addControlPoint(const Point& point);
    virtual void insertControlPoint(size_t index, const Point& point);
    virtual void removeControlPoint(size_t index);
    virtual void setControlPoints(const std::vector<Point>& points);
    virtual void moveControlPoint(size_t index, const Point& newPosition);
    virtual const std::vector<Point>& getControlPoints() const;
    virtual size_t getControlPointCount() const;

    Vector getPosition() const override;
    void setPosition(const Eigen::Vector3d& position) override;
    void transform(const TransformMatrix& matrix) override;

    const CurveStyle& getStyle() const { return m_style; }
    void setStyle(const CurveStyle& style) { m_style = style; }

    void setColor(const QColor& color) { m_style.color = color; }
    QColor getColor() const { return m_style.color; }

    void setLineWidth(double width) { m_style.lineWidth = width; }
    double getLineWidth() const { return m_style.lineWidth; }

    void setLineStyle(Qt::PenStyle style) { m_style.lineStyle = style; }
    Qt::PenStyle getLineStyle() const { return m_style.lineStyle; }

    const ControlPointStyle& getControlPointStyle() const { return m_controlPointStyle; }
    void setControlPointStyle(const ControlPointStyle& style) { m_controlPointStyle = style; }

    void setShowControlPoints(bool show) { m_controlPointStyle.show = show; }
    bool getShowControlPoints() const { return m_controlPointStyle.show; }

    void setControlPointNormalColor(const QColor& color) { m_controlPointStyle.normalColor = color; }
    QColor getControlPointNormalColor() const { return m_controlPointStyle.normalColor; }

    void setControlPointSelectedColor(const QColor& color) { m_controlPointStyle.selectedColor = color; }
    QColor getControlPointSelectedColor() const { return m_controlPointStyle.selectedColor; }

    void setControlPointActiveColor(const QColor& color) { m_controlPointStyle.activeColor = color; }
    QColor getControlPointActiveColor() const { return m_controlPointStyle.activeColor; }

    void setControlPointSize(double size) { m_controlPointStyle.size = size; }
    double getControlPointSize() const { return m_controlPointStyle.size; }

    bool getDrawPolyline() const { return m_drawPolyline; }
    void setDrawPolyline(bool draw) { m_drawPolyline = draw; }

    bool getDrawConvexHull() const { return m_drawConvexHull; }
    void setDrawConvexHull(bool draw) { m_drawConvexHull = draw; }

    QColor getPolylineColor() const { return m_polylineColor; }
    void setPolylineColor(const QColor& color) { m_polylineColor = color; }

    double getPolylineWidth() const { return m_polylineWidth; }
    void setPolylineWidth(double width) { m_polylineWidth = width; }

    Qt::PenStyle getPolylineStyle() const { return m_polylineStyle; }
    void setPolylineStyle(Qt::PenStyle style) { m_polylineStyle = style; }

    QColor getConvexHullColor() const { return m_convexHullColor; }
    void setConvexHullColor(const QColor& color) { m_convexHullColor = color; }

    double getConvexHullWidth() const { return m_convexHullWidth; }
    void setConvexHullWidth(double width) { m_convexHullWidth = width; }

    Qt::PenStyle getConvexHullStyle() const { return m_convexHullStyle; }
    void setConvexHullStyle(Qt::PenStyle style) { m_convexHullStyle = style; }

    virtual bool getDrawMainCurve() const { return m_drawMainCurve; }
    virtual void setDrawMainCurve(bool draw) { m_drawMainCurve = draw; }

    const CurvePrecisionConfig& getPrecisionConfig() const { return m_precisionConfig; }
    void setPrecisionConfig(const CurvePrecisionConfig& config) { m_precisionConfig = config; }

    virtual int getDefaultLengthSamples() const { return m_precisionConfig.lengthSamples; }
    virtual double getDefaultTangentEpsilon() const { return m_precisionConfig.tangentEpsilon; }
    virtual int getDefaultRenderSamples() const { return m_precisionConfig.renderSamples; }
    Point interpolateLinear(const Point& p1, const Point& p2, double t) const;

protected:
    virtual std::unique_ptr<Curve> createClone() const = 0;
    virtual void copySpecificPropertiesTo(Curve* target) const {}

    void copyStylesTo(Curve* target) const;
    virtual void onControlPointsChanged() {}

    double calculateLengthByDiscretization(int numSamples = -1) const;
    bool validateParameterRange(double& t) const;
    double calculateParameterForSegment(size_t segmentIndex, size_t totalSegments, double localT) const;

    std::vector<Point> m_controlPoints;
    CurveStyle m_style;
    ControlPointStyle m_controlPointStyle;

    bool m_drawPolyline{false};
    bool m_drawConvexHull{false};
    bool m_drawMainCurve{true};

    QColor m_polylineColor{Qt::darkGreen};
    double m_polylineWidth{1.0};
    Qt::PenStyle m_polylineStyle{Qt::DashLine};

    QColor m_convexHullColor{Qt::red};
    double m_convexHullWidth{1.0};
    Qt::PenStyle m_convexHullStyle{Qt::DotLine};

    CurvePrecisionConfig m_precisionConfig;

private:
    void initializeFromConfig(const DefaultCurveConfig& config);
    void initializePrecisionConfig(CurveType type);

    CurveType m_type;
};
