#pragma once

#include "image.hpp"
#include "color/color_lut.hpp"

class RGBIlluminantImage {
private:
    struct Pixel {
        float c0 {};
        float c1 {};
        float c2 {};
        float k {};
    };
public:
    RGBIlluminantImage(const ColorLUT *color_lut, const Image &image);

    SpectrumSamples sample(size_t x, size_t y, const WavelengthSamples &wavelength) const {
        auto pixel = pixels[glm::clamp<size_t>(y, 0, height - 1) * width + glm::clamp<size_t>(x, 0, width - 1)];
        return SigmoidPolynomialSpectrum(pixel.c0, pixel.c1, pixel.c2).sample(wavelength) * illuminant->sample(wavelength) * pixel.k;
    }

    SpectrumSamples sample(const glm::vec2 &point, const WavelengthSamples &wavelength) const {
        return sample(static_cast<size_t>(point.x), static_cast<size_t>(point.y), wavelength);
    }

    void setPixel(size_t x, size_t y, const Pixel &value) {
        pixels[glm::clamp<size_t>(y, 0, height - 1) * width + glm::clamp<size_t>(x, 0, width - 1)] = value;
    }

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    glm::ivec2 getResolution() const { return { width, height }; }
private:
    std::vector<Pixel> pixels {};
    size_t width {}, height {};
    const Spectrum *illuminant {};
};
