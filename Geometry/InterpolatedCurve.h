#pragma once

#include "Curve.h"
#include "../Math/IMathProvider.h"

class InterpolatedCurve final : public Curve {
public:
    enum class InterpolationType {
        NIFS3,
        OIFS3,
        Lagrange,
        Barycentric
    };

    enum class NodeDistribution {
        Equidistant,
        Chebyshev
    };

    explicit InterpolatedCurve(std::shared_ptr<IMathProvider> mathProvider, InterpolationType type);
    ~InterpolatedCurve() override = default;

    void setInterpolationType(InterpolationType type);
    InterpolationType getInterpolationType() const;

    Point getPointAt(double t) const override;

protected:
    std::unique_ptr<Curve> createClone() const override;
    void copySpecificPropertiesTo(Curve* target) const override;
    void onControlPointsChanged() override;

private:
    InterpolationType type;
    std::shared_ptr<IMathProvider> mathProvider;

    mutable std::vector<Eigen::Matrix<double, 4, 2>> coefficients;
    mutable bool needsRebuild{true};

    void buildNIFS3() const;
    void buildOIFS3() const;
    void buildLagrange() const;
    void buildBarycentric() const;
    void rebuildInterpolation() const;
};
