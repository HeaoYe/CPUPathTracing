#include "renderer/normal_renderer.hpp"

PixelSample NormalRenderer::renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) {
    auto ray = camera.generateRay(pixel_coord);
    auto hit_info = scene.intersect(ray);
    if (hit_info.has_value()) {
        return { EncodedRGB { hit_info->normal * 0.5f + 0.5f }, target_color_space };
    }
    return {};
}
