#pragma once

#include "camera/ray.hpp"

struct Bounds {
    Bounds() : b_min(std::numeric_limits<float>::infinity()), b_max(-std::numeric_limits<float>::infinity()) {}

    Bounds(const glm::vec3 &b_min, const glm::vec3 &b_max) : b_min(b_min), b_max(b_max) {}

    void expand(const glm::vec3 &pos) {
        b_min = glm::min(pos, b_min);
        b_max = glm::max(pos, b_max);
    }

    void expand(const Bounds &bounds) {
        b_min = glm::min(bounds.b_min, b_min);
        b_max = glm::max(bounds.b_max, b_max);
    }

    bool hasIntersection(const Ray &ray, float t_min, float t_max) const;

    bool hasIntersection(const Ray &ray, const glm::vec3 &inv_direction, float t_min, float t_max) const;

    glm::vec3 diagonal() const { return b_max - b_min; }

    float area() const {
        auto diag = diagonal();
        return (diag.x * (diag.y + diag.z) + diag.y * diag.z) * 2.f;
    }

    glm::vec3 b_min;
    glm::vec3 b_max;
};
