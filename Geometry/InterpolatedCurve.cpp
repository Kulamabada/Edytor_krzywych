#include "InterpolatedCurve.h"
#include "Point.h"
#include "Vector.h"
#include "IMathProvider.h"
#include <Eigen/Dense>
#include <cmath>

// Definicja struktury wewnętrznych danych interpolacji
class InterpolatedCurve::InterpolationData {
public:
    // Dla cubic spline:
    std::vector<Eigen::Vector4d> splineCoefficients;

    // Dla Catmull-Rom i Cardinal:
    std::vector<Eigen::Matrix<double, 4, 2>> segmentMatrices;

    void clear() {
        splineCoefficients.clear();
        segmentMatrices.clear();
    }
};

InterpolatedCurve::InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider,
                                   InterpolationType type)
    : mathProvider(std::move(mathProvider)),
      type(type),
      data(std::make_unique<InterpolationData>()) {
    // Inicjalizacja krzywej interpolowanej
}

void InterpolatedCurve::addPoint(const Point& point) {
    points.push_back(point);
    needsRebuild = true;
}

void InterpolatedCurve::insertPoint(size_t index, const Point& point) {
    if (index <= points.size()) {
        points.insert(points.begin() + index, point);
        needsRebuild = true;
    }
}

void InterpolatedCurve::removePoint(size_t index) {
    if (index < points.size()) {
        points.erase(points.begin() + index);
        needsRebuild = true;
    }
}

void InterpolatedCurve::setPoints(const std::vector<Point>& newPoints) {
    points = newPoints;
    needsRebuild = true;
}

void InterpolatedCurve::movePoint(size_t index, const Point& newPosition) {
    if (index < points.size()) {
        points[index] = newPosition;
        needsRebuild = true;
    }
}

const std::vector<Point>& InterpolatedCurve::getPoints() const {
    return points;
}

size_t InterpolatedCurve::getPointCount() const {
    return points.size();
}

void InterpolatedCurve::setInterpolationType(InterpolationType newType) {
    if (type != newType) {
        type = newType;
        needsRebuild = true;
    }
}

InterpolatedCurve::InterpolationType InterpolatedCurve::getInterpolationType() const {
    return type;
}

void InterpolatedCurve::setTension(double newTension) {
    if (tension != newTension) {
        tension = newTension;
        needsRebuild = true;
    }
}

double InterpolatedCurve::getTension() const {
    return tension;
}

void InterpolatedCurve::rebuildInterpolation() {
    if (!needsRebuild) {
        return;
    }

    data->clear();

    if (points.size() < 2) {
        needsRebuild = false;
        return;
    }

    switch (type) {
        case InterpolationType::Linear:
            // Liniowa interpolacja nie wymaga budowania współczynników
            break;

        case InterpolationType::CubicSpline:
            buildCubicSpline();
            break;

        case InterpolationType::CatmullRom:
            buildCatmullRom();
            break;

        case InterpolationType::CardinalSpline:
            buildCardinalSpline();
            break;
    }

    needsRebuild = false;
}

void InterpolatedCurve::buildCubicSpline() {
    size_t n = points.size();
    if (n < 2) return;

    // Dla każdego wymiaru (x, y) rozwiązujemy oddzielny system
    for (int dim = 0; dim < 2; ++dim) {
        // Budujemy układ równań Ax = b dla drugich pochodnych
        Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
        Eigen::VectorXd b = Eigen::VectorXd::Zero(n);

        // Warunki brzegowe dla naturalnych spline'ów (druga pochodna na końcach = 0)
        A(0, 0) = 1.0;
        A(n-1, n-1) = 1.0;

        // Wypełnianie macierzy dla punktów wewnętrznych
        for (size_t i = 1; i < n - 1; ++i) {
            double h_i = 1.0;  // Równomierne próbkowanie dla uproszczenia
            double h_i1 = 1.0;

            A(i, i-1) = h_i;
            A(i, i) = 2 * (h_i + h_i1);
            A(i, i+1) = h_i1;

            double value_im1 = (dim == 0) ? points[i-1].x() : points[i-1].y();
            double value_i = (dim == 0) ? points[i].x() : points[i].y();
            double value_ip1 = (dim == 0) ? points[i+1].x() : points[i+1].y();

            b(i) = 6.0 * ((value_ip1 - value_i) / h_i1 - (value_i - value_im1) / h_i);
        }

        // Rozwiązanie układu równań dla drugich pochodnych
        Eigen::VectorXd secondDerivatives = A.fullPivLu().solve(b);

        // Obliczanie współczynników dla każdego segmentu krzywej
        for (size_t i = 0; i < n - 1; ++i) {
            double value_i = (dim == 0) ? points[i].x() : points[i].y();
            double value_ip1 = (dim == 0) ? points[i+1].x() : points[i+1].y();
            double h_i = 1.0;  // Równomierne próbkowanie

            if (dim == 0) {  // Dla wymiaru x
                Eigen::Vector4d coeffs;
                coeffs(0) = secondDerivatives(i) / (6.0 * h_i);  // a
                coeffs(1) = secondDerivatives(i+1) / (6.0 * h_i); // b
                coeffs(2) = value_i / h_i - secondDerivatives(i) * h_i / 6.0; // c
                coeffs(3) = value_ip1 / h_i - secondDerivatives(i+1) * h_i / 6.0; // d

                if (i < data->splineCoefficients.size()) {
                    data->splineCoefficients[i] = coeffs;
                } else {
                    data->splineCoefficients.push_back(coeffs);
                }
            } else {  // Dla wymiaru y - aktualizujemy istniejące współczynniki
                if (i < data->splineCoefficients.size()) {
                    Eigen::Vector4d& coeffs = data->splineCoefficients[i];
                    // Zapisujemy współczynniki y w drugiej połowie wektora
                    coeffs(0) = secondDerivatives(i) / (6.0 * h_i);
                    coeffs(1) = secondDerivatives(i+1) / (6.0 * h_i);
                    coeffs(2) = value_i / h_i - secondDerivatives(i) * h_i / 6.0;
                    coeffs(3) = value_ip1 / h_i - secondDerivatives(i+1) * h_i / 6.0;
                }
            }
        }
    }
}

