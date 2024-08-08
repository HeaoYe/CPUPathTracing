#pragma once

#include "ray.hpp"
#include <optional>

struct Shape {
    virtual std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min = 1e-5,
        float t_max = std::numeric_limits<float>::infinity()
    ) const = 0;
};
