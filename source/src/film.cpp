#include "film.hpp"
#include <fstream>

Film::Film(size_t width, size_t height) : width(width), height(height) {
    pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &filename) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << width << ' ' << height << "\n255\n";

    for (size_t y = 0; y < height; y ++) {
        for (size_t x = 0; x < width; x ++) {
            const glm::vec3 &color = getPixel(x, y);
            glm::ivec3 color_i = glm::clamp(color * 255.f, 0.f, 255.f);
            file << static_cast<uint8_t>(color_i.x) << static_cast<uint8_t>(color_i.y) << static_cast<uint8_t>(color_i.z);
        }
    }
}
