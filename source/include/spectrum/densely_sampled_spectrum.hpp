#pragma once

#include "spectrum.hpp"
#include <vector>
#include <cmath>

class DenselySampledSpectrum : public Spectrum {
public:
    explicit DenselySampledSpectrum(
        const Spectrum &spectrum,
        int lambda_min = g_lambda_min, int lambda_max = g_lambda_max
    ) : Spectrum(lambda_min, lambda_max) {
        values.resize(lambda_max - lambda_min + 1);
        maximum = spectrum[lambda_min];
        for (size_t i = 0; i < g_lambda_max - g_lambda_min + 1; i ++) {
            values[i] = spectrum[lambda_min + i];
            if (maximum < values[i]) {
                maximum = values[i];
            }
        }
    }

    explicit DenselySampledSpectrum(
        const std::vector<float> &values,
        int lambda_min = g_lambda_min, int lambda_max = g_lambda_max
    ) : Spectrum(lambda_min, lambda_max), values(values) {
        if (values.size() < lambda_max - lambda_min + 1) {
            this->lambda_max = lambda_min + values.size() - 1;
        }
        maximum = values.front();
        for (size_t i = 1; i < values.size(); i ++) {
            if (maximum < values[i]) {
                maximum = values[i];
            }
        }
    }

    float operator[](float lambda) const {
        auto offset = std::lround(lambda - lambda_min);
        if (offset < 0 || offset >= values.size()) {
            return 0;
        }
        return values[offset];
    }
private:
    std::vector<float> values;
};
