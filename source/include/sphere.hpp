#pragma once

#include "ray.hpp"
#include <optional>

struct Sphere {
    glm::vec3 center;
    float radius;

    std::optional<float> intersect(const Ray &ray) const;
};
