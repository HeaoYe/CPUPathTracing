#pragma once

#include "spectrum/spectrum.hpp"

class SigmoidPolynomialSpectrum : public Spectrum {
public:
    SigmoidPolynomialSpectrum(float c0, float c1, float c2) {
        this->c0() = c0;
        this->c1() = c1;
        this->c2() = c2;
    }

    float operator[](float lambda) const override {
        if (lambda < g_lambda_min || lambda > g_lambda_max) {
            return 0;
        }
        float u = (lambda - g_lambda_min) / (g_lambda_max - g_lambda_min);
        return sigmoid(((c2() * u) + c1()) * u + c0());
    }

    float max() const override {
        float left = (*this)[g_lambda_min];
        float right = (*this)[g_lambda_max];
        if (c2() >= 0) {
            return std::max(left, right);
        }
        float mid = - c1() / (2 * c2());
        if (mid <= g_lambda_min) {
            return left;
        }
        if (mid >= g_lambda_max) {
            return right;
        }
        return (*this)[mid];
    }

    float c0() const { return lambda_min; }
    float c1() const { return lambda_max; }
    float c2() const { return maximum; }

    float &c0() { return lambda_min; }
    float &c1() { return lambda_max; }
    float &c2() { return maximum; }
private:
    static float sigmoid(float x) {
        if (std::isinf(x)) {
            if (x > 0) {
                return 1;
            }
            return 0;
        }
        return 0.5f * x / std::sqrt(1 + x * x) + 0.5f;
    }
};
