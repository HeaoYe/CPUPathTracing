#pragma once

#include "color/color_space.hpp"
#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

class PixelSample {
public:
    PixelSample() = default;

    PixelSample(const SpectrumSamples &spectrum_samples, const WavelengthSamples &wavelength)
        : spectrum_samples(spectrum_samples), wavelength(wavelength), type(ContributeType::eSpectrum) {}

    PixelSample(const EncodedRGB &encoded_rgb, const ColorSpace *color_space)
        : encoded_rgb(encoded_rgb), color_space(color_space), type(ContributeType::eRGB) {}
public:
    union {
        struct {
            SpectrumSamples spectrum_samples;
            WavelengthSamples wavelength;
        };
        struct {
            EncodedRGB encoded_rgb {};
            const ColorSpace *color_space {};
        };
    };

    enum class ContributeType {
        eNone,
        eSpectrum,
        eRGB,
    } type { ContributeType::eNone };
};

struct Pixel {
    glm::dvec3 color_xyz { 0, 0, 0 };
    int sample_count { 0 };
};

class Film {
public:
    Film(size_t width, size_t height);

    void save(const std::filesystem::path &filename, const ColorSpace *color_space) const;

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    Pixel getPixel(size_t x, size_t y) const { return pixels[y * width + x]; }
    void addSample(size_t x, size_t y, const PixelSample &sample) {
        XYZ xyz_sample {};
        switch (sample.type) {
        case PixelSample::ContributeType::eSpectrum:
            xyz_sample = XYZ(sample.spectrum_samples, sample.wavelength);
            break;
        case PixelSample::ContributeType::eRGB:
            xyz_sample = sample.color_space->XYZFromRGB(sample.color_space->decode(sample.encoded_rgb));
            break;
        case PixelSample::ContributeType::eNone:
        default:
            return;
        }
        if (glm::any(glm::isnan(xyz_sample.data))) {
            return;
        }
        pixels[y * width + x].color_xyz += xyz_sample.data;
        pixels[y * width + x].sample_count ++;
    }
    void clear() { pixels.clear(); pixels.resize(width * height); }
    void setResolution(size_t width, size_t height) {
        this->width = width;
        this->height = height;
        pixels.resize(width * height);
    }
    std::vector<uint8_t> generateRGBABuffer(const ColorSpace *color_space);
private:
    size_t width, height;
    std::vector<Pixel> pixels;
};
