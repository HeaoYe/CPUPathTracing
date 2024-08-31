// #include "renderer/simple_rt_renderer.hpp"
// #include "util/frame.hpp"
// #include "sample/spherical.hpp"

// glm::vec3 SimpleRTRenderer::renderPixel(const glm::ivec2 &pixel_coord) {
//     auto ray = camera.generateRay(pixel_coord, { rng.uniform(), rng.uniform() });
//     glm::vec3 beta = { 1, 1, 1 };
//     glm::vec3 color = { 0, 0, 0 };
//     size_t max_bounce_count = 32;

//     while (max_bounce_count--) {
//         auto hit_info = scene.intersect(ray);
//         if (hit_info.has_value()) {
//             color += beta * hit_info->material->emissive;
//             beta *= hit_info->material->albedo;

//             ray.origin = hit_info->hit_point;

//             Frame frame(hit_info->normal);
//             glm::vec3 light_direction;
//             if (hit_info->material->is_specular) {
//                 glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
//                 light_direction = { -view_direction.x, view_direction.y, -view_direction.z };
//             } else {
//                 light_direction = UniformSampleHemisphere(rng);
//             }
//             ray.direction = frame.worldFromLocal(light_direction);
//         } else {
//             break;
//         }
//     }

//     return color;
// }
