#include "shape/triangle.hpp"

std::optional<HitInfo> Triangle::intersect(const Ray &ray, float t_min, float t_max) const {
    glm::vec3 e1 = p1 - p0;
    glm::vec3 e2 = p2 - p0;
    glm::vec3 s1 = glm::cross(ray.direction, e2);
    float inv_det = 1.f / glm::dot(s1, e1);

    glm::vec3 s = ray.origin - p0;
    float u = glm::dot(s1, s) * inv_det;
    if (u < 0 || u > 1) { return {}; }

    glm::vec3 s2 = glm::cross(s, e1);
    float v = glm::dot(s2, ray.direction) * inv_det;
    if (v < 0 || u + v > 1) { return {}; }

    float hit_t = glm::dot(s2, e2) * inv_det;
    if (hit_t > t_min && hit_t < t_max) {
        glm::vec3 hit_point = ray.hit(hit_t);
        glm::vec3 normal = (1.f - u - v) * n0 + u * n1 + v * n2;
        return HitInfo { hit_t, hit_point, glm::normalize(normal) };
    }
    return {};
}
