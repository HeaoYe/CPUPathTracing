#pragma once

#include "color/color_space.hpp"
#include "spectrum/sigmoid_polynomial_spectrum.hpp"
#include <filesystem>
#include <array>

constexpr size_t lut_resolution = 64;

class ColorLUT {
public:
    ColorLUT(const std::filesystem::path &filename, const ColorSpace *color_space, const Spectrum *illuminant);
    ~ColorLUT();

    SigmoidPolynomialSpectrum look(uint8_t r, uint8_t g, uint8_t b) const { return look(color_space->decode(EncodedRGB(r, g, b, 8))); }
    SigmoidPolynomialSpectrum look(const EncodedRGB &encoded_rgb) const { return look(color_space->decode(encoded_rgb)); }
    SigmoidPolynomialSpectrum look(const LinearRGB &linear_rgb) const;

    const ColorSpace *getColorSpace() const { return color_space; }
    const Spectrum *getIlluminant() const { return illuminant; }
private:
    size_t findAlphaNodeIdx(float alpha) const {
        if (alpha <= alpha_nodes->front()) {
            return 0;
        }
        if (alpha >= alpha_nodes->back()) {
            return lut_resolution - 2;
        }
        return std::upper_bound(alpha_nodes->begin(), alpha_nodes->end(), alpha) - alpha_nodes->begin() - 1;
    }

    glm::vec3 at(size_t i, size_t x, size_t y, size_t alpha) const {
        return { (*data)[i][x][y][alpha][0], (*data)[i][x][y][alpha][1], (*data)[i][x][y][alpha][2] };
    }

    bool load(const std::filesystem::path &filename);
    void genetate();
    void save(const std::filesystem::path &filename);
private:
    const ColorSpace *color_space {};
    const Spectrum *illuminant {};

    using AlphaNodes = std::array<double, lut_resolution>;
    using LUTData = std::array<
        std::array<
            std::array<
                std::array<
                    std::array<float, 3>,
                    lut_resolution>,
                lut_resolution>,
            lut_resolution>,
        3>;
    AlphaNodes *alpha_nodes {};
    LUTData *data {};
};

extern ColorLUT *ColorLUT_sRGB;
