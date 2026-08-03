#pragma once

#include "wavelength.hpp"

class SpectrumSamples {
public:
    SpectrumSamples() = default;

    explicit SpectrumSamples(float value) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] = value;
        }
    }

    float max() const {
        float maximum = data[0];
        for (size_t i = 1; i < g_wavelength_sample_count; i ++) {
            if (maximum < data[i]) {
                maximum = data[i];
            }
        }
        return maximum;
    }

    float operator[](size_t idx) const { return data[idx]; }

    float &operator[](size_t idx) { return data[idx]; }

    SpectrumSamples operator+(const SpectrumSamples &rhs) {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = data[i] + rhs[i];
        }
        return result;
    }

    SpectrumSamples operator-(const SpectrumSamples &rhs) {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = data[i] - rhs[i];
        }
        return result;
    }

    SpectrumSamples operator*(const SpectrumSamples &rhs) {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = data[i] * rhs[i];
        }
        return result;
    }

    SpectrumSamples operator/(const SpectrumSamples &rhs) {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = data[i] / rhs[i];
        }
        return result;
    }

    SpectrumSamples operator*(float rhs) {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = data[i] * rhs;
        }
        return result;
    }

    SpectrumSamples operator/(float rhs) {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = data[i] / rhs;
        }
        return result;
    }

    SpectrumSamples &operator+=(const SpectrumSamples &rhs) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] += rhs[i];
        }
        return *this;
    }

    SpectrumSamples &operator-=(const SpectrumSamples &rhs) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] -= rhs[i];
        }
        return *this;
    }

    SpectrumSamples &operator*=(const SpectrumSamples &rhs) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] *= rhs[i];
        }
        return *this;
    }

    SpectrumSamples &operator/=(const SpectrumSamples &rhs) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] /= rhs[i];
        }
        return *this;
    }

    SpectrumSamples &operator*=(float rhs) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] *= rhs;
        }
        return *this;
    }

    SpectrumSamples &operator/=(float rhs) {
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            data[i] /= rhs;
        }
        return *this;
    }
public:
    std::array<float, g_wavelength_sample_count> data {};
};

inline SpectrumSamples operator+(float lhs, const SpectrumSamples &rhs) {
    SpectrumSamples result {};
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        result[i] = lhs + rhs[i];
    }
    return result;
}

inline SpectrumSamples operator-(float lhs, const SpectrumSamples &rhs) {
    SpectrumSamples result {};
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        result[i] = lhs - rhs[i];
    }
    return result;
}

inline SpectrumSamples operator*(float lhs, const SpectrumSamples &rhs) {
    SpectrumSamples result {};
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        result[i] = lhs * rhs[i];
    }
    return result;
}

inline SpectrumSamples operator/(float lhs, const SpectrumSamples &rhs) {
    SpectrumSamples result {};
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        result[i] = lhs / rhs[i];
    }
    return result;
}

class Spectrum {
public:
    Spectrum() : lambda_min(g_lambda_min), lambda_max(g_lambda_max) {}

    explicit Spectrum(float lambda_min, float lambda_max) : lambda_min(lambda_min), lambda_max(lambda_max) {}

    virtual float operator[](float lambda) const = 0;

    virtual float max() const { return maximum; }

    virtual bool isConstant() const { return false; }

    SpectrumSamples sample(const WavelengthSamples &wavelength) const {
        SpectrumSamples result {};
        for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
            result[i] = (*this)[wavelength.lambdas[i]];
        }
        return result;
    }
protected:
    float lambda_min {}, lambda_max {};
    float maximum {};
};
