#include "camera/film.hpp"
#include "thread/thread_pool.hpp"
#include "image/image.hpp"

Film::Film(size_t width, size_t height) : width(width), height(height) {
    pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &filename, const ColorSpace *color_space) const {
    std::vector<LinearRGB> buffer(width * height);
    thread_pool.parallelFor(width, height, [&](size_t x, size_t y) {
        auto pixel = getPixel(x, y);
        if (pixel.sample_count == 0) {
            return;
        }
        XYZ xyz { pixel.color_xyz / static_cast<double>(pixel.sample_count) };
        buffer[y * width + x] = color_space->RGBFromXYZ(xyz);
    }, false);
    thread_pool.wait();

    Image image(std::move(buffer), width, height, color_space);
    image.save(filename);
}

std::vector<uint8_t> Film::generateRGBABuffer(const ColorSpace *color_space) {
    std::vector<uint8_t> buffer(width * height * 4);

    for (size_t y = 0; y < height; y ++) {
        for (size_t x = 0; x < width; x ++) {
            auto pixel = getPixel(x, y);
            if (pixel.sample_count == 0) {
                continue;
            }
            XYZ xyz { pixel.color_xyz / static_cast<double>(pixel.sample_count) };
            LinearRGB linear_rgb { glm::clamp(color_space->RGBFromXYZ(xyz).data, 0.f, 1.f) };
            auto rgb = color_space->encode(linear_rgb).toBytes(8);
            auto idx = (y * width + x) * 4;
            buffer[idx + 0] = rgb.r;
            buffer[idx + 1] = rgb.g;
            buffer[idx + 2] = rgb.b;
            buffer[idx + 3] = 255;
        }
    }

    return buffer;
}
