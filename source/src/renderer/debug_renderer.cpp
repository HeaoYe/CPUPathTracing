#include "renderer/debug_renderer.hpp"
#include "util/rgb.hpp"

glm::vec3 BoundsTestCountRenderer::renderPixel(const glm::ivec2 &pixel_coord) {
    #ifdef WITH_DEBUG_INFO
    auto ray = camera.generateRay(pixel_coord);
    scene.intersect(ray);
    return RGB::GenerateHeatmapRGB(ray.bounds_test_count / 150.f);
    return {};
    #else
    return {};
    #endif
}

glm::vec3 TriangleTestCountRenderer::renderPixel(const glm::ivec2 &pixel_coord) {
    #ifdef WITH_DEBUG_INFO
    auto ray = camera.generateRay(pixel_coord);
    scene.intersect(ray);
    return RGB::GenerateHeatmapRGB(ray.triangle_test_count / 7.f);
    return {};
    #else
    return {};
    #endif
}
