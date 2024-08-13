#include "shape/plane.hpp"

std::optional<HitInfo> Plane::intersect(const Ray &ray, float t_min, float t_max) const {
    float hit_t = glm::dot(point - ray.origin, normal) / glm::dot(ray.direction, normal);
    if (hit_t > t_min && hit_t < t_max) {
        return HitInfo { hit_t, ray.hit(hit_t), normal };
    }
    return {};
}
