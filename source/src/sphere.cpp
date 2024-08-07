#include "sphere.hpp"

std::optional<HitInfo> Sphere::intersect(const Ray &ray, float t_min, float t_max) const {
    glm::vec3 co = ray.origin - center;
    float b = 2 * glm::dot(ray.direction, co);
    float c = glm::dot(co, co) - radius * radius;
    float delta = b * b - 4 * c;
    if (delta < 0) { return {}; }
    float hit_t = (-b - glm::sqrt(delta)) * 0.5;
    if (hit_t < 0) {
        hit_t = (-b + glm::sqrt(delta)) * 0.5;
    }
    if (hit_t > t_min && hit_t < t_max) {
        glm::vec3 hit_point = ray.hit(hit_t);
        glm::vec3 normal = glm::normalize(hit_point - center);
        return HitInfo { hit_t, hit_point, normal };
    }
    return {};
}
