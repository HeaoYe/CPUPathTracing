#include "renderer/normal_renderer.hpp"
#include "util/rgb.hpp"

glm::vec3 NormalRenderer::renderPixel(const glm::ivec3 &pixel_coord) {
    auto ray = camera.generateRay(pixel_coord);
    auto hit_info = scene.intersect(ray);
    if (hit_info.has_value()) {
        glm::ivec3 color = (hit_info->normal * 0.5f + 0.5f) * 255.f;
        return RGB(color.r, color.g, color.b);
    }
    return {};
}