void InterpolatedCurve::buildCatmullRom() {
    size_t n = points.size();
    if (n < 2) return;

    // Dla każdego segmentu krzywej obliczamy macierz Catmull-Rom
    for (size_t i = 0; i < n - 1; ++i) {
        // Dla Catmull-Rom potrzebujemy 4 punktów kontrolnych
        Point p0 = (i > 0) ? points[i-1] : points[i];  // Jeśli i=0, powtórz pierwszy punkt
        Point p1 = points[i];
        Point p2 = points[i+1];
        Point p3 = (i+1 < n-1) ? points[i+2] : points[i+1];  // Jeśli i=n-2, powtórz ostatni punkt

        // Macierz bazowa Catmull-Rom
        Eigen::Matrix4d basis;
        basis << 0, 1, 0, 0,
                -0.5, 0, 0.5, 0,
                1, -2.5, 2, -0.5,
                -0.5, 1.5, -1.5, 0.5;

        // Macierz punktów kontrolnych
        Eigen::Matrix<double, 4, 2> controlPoints;
        controlPoints << p0.x(), p0.y(),
                         p1.x(), p1.y(),
                         p2.x(), p2.y(),
                         p3.x(), p3.y();

        // Macierz segmentu = basis * controlPoints
        Eigen::Matrix<double, 4, 2> segmentMatrix = basis * controlPoints;

        // Zapisanie macierzy segmentu
        if (i < data->segmentMatrices.size()) {
            data->segmentMatrices[i] = segmentMatrix;
        } else {
            data->segmentMatrices.push_back(segmentMatrix);
        }
    }
}

void InterpolatedCurve::buildCardinalSpline() {
    size_t n = points.size();
    if (n < 2) return;

    // Dla każdego segmentu krzywej obliczamy macierz Cardinal
    for (size_t i = 0; i < n - 1; ++i) {
        // Dla Cardinal potrzebujemy 4 punktów kontrolnych
        Point p0 = (i > 0) ? points[i-1] : points[i];  // Jeśli i=0, powtórz pierwszy punkt
        Point p1 = points[i];
        Point p2 = points[i+1];
        Point p3 = (i+1 < n-1) ? points[i+2] : points[i+1];  // Jeśli i=n-2, powtórz ostatni punkt

        // Macierz bazowa Cardinal, gdzie s = (1-tension)/2
        double s = (1.0 - tension) / 2.0;
        Eigen::Matrix4d basis;
        basis << 0, 1, 0, 0,
                -s, 0, s, 0,
                2*s, s-3, 3-2*s, -s,
                -s, 2-s, s-2, s;

        // Macierz punktów kontrolnych
        Eigen::Matrix<double, 4, 2> controlPoints;
        controlPoints << p0.x(), p0.y(),
                         p1.x(), p1.y(),
                         p2.x(), p2.y(),
                         p3.x(), p3.y();

        // Macierz segmentu = basis * controlPoints
        Eigen::Matrix<double, 4, 2> segmentMatrix = basis * controlPoints;

        // Zapisanie macierzy segmentu
        if (i < data->segmentMatrices.size()) {
            data->segmentMatrices[i] = segmentMatrix;
        } else {
            data->segmentMatrices.push_back(segmentMatrix);
        }
    }
}

Point InterpolatedCurve::getPointAt(double t) const {
    if (points.empty()) {
        return Point(0.0, 0.0);
    }

    if (points.size() == 1) {
        return points[0];
    }

    // Upewnij się, że mamy zaktualizowane dane interpolacji
    if (needsRebuild) {
        const_cast<InterpolatedCurve*>(this)->rebuildInterpolation();
    }

    // Ograniczenie parametru t do zakresu [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Liczba segmentów
    size_t numSegments = points.size() - 1;

    // Obliczenie indeksu segmentu i lokalnego parametru
    double segmentLength = 1.0 / numSegments;
    size_t segmentIndex = std::min(static_cast<size_t>(t / segmentLength), numSegments - 1);
    double localT = (t - segmentIndex * segmentLength) / segmentLength;

    switch (type) {
        case InterpolationType::Linear: {
            // Interpolacja liniowa
            double oneMinusT = 1.0 - localT;
            double x = oneMinusT * points[segmentIndex].x() + localT * points[segmentIndex + 1].x();
            double y = oneMinusT * points[segmentIndex].y() + localT * points[segmentIndex + 1].y();
            double z = oneMinusT * poi
