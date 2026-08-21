#pragma once

#include "spectrum/sigmoid_polynomial_spectrum.hpp"
#include "color/color_lut.hpp"

class RGBIlluminantSpectrum : public SigmoidPolynomialSpectrum {
public:
    RGBIlluminantSpectrum(const ColorLUT *color_lut, const LinearRGB &linear_rgb)
        : SigmoidPolynomialSpectrum(0, 0, 0), illuminant(color_lut->getIlluminant()), k(0), maximum(0) {
        float scale = 2 * glm::max(glm::max(linear_rgb.r(), linear_rgb.g()), linear_rgb.b());
        auto lut_rgb = linear_rgb / scale;
        auto spec = color_lut->look(lut_rgb);
        c0() = spec.c0();
        c1() = spec.c1();
        c2() = spec.c2();

        for (int lambda = g_lambda_min; lambda <= g_lambda_max; lambda ++) {
            float value = SigmoidPolynomialSpectrum::operator[](lambda) * (*illuminant)[lambda];
            if (value > maximum) {
                maximum = value;
            }
            k += (*illuminant)[lambda] * Y_color_matching[lambda];
        }
        k = scale / k;
        maximum *= k;
    }

    float operator[](float lambda) const override {
        return SigmoidPolynomialSpectrum::operator[](lambda) * (*illuminant)[lambda] * k;
    }

    float max() const override { return maximum; }
private:
    const Spectrum *illuminant {};
    float k {};
    float maximum {};
};
