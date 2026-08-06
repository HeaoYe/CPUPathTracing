#include "renderer/debug_renderer.hpp"

PixelSample BoundsTestCountRenderer::renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) {
    #ifdef WITH_DEBUG_INFO
    auto ray = camera.generateRay(pixel_coord);
    scene.intersect(ray);
    return { EncodedRGB::GenerateHeatmapRGB(ray.bounds_test_count / 150.f), target_color_space };
    #else
    return {};
    #endif
}

PixelSample TriangleTestCountRenderer::renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) {
    #ifdef WITH_DEBUG_INFO
    auto ray = camera.generateRay(pixel_coord);
    scene.intersect(ray);
    return { EncodedRGB::GenerateHeatmapRGB(ray.triangle_test_count / 7.f), target_color_space };
    #else
    return {};
    #endif
}
