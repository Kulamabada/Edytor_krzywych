#include "InterpolatedCurve.h"
#include "Point.h"
#include "../Math/IMathProvider.h"
#include <cmath>
#include <limits>
#include <algorithm>

InterpolatedCurve::InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider,
                                    InterpolationType type)
    : type(type),
      mathProvider(std::move(mathProvider)),
      needsRebuild(true) {
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

void InterpolatedCurve::rebuildInterpolation() const {
    if (!needsRebuild || points.size() < 2) {
        needsRebuild = false;
        return;
    }

    coefficients.clear();

    switch (type) {
        case InterpolationType::CubicSpline:
            buildCubicSpline();
            break;
        case InterpolationType::CatmullRom:
            buildCatmullRom();
            break;
        case InterpolationType::Linear:
            // Dla interpolacji liniowej nie potrzebujemy współczynników
            break;
    }

    needsRebuild = false;
}

void InterpolatedCurve::buildCubicSpline() const {
    const size_t n = points.size();
    if (n < 2) return;

    // Utwórz trójdiagonalną macierz dla obliczania współczynników spline
    Eigen::MatrixXd A = Eigen::MatrixXd::Zero(n, n);
    Eigen::VectorXd bx = Eigen::VectorXd::Zero(n);
    Eigen::VectorXd by = Eigen::VectorXd::Zero(n);

    // Naturalne warunki brzegowe
    A(0, 0) = 1.0;
    A(n-1, n-1) = 1.0;

    // Wypełnianie macierzy dla punktów wewnętrznych
    for (size_t i = 1; i < n - 1; ++i) {
        A(i, i-1) = 1.0;
        A(i, i) = 4.0;
        A(i, i+1) = 1.0;

        bx(i) = 3.0 * (points[i+1].x() - points[i-1].x());
        by(i) = 3.0 * (points[i+1].y() - points[i-1].y());
    }

    // Rozwiązanie układu równań dla pochodnych
    Eigen::VectorXd cx = A.colPivHouseholderQr().solve(bx);
    Eigen::VectorXd cy = A.colPivHouseholderQr().solve(by);

    // Obliczenie współczynników dla każdego segmentu
    for (size_t i = 0; i < n - 1; ++i) {
        Eigen::Matrix<double, 4, 2> coef;

        // Współczynniki dla x
        coef(0, 0) = points[i].x();
        coef(1, 0) = cx(i);
        coef(2, 0) = 3.0 * (points[i+1].x() - points[i].x()) - 2.0 * cx(i) - cx(i+1);
        coef(3, 0) = 2.0 * (points[i].x() - points[i+1].x()) + cx(i) + cx(i+1);

        // Współczynniki dla y
        coef(0, 1) = points[i].y();
        coef(1, 1) = cy(i);
        coef(2, 1) = 3.0 * (points[i+1].y() - points[i].y()) - 2.0 * cy(i) - cy(i+1);
        coef(3, 1) = 2.0 * (points[i].y() - points[i+1].y()) + cy(i) + cy(i+1);

        coefficients.push_back(coef);
    }
}

void InterpolatedCurve::buildCatmullRom() const {
    if (points.size() < 2) return;

    // Macierz bazowa Catmull-Rom
    Eigen::Matrix4d basis;
    basis << 0, 1, 0, 0,
             -0.5, 0, 0.5, 0,
             1, -2.5, 2, -0.5,
             -0.5, 1.5, -1.5, 0.5;

    // Dla każdego segmentu
    for (size_t i = 0; i < points.size() - 1; ++i) {
        // Punkty kontrolne z ekstrapolacją na krańcach
        Point p0 = (i > 0) ? points[i-1] : points[i];
        Point p1 = points[i];
        Point p2 = points[i+1];
        Point p3 = (i+1 < points.size()-1) ? points[i+2] : points[i+1];

        // Macierz punktów kontrolnych
        Eigen::Matrix<double, 4, 2> controls;
        controls << p0.x(), p0.y(),
                   p1.x(), p1.y(),
                   p2.x(), p2.y(),
                   p3.x(), p3.y();

        // Obliczenie współczynników
        coefficients.push_back(basis * controls);
    }
}

Point InterpolatedCurve::getPointAt(double t) const {
    if (points.empty()) return Point(0.0, 0.0, 0.0);
    if (points.size() == 1) return points[0];

    // Aktualizacja danych interpolacji jeśli potrzebna
    if (needsRebuild) {
        rebuildInterpolation();
    }

    // Ograniczenie parametru t do [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Obliczenie segmentu i lokalnego parametru
    size_t numSegments = points.size() - 1;
    double segmentLength = 1.0 / numSegments;
    size_t segmentIndex = std::min(static_cast<size_t>(t / segmentLength), numSegments - 1);
    double localT = (t - segmentIndex * segmentLength) / segmentLength;

    if (type == InterpolationType::Linear) {
        // Prosta interpolacja liniowa
        double oneMinusT = 1.0 - localT;
        double x = oneMinusT * points[segmentIndex].x() + localT * points[segmentIndex + 1].x();
        double y = oneMinusT * points[segmentIndex].y() + localT * points[segmentIndex + 1].y();
        double z = oneMinusT * points[segmentIndex].z() + localT * points[segmentIndex + 1].z();
        return Point(x, y, z);
    } else {
        // Interpolacja krzywoliniowa
        if (segmentIndex >= coefficients.size()) {
            return points[segmentIndex]; // Awaryjnie
        }

        const auto& coef = coefficients[segmentIndex];
        Eigen::Vector4d tVec;
        tVec << 1.0, localT, localT*localT, localT*localT*localT;

        double x = tVec.dot(coef.col(0));
        double y = tVec.dot(coef.col(1));

        return Point(x, y, 0.0);
    }
}

Vector InterpolatedCurve::getTangentAt(double t) const {
    // Numeryczne obliczenie pochodnej
    const double epsilon = 1e-6;
    Point p1 = getPointAt(t);
    Point p2 = getPointAt(t + epsilon);

    return Vector(
        (p2.x() - p1.x()) / epsilon,
        (p2.y() - p1.y()) / epsilon,
        (p2.z() - p1.z()) / epsilon
    );
}

double InterpolatedCurve::getCurvatureAt(double t) const {
    // Obliczenie krzywizny przez przybliżenie drugiej pochodnej
    const double h = 1e-6;
    Vector first = getTangentAt(t);
    Vector second = (getTangentAt(t + h) - first) / h;

    double numerator = std::abs(first.x() * second.y() - first.y() * second.x());
    double denominator = std::pow(first.x() * first.x() + first.y() * first.y(), 1.5);

    return (denominator < 1e-10) ? 0.0 : (numerator / denominator);
}

bool InterpolatedCurve::isPointOnCurve(const Point& point, double tolerance) const {
    // Sprawdzenie przez próbkowanie krzywej
    const int steps = 50;
    for (int i = 0; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        if (getPointAt(t).distanceTo(point) <= tolerance) {
            return true;
        }
    }
    return false;
}

double InterpolatedCurve::getLength() const {
    // Przybliżenie długości przez sumowanie odcinków
    const int steps = 20;
    double length = 0.0;
    Point prev = getPointAt(0.0);

    for (int i = 1; i <= steps; ++i) {
        double t = static_cast<double>(i) / steps;
        Point curr = getPointAt(t);
        length += prev.distanceTo(curr);
        prev = curr;
    }

    return length;
}

std::vector<Point> InterpolatedCurve::getIntersectionsWith(const Curve& other) const {
    // Ulepszona implementacja - adaptacyjna metoda znajdowania przecięć
    std::vector<Point> result;

    // Najpierw sprawdzamy otoczki wypukłe obu krzywych
    // Jeśli się nie przecinają, możemy szybko wrócić

    // Adaptacyjny podział przestrzeni dla efektywnego znajdowania przecięć
    const int initialSteps = 20;
    const double initialTolerance = 1e-3;
    const double finalTolerance = 1e-6;
    const int maxRecursionDepth = 8;

    std::function<void(double, double, double, double, int)> findIntersectionsRecursive =
        [&](double t1Start, double t1End, double t2Start, double t2End, int depth) {
            Point p1Start = this->getPointAt(t1Start);
            Point p1End = this->getPointAt(t1End);
            Point p2Start = other.getPointAt(t2Start);
            Point p2End = other.getPointAt(t2End);

            // Sprawdzenie, czy prostokąty ograniczające mogą się przecinać
            double minX1 = std::min(p1Start.x(), p1End.x());
            double maxX1 = std::max(p1Start.x(), p1End.x());
            double minY1 = std::min(p1Start.y(), p1End.y());
            double maxY1 = std::max(p1Start.y(), p1End.y());

            double minX2 = std::min(p2Start.x(), p2End.x());
            double maxX2 = std::max(p2Start.x(), p2End.x());
            double minY2 = std::min(p2Start.y(), p2End.y());
            double maxY2 = std::max(p2Start.y(), p2End.y());

            // Jeśli prostokąty się nie przecinają, kończymy rekurencję
            if (maxX1 < minX2 || minX1 > maxX2 || maxY1 < minY2 || minY1 > maxY2) {
                return;
            }

            // Sprawdzenie punktu środkowego
            double t1Mid = (t1Start + t1End) / 2.0;
            double t2Mid = (t2Start + t2End) / 2.0;
            Point p1Mid = this->getPointAt(t1Mid);
            Point p2Mid = other.getPointAt(t2Mid);

            double distance = p1Mid.distanceTo(p2Mid);

            // Jeśli znaleźliśmy przecięcie lub osiągnęliśmy maksymalną głębokość rekurencji
            if (distance < finalTolerance || depth >= maxRecursionDepth) {
                // Sprawdź, czy ten punkt nie został już dodany
                bool exists = false;
                for (const auto& p : result) {
                    if (p1Mid.distanceTo(p) < finalTolerance) {
                        exists = true;
                        break;
                    }
                }

                if (!exists && distance < initialTolerance) {
                    // Dodajemy średni punkt jako przybliżenie przecięcia
                    Point intersection((p1Mid.x() + p2Mid.x()) / 2.0,
                                      (p1Mid.y() + p2Mid.y()) / 2.0,
                                      (p1Mid.z() + p2Mid.z()) / 2.0);
                    result.push_back(intersection);
                }
                return;
            }

            // Kontynuujemy rekurencyjne przeszukiwanie
            findIntersectionsRecursive(t1Start, t1Mid, t2Start, t2Mid, depth + 1);
            findIntersectionsRecursive(t1Start, t1Mid, t2Mid, t2End, depth + 1);
            findIntersectionsRecursive(t1Mid, t1End, t2Start, t2Mid, depth + 1);
            findIntersectionsRecursive(t1Mid, t1End, t2Mid, t2End, depth + 1);
        };

    // Uruchomienie rekurencyjnego wyszukiwania
    findIntersectionsRecursive(0.0, 1.0, 0.0, 1.0, 0);

    return result;
}

void InterpolatedCurve::transform(const TransformMatrix& matrix) {
    for (auto& point : points) {
        point.transform(matrix);
    }
    needsRebuild = true;
}

std::unique_ptr<Curve> InterpolatedCurve::clone() const {
    auto copy = std::make_unique<InterpolatedCurve>(mathProvider, type);
    copy->setPoints(points);
    return copy;
}

Vector InterpolatedCurve::getPosition() const {
    if (points.empty()) return Vector::Zero();

    Vector sum = Vector::Zero();
    for (const auto& p : points) {
        sum += Vector(p.x(), p.y(), p.z());
    }
    return sum / static_cast<double>(points.size());
}

void InterpolatedCurve::setPosition(const Eigen::Vector3d& position) {
    if (points.empty()) return;

    Vector delta = position - getPosition();
    for (auto& p : points) {
        p.setX(p.x() + delta.x());
        p.setY(p.y() + delta.y());
        p.setZ(p.z() + delta.z());
    }
    needsRebuild = true;
}

Vector InterpolatedCurve::getScale() const {
    return Vector::Ones();
}

void InterpolatedCurve::setScale(const Vector& scale) {
    if (points.empty()) return;

    Vector center = getPosition();
    for (auto& p : points) {
        Vector relPos(p.x() - center.x(), p.y() - center.y(), p.z() - center.z());
        p.setX(center.x() + relPos.x() * scale.x());
        p.setY(center.y() + relPos.y() * scale.y());
        p.setZ(center.z() + relPos.z() * scale.z());
    }
    needsRebuild = true;
}

Eigen::Quaterniond InterpolatedCurve::getRotation() const {
    return Eigen::Quaterniond::Identity();
}

void InterpolatedCurve::setRotation(const Eigen::Quaterniond& rotation) {
    if (points.empty()) return;

    Vector center = getPosition();
    for (auto& p : points) {
        Vector relPos(p.x() - center.x(), p.y() - center.y(), p.z() - center.z());
        Vector rotated = rotation * relPos;
        p.setX(center.x() + rotated.x());
        p.setY(center.y() + rotated.y());
        p.setZ(center.z() + rotated.z());
    }
    needsRebuild = true;
}

// Nowe metody dla zaawansowanych operacji na krzywych

int InterpolatedCurve::findNearestControlPoint(const Point& point, double* outDistance) const {
    if (points.empty()) {
        if (outDistance) *outDistance = std::numeric_limits<double>::max();
        return -1;
    }

    int nearestIdx = 0;
    double minDistance = points[0].distanceTo(point);

    for (size_t i = 1; i < points.size(); ++i) {
        double dist = points[i].distanceTo(point);
        if (dist < minDistance) {
            minDistance = dist;
            nearestIdx = static_cast<int>(i);
        }
    }

    if (outDistance) *outDistance = minDistance;
    return nearestIdx;
}

std::vector<double> InterpolatedCurve::generateEvenlySpacedParameters(int count) const {
    std::vector<double> parameters;
    if (count <= 0) return parameters;

    for (int i = 0; i < count; ++i) {
        double t = static_cast<double>(i) / (count - 1);
        parameters.push_back(t);
    }

    return parameters;
}

std::vector<Point> InterpolatedCurve::samplePoints(int count) const {
    std::vector<Point> sampledPoints;
    if (count <= 0 || points.empty()) return sampledPoints;

    auto params = generateEvenlySpacedParameters(count);
    for (double t : params) {
        sampledPoints.push_back(getPointAt(t));
    }

    return sampledPoints;
}

void InterpolatedCurve::simplify(double tolerance) {
    if (points.size() <= 2) return; // Nie można uprościć krzywej z mniej niż 3 punktami

    std::vector<Point> result;
    result.push_back(points.front());

    // Algorytm Douglas-Peucker do upraszczania krzywych
    std::function<void(size_t, size_t)> simplifySegment = [&](size_t start, size_t end) {
        if (end <= start + 1) return;

        double maxDistance = 0.0;
        size_t maxIndex = start;

        // Znajdź punkt najbardziej oddalony od linii prostej
        Point startPoint = points[start];
        Point endPoint = points[end];

        for (size_t i = start + 1; i < end; ++i) {
            double distance = distancePointToLine(points[i], startPoint, endPoint);
            if (distance > maxDistance) {
                maxDistance = distance;
                maxIndex = i;
            }
        }

        // Jeśli najdalszy punkt jest dalej niż tolerancja, zachowaj go i rekurencyjnie kontynuuj
        if (maxDistance > tolerance) {
            simplifySegment(start, maxIndex);
            result.push_back(points[maxIndex]);
            simplifySegment(maxIndex, end);
        }
    };

    // Uruchomienie rekurencyjnej procedury upraszczania
    simplifySegment(0, points.size() - 1);
    result.push_back(points.back());

    // Zastąp punkty kontrolne nowymi, uproszczonymi punktami
    points = result;
    needsRebuild = true;
}

double InterpolatedCurve::distancePointToLine(const Point& point, const Point& lineStart, const Point& lineEnd) const {
    double lineLength = lineStart.distanceTo(lineEnd);
    if (lineLength < 1e-10) return point.distanceTo(lineStart);

    // Obliczenie wektora kierunkowego linii
    Vector lineDirection(
        lineEnd.x() - lineStart.x(),
        lineEnd.y() - lineStart.y(),
        lineEnd.z() - lineStart.z()
    );
    lineDirection.normalize();

    // Wektor od punktu początkowego linii do badanego punktu
    Vector pointVector(
        point.x() - lineStart.x(),
        point.y() - lineStart.y(),
        point.z() - lineStart.z()
    );

    // Obliczenie rzutu wektora pointVector na kierunek linii
    double projection = pointVector.dot(lineDirection);

    // Jeśli projekcja jest ujemna, najbliższy jest punkt początkowy
    if (projection < 0) {
        return point.distanceTo(lineStart);
    }
    // Jeśli projekcja jest większa niż długość linii, najbliższy jest punkt końcowy
    else if (projection > lineLength) {
        return point.distanceTo(lineEnd);
    }
    // W przeciwnym razie oblicz odległość od prostej
    else {
        Vector projectionPoint(
            lineStart.x() + projection * lineDirection.x(),
            lineStart.y() + projection * lineDirection.y(),
            lineStart.z() + projection * lineDirection.z()
        );

        return std::sqrt(
            std::pow(point.x() - projectionPoint.x(), 2) +
            std::pow(point.y() - projectionPoint.y(), 2) +
            std::pow(point.z() - projectionPoint.z(), 2)
        );
    }
}

void InterpolatedCurve::reparameterize() {
    if (points.size() <= 1) return;

    // Oblicz długości łuków
    std::vector<double> arcLengths;
    arcLengths.push_back(0.0);

    double totalLength = 0.0;
    const int samples = 100;

    Point prevPoint = points[0];
    for (int i = 1; i <= samples; ++i) {
        double t = static_cast<double>(i) / samples;
        Point currPoint = getPointAt(t);
        totalLength += prevPoint.distanceTo(currPoint);
        arcLengths.push_back(totalLength);
        prevPoint = currPoint;
    }

    // Normalizacja długości łuków
    for (auto& length : arcLengths) {
        length /= totalLength;
    }

    // Nowe punkty kontrolne
    std::vector<Point> newPoints;
    newPoints.push_back(points.front());

    // Równomierne próbkowanie po długości łuku, pomijając pierwszy i ostatni punkt
    for (size_t i = 1; i < points.size() - 1; ++i) {
        double targetArcLength = static_cast<double>(i) / (points.size() - 1);

        // Znajdź indeks w tablicy arcLengths, gdzie targetArcLength znajduje się między wartościami
        auto it = std::lower_bound(arcLengths.begin(), arcLengths.end(), targetArcLength);
        size_t idx = std::distance(arcLengths.begin(), it);

        // Interpolacja liniowa dla znalezienia odpowiedniego parametru t
        if (idx > 0 && idx < arcLengths.size()) {
            double t1 = static_cast<double>(idx - 1) / samples;
            double t2 = static_cast<double>(idx) / samples;
            double a1 = arcLengths[idx - 1];
            double a2 = arcLengths[idx];

            // Interpolacja liniowa parametru t na podstawie długości łuku
            double t = t1 + (targetArcLength - a1) * (t2 - t1) / (a2 - a1);
            newPoints.push_back(getPointAt(t));
        }
    }

    newPoints.push_back(points.back());

    // Zastąp punkty kontrolne nowymi
    points = newPoints;
    needsRebuild = true;
}
