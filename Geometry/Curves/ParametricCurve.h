#pragma once

#include "Curve.h"
#include <functional>
#include <memory>
#include <string>

class IMathProvider;

class ParametricCurve : public Curve {
public:
    using ParametricFunction = std::function<double(double)>;
    
    explicit ParametricCurve(std::shared_ptr<IMathProvider> mathProvider);
    ~ParametricCurve() override = default;

    // Funkcje parametryczne
    void setXFunction(ParametricFunction xFunc);
    void setYFunction(ParametricFunction yFunc);
    
    // Ustawienie funkcji z wyrażeń tekstowych
    bool setXFunctionFromExpression(const std::string& expression);
    bool setYFunctionFromExpression(const std::string& expression);
    
    // Zakres parametru t
    void setParameterRange(double tMin, double tMax);
    std::pair<double, double> getParameterRange() const;

    // Implementacja metod z klasy bazowej Curve
    Point getPointAt(double t) const override;
    Vector getTangentAt(double t) const override;
    double getCurvatureAt(double t) const override;

private:
    ParametricFunction xFunction;
    ParametricFunction yFunction;
    double tMin = 0.0;
    double tMax = 1.0;
    std::shared_ptr<IMathProvider> mathProvider;
};
