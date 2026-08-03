#pragma once

#include <array>
#include <cmath>

constexpr int g_lambda_min = 360;
constexpr int g_lambda_max = 830;

constexpr size_t g_wavelength_sample_count = 4;

struct WavelengthSamples {
    std::array<float, g_wavelength_sample_count> lambdas {};
    std::array<float, g_wavelength_sample_count> pdfs {};
};

inline WavelengthSamples UniformSampleWavelength(float u) {
    WavelengthSamples result {};
    float delta = static_cast<float>(g_lambda_max - g_lambda_min) / g_wavelength_sample_count;
    float lambda_start = g_lambda_min * (1 - u) + g_lambda_max * u;
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        result.lambdas[i] = lambda_start + i * delta;
        result.pdfs[i] = 1.f / (g_lambda_max - g_lambda_min);
        if (result.lambdas[i] > g_lambda_max) {
            result.lambdas[i] = g_lambda_min + (result.lambdas[i] - g_lambda_max);
        }
    }
    return result;
}

inline WavelengthSamples ImportanceSampleWavelength(float u) {
    WavelengthSamples result {};
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        float ui = u + static_cast<float>(i) / g_wavelength_sample_count;
        if (ui > 1) {
            ui -= 1;
        }

        float lambda = 538 + 138.888889f * std::atanh(1.82750197f * ui - 0.85691062f);
        result.lambdas[i] = lambda;
        result.pdfs[i] = 0.0039398042f / std::pow(std::cosh(0.0072f * (lambda - 538)), 2);
    }
    return result;
}
