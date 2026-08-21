#include "image/rgb_illuminant_image.hpp"
#include "thread/thread_pool.hpp"

RGBIlluminantImage::RGBIlluminantImage(const ColorLUT *color_lut, const Image &image) {
    width = image.getWidth();
    height = image.getHeight();
    pixels.resize(width * height);
    illuminant = color_lut->getIlluminant();

    float k = 0;
    for (int lambda = g_lambda_min; lambda <= g_lambda_max; lambda ++) {
        k += (*illuminant)[lambda] * Y_color_matching[lambda];
    }
    k = 1 / k;

    thread_pool.parallelFor(width, height, [&](size_t x, size_t y) {
        auto linear_rgb = image.getPixel(x, y);
        if (color_lut->getColorSpace() != image.getColorSpace()) {
            linear_rgb = color_lut->getColorSpace()->RGBFromXYZ(image.getColorSpace()->XYZFromRGB(linear_rgb));
            linear_rgb.data = glm::max(linear_rgb.data, glm::vec3());
        }

        float scale = 2 * glm::max(glm::max(linear_rgb.r(), linear_rgb.g()), linear_rgb.b());
        auto lut_rgb = linear_rgb / scale;
        auto spec = color_lut->look(lut_rgb);

        setPixel(x, y, {
            spec.c0(),
            spec.c1(),
            spec.c2(),
            scale * k,
        });
    });
    thread_pool.wait();
}
