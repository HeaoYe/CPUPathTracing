#pragma once

#include <filesystem>
#include <glm/glm.hpp>

class Image {
public:
    Image(const std::filesystem::path &filename);
    Image(const std::vector<glm::vec3> &pixels, size_t width, size_t height) : pixels(pixels), width(width), height(height) {}
    Image(std::vector<glm::vec3> &&pixels, size_t width, size_t height) : pixels(pixels), width(width), height(height) {}

    glm::vec3 getPixel(size_t x, size_t y) const { return pixels[glm::clamp<size_t>(y, 0, height - 1) * width + glm::clamp<size_t>(x, 0, width - 1)]; }
    glm::vec3 getPixel(const glm::vec2 &point) const { return getPixel(static_cast<size_t>(point.x), static_cast<size_t>(point.y)); }
    void setPixel(size_t x, size_t y, const glm::vec3 &value) {
        pixels[glm::clamp<size_t>(y, 0, height - 1) * width + glm::clamp<size_t>(x, 0, width - 1)] = value;
    }

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    glm::ivec2 getResolution() const { return { width, height }; }

    void save(const std::filesystem::path &filename) const;
private:
    void savePPM(const std::filesystem::path &filename) const;
    void saveEXR(const std::filesystem::path &filename) const;
private:
    size_t width, height;
    std::vector<glm::vec3> pixels;
};
