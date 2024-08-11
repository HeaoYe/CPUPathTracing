#include "film.hpp"
#include "rgb.hpp"
#include <fstream>

Film::Film(size_t width, size_t height) : width(width), height(height) {
    pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &filename) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << width << ' ' << height << "\n255\n";

    for (size_t y = 0; y < height; y ++) {
        for (size_t x = 0; x < width; x ++) {
            auto pixel = getPixel(x, y);
            RGB rgb(pixel.color / static_cast<float>(pixel.sample_count));
            file << static_cast<uint8_t>(rgb.r) << static_cast<uint8_t>(rgb.g) << static_cast<uint8_t>(rgb.b);
        }
    }
}
