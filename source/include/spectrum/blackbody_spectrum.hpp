#pragma once

#include "spectrum.hpp"
#include <algorithm>
#include <cmath>

class BlackbodySpectrum : public Spectrum {
public:
    explicit BlackbodySpectrum(float T, float lambda_min = g_lambda_min, float lambda_max = g_lambda_max)
        : Spectrum(lambda_min, lambda_max), T(T) {
        maximum = std::max((*this)[lambda_min], (*this)[lambda_max]);
        for (float lambda = lambda_min + 0.1; lambda < lambda_max; lambda += 0.1) {
            float value = (*this)[lambda];
            if (maximum < value) {
                maximum = value;
            }
        }
    }

    float operator[](float lambda) const {
        if (lambda < lambda_min || lambda > lambda_max) {
            return 0;
        }

        const float c = 299792458.f;
        const float h = 6.62606957e-34f;
        const float kb = 1.3806488e-23f;

        float l = lambda * 1e-9f;
        return 2 * h * c * c / (std::pow(l, 5) * std::expm1(h * c / (l * kb * T)));
    }
private:
    float T;
};
