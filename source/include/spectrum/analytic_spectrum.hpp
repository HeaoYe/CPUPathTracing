#pragma once

#include "spectrum.hpp"
#include <functional>
#include <algorithm>

class AnalyticSpectrum : public Spectrum {
public:
    explicit AnalyticSpectrum(
        std::function<float(float)> expression,
        float lambda_min = g_lambda_min, float lambda_max = g_lambda_max
    ) : Spectrum(lambda_min, lambda_max), expression(std::move(expression)) {
        maximum = std::max(expression(lambda_min), expression(lambda_max));
        for (float lambda = lambda_min + 0.1; lambda < lambda_max; lambda += 0.1) {
            float value = expression(lambda);
            if (maximum < value) {
                maximum = value;
            }
        }
    }

    explicit AnalyticSpectrum(
        float maximum, std::function<float(float)> expression,
        float lambda_min = g_lambda_min, float lambda_max = g_lambda_max
    ) : Spectrum(lambda_min, lambda_max), expression(std::move(expression)) {
        this->maximum = maximum;
    }

    float operator[](float lambda) const {
        if (lambda < lambda_min || lambda > lambda_max) {
            return 0;
        }
        return expression(lambda);
    }
private:
    std::function<float(float)> expression;
};
