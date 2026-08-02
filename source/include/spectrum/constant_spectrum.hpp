#pragma once

#include "spectrum.hpp"

class ConstantSpectrum : public Spectrum {
public:
    explicit ConstantSpectrum(float constant, float lambda_min = g_lambda_min, float lambda_max = g_lambda_max)
        : Spectrum(lambda_min, lambda_max) {
        maximum = constant;
    }

    float operator[](float lambda) const override {
        if (lambda < lambda_min || lambda > lambda_max) {
            return 0;
        }
        return maximum;
    }

    bool isConstant() const override { return true; }
};
