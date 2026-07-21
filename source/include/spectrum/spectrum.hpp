#pragma once

#include "wavelength.hpp"

class Spectrum {
public:
    Spectrum() : lambda_min(g_lambda_min), lambda_max(g_lambda_max) {}

    explicit Spectrum(float lambda_min, float lambda_max) : lambda_min(lambda_min), lambda_max(lambda_max) {}

    virtual float operator[](float lambda) const = 0;

    virtual float max() const { return maximum; }
protected:
    float lambda_min {}, lambda_max {};
    float maximum {};
};
