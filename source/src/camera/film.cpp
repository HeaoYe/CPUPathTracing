#include "camera/film.hpp"
#include "thread/thread_pool.hpp"
#include "util/rgb.hpp"
#include <fstream>

Film::Film(size_t width, size_t height) : width(width), height(height) {
    pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &filename) {
    std::ofstream file(filename, std::ios::binary);
    file << "P6\n" << width << ' ' << height << "\n255\n";

    std::vector<uint8_t> buffer(width * height * 3);

    thread_pool.parallelFor(width, height, [&](size_t x, size_t y) {
        auto pixel = getPixel(x, y);
        RGB rgb(pixel.color / static_cast<float>(pixel.sample_count));
        auto idx = (y * width + x) * 3;
        buffer[idx + 0] = rgb.r;
        buffer[idx + 1] = rgb.g;
        buffer[idx + 2] = rgb.b;
    }, false);
    thread_pool.wait();

    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}
