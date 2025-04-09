#include "ParametricCurve.h"
#include "Point.h"
#include "Vector"
#include "IMathProvider.h"
#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

ParametricCurve::ParametricCurve(std::shared_ptr<IMathProvider> mathProvider)
    : mathProvider(std::move(mathProvider)) {
    // Domyślna inicjalizacja funkcji parametrycznych - okrąg jednostkowy
    xFunction = [](double t) { return std::cos(2 * M_PI * t); };
    yFunction = [](double t) { return std::sin(2 * M_PI * t); };
}

void ParametricCurve::setXFunction(ParametricFunction xFunc) {
    if (xFunc) {
        xFunction = std::move(xFunc);
    }
}

void ParametricCurve::setYFunction(ParametricFunction yFunc) {
    if (yFunc) {
        yFunction = std::move(yFunc);
    }
}

bool ParametricCurve::setXFunctionFromExpression(const std::string& expression) {
    try {
        // W rzeczywistej implementacji należałoby użyć parsera wyrażeń matematycznych
        // Poniżej przykładowa implementacja, która w praktyce powinna zostać zastąpiona
        // faktycznym parsowaniem i kompilacją wyrażenia

        // Tymczasowa prosta implementacja dla kilku podstawowych funkcji
        if (expression == "sin(t)") {
            xFunction = [](double t) { return std::sin(t); };
            return true;
        } else if (expression == "cos(t)") {
            xFunction = [](double t) { return std::cos(t); };
            return true;
        } else if (expression == "t") {
            xFunction = [](double t) { return t; };
            return true;
        } else if (expression == "t^2" || expression == "t*t") {
            xFunction = [](double t) { return t * t; };
            return true;
        }

        // Domyślnie - nieudane parsowanie
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

bool ParametricCurve::setYFunctionFromExpression(const std::string& expression) {
    try {
        // W rzeczywistej implementacji należałoby użyć parsera wyrażeń matematycznych
        // Poniżej przykładowa implementacja, która w praktyce powinna zostać zastąpiona
        // faktycznym parsowaniem i kompilacją wyrażenia

        // Tymczasowa prosta implementacja dla kilku podstawowych funkcji
        if (expression == "sin(t)") {
            yFunction = [](double t) { return std::sin(t); };
            return true;
        } else if (expression == "cos(t)") {
            yFunction = [](double t) { return std::cos(t); };
            return true;
        } else if (expression == "t") {
            yFunction = [](double t) { return t; };
            return true;
        } else if (expression == "t^2" || expression == "t*t") {
            yFunction = [](double t) { return t * t; };
            return true;
        }

        // Domyślnie - nieudane parsowanie
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

void ParametricCurve::setParameterRange(double newTMin, double newTMax) {
    if (newTMin >= newTMax) {
        throw std::invalid_argument("tMin musi być mniejsze niż tMax");
    }

    tMin = newTMin;
    tMax = newTMax;
}

std::pair<double, double> ParametricCurve::getParameterRange() const {
    return std::make_pair(tMin, tMax);
}

Point ParametricCurve::getPointAt(double t) const {
    // Mapowanie parametru t z przedziału [0,1] do [tMin,tMax]
    double mappedT = tMin + t * (tMax - tMin);

    // Obliczenie współrzędnych punktu
    double x = xFunction(mappedT);
    double y = yFunction(mappedT);

    return Point(x, y, 0.0);
}

Vector ParametricCurve::getTangentAt(double t) const {
    // Numeryczne obliczenie pochodnej
    const double epsilon = 1e-6;

    // Mapowanie parametrów z przedziału [0,1] do [tMin,tMax]
    double mappedT1 = tMin + t * (tMax - tMin);
    double mappedT2 = tMin + (t + epsilon) * (tMax - tMin);

    // Obliczenie pochodnych jako ilorazów różnicowych
    double x1 = xFunction(mappedT1);
    double y1 = yFunction(mappedT1);

    double x2 = xFunction(mappedT2);
    double y2 = yFunction(mappedT2);

    double dx = (x2 - x1) / (mappedT2 - mappedT1);
    double dy = (y2 - y1) / (mappedT2 - mappedT1);

    return Vector(dx, dy, 0.0);
}

double ParametricCurve::getCurvatureAt(double t) const {
    // Obliczenie krzywizny na podstawie pierwszej i drugiej pochodnej
    const double h = 1e-6;
    double mappedT = tMin + t * (tMax - tMin);

    // Obliczenie wartości funkcji w trzech punktach
    double x = xFunction(mappedT);
    double y = yFunction(mappedT);

    double xh = xFunction(mappedT + h);
    double yh = yFunction(mappedT + h);

    double x2h = xFunction(mappedT + 2*h);
    double y2h = yFunction(mappedT + 2*h);

    // Obliczenie pierwszych pochodnych
    double dxdt = (xh - x) / h;
    double dydt = (yh - y) / h;

    // Obliczenie drugich pochodnych
    double d2xdt2 = (x2h - 2*xh + x) / (h*h);
    double d2ydt2 = (y2h - 2*yh + y) / (h*h);

    // Obliczenie krzywizny w przestrzeni 2D
    // Formuła: κ = |x'y'' - y'x''| / (x'^2 + y'^2)^(3/2)
    double numerator = std::abs(dxdt * d2ydt2 - dydt * d2xdt2);
    double denominator = std::pow(dxdt * dxdt + dydt * dydt, 1.5);

    if (denominator < 1e-10) {
        return 0.0;  // Unikamy dzielenia przez zero
    }

    return numerator / denominator;
}
