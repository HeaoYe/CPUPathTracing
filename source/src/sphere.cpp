#include "sphere.hpp"

std::optional<float> Sphere::intersect(const Ray &ray) const {
    glm::vec3 co = ray.origin - center;
    float b = 2 * glm::dot(ray.direction, co);
    float c = glm::dot(co, co) - radius * radius;
    float delta = b * b - 4 * c;
    if (delta < 0) {
        return {};
    }
    float hit_t = (-b - sqrt(delta)) * 0.5;
    if (hit_t < 0) {
        hit_t = (-b + sqrt(delta)) * 0.5;
    }
    return hit_t;
}
