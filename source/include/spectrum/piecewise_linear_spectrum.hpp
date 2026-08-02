#pragma once

#include "spectrum.hpp"
#include <vector>
#include <algorithm>

class PiecewiseLinearSpectrum : public Spectrum {
public:
    struct SamplePoint {
        float lambda;
        float value;
    };
public:
    explicit PiecewiseLinearSpectrum(const std::vector<SamplePoint> &samples)
        : Spectrum(samples.front().lambda, samples.back().lambda) {
        lambdas.reserve(samples.size());
        values.reserve(samples.size());
        maximum = samples.front().value;
        for (const auto &sample : samples) {
            lambdas.push_back(sample.lambda);
            values.push_back(sample.value);
            if (maximum < sample.value) {
                maximum = sample.value;
            }
        }
    }

    explicit PiecewiseLinearSpectrum(const std::vector<float> &lambdas, const std::vector<float> &values)
        : Spectrum(lambdas.front(), lambdas.back()), lambdas(lambdas), values(values) {
        maximum = values.front();
        for (size_t i = 1; i < values.size(); i ++) {
            if (maximum < values[i]) {
                maximum = values[i];
            }
        }
    }

    float operator[](float lambda) const override {
        if (lambda < lambda_min || lambda > lambda_max) {
            return 0;
        }
        if (lambda == lambda_max) {
            return values.back();
        }
        size_t right = std::upper_bound(lambdas.begin(), lambdas.end(), lambda) - lambdas.begin();
        float t = (lambda - lambdas[right - 1]) / (lambdas[right] - lambdas[right - 1]);
        return values[right - 1] * (1 - t) + values[right] * t;
    }
private:
    std::vector<float> lambdas;
    std::vector<float> values;
};
