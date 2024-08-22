#pragma once

#include "../camera/ray.hpp"
#include "../accelerate/bounds.hpp"
#include <optional>

struct Shape {
    virtual std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const = 0;
    virtual Bounds getBounds() const { return {}; }
};
