#include "renderer/path_tracing_renderer.hpp"
#include "util/frame.hpp"
#include "sample/spherical.hpp"

glm::vec3 PathTracingRenderer::renderPixel(const glm::ivec2 &pixel_coord) {
    auto ray = camera.generateRay(pixel_coord, { rng.uniform(), rng.uniform() });
    glm::vec3 beta = { 1, 1, 1 };
    glm::vec3 L = { 0, 0, 0 };
    float q = 0.9;

    while (true) {
        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
            if (rng.uniform() > q) {
                break;
            }
            L += beta * hit_info->material->emissive;
            beta *= hit_info->material->albedo / q;

            Frame frame(hit_info->normal);
            glm::vec3 light_direction;
            if (hit_info->material->is_specular) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                light_direction = { -view_direction.x, view_direction.y, -view_direction.z };
            } else {
                light_direction = CosineSampleHemisphere({ rng.uniform(), rng.uniform() });
            }

            ray.origin = hit_info->hit_point;
            ray.direction = frame.worldFromLocal(light_direction);
        } else {
            break;
        }
    }

    return L;
}
