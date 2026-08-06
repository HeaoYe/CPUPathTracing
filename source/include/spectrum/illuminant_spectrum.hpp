#pragma once

#include "spectrum.hpp"
#include "illuminant.hpp"

class IlluminantSpectrum : public Spectrum {
public:
    explicit IlluminantSpectrum(
        const Spectrum &illuminant, float luminance,
        float lambda_min = g_lambda_min, float lambda_max = g_lambda_max
    ) : Spectrum(lambda_min, lambda_max), illuminant(illuminant) {
        float total = 0;
        for (float lambda = lambda_min; lambda <= lambda_max; lambda += 1) {
            total += illuminant[lambda] * CIE_1924_V[lambda];
        }
        k = luminance / (total * Kcd);
        maximum = k * illuminant.max();
    }

    float operator[](float lambda) const override {
        if (lambda < lambda_min || lambda > lambda_max) {
            return 0;
        }
        return k * illuminant[lambda];
    }
private:
    const Spectrum &illuminant;
    float k {};
};
