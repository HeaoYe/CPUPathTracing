#pragma once

#include <filesystem>
#include <vector>
#include <glm/glm.hpp>

struct Pixel {
    glm::dvec3 color { 0, 0, 0 };
    int sample_count { 0 };
};

class Film {
public:
    Film(size_t width, size_t height);

    void save(const std::filesystem::path &filename) const;

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    Pixel getPixel(size_t x, size_t y) const { return pixels[y * width + x]; }
    void addSample(size_t x, size_t y, const glm::vec3 &color) {
        if (glm::any(glm::isnan(color))) {
            return;
        }
        pixels[y * width + x].color += color;
        pixels[y * width + x].sample_count ++;
    }
    void clear() { pixels.clear(); pixels.resize(width * height); }
    void setResolution(size_t width, size_t height) {
        this->width = width;
        this->height = height;
        pixels.resize(width * height);
    }
    std::vector<uint8_t> generateRGBABuffer();
private:
    size_t width, height;
    std::vector<Pixel> pixels;
};
