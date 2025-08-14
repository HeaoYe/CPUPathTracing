#pragma once

#include "camera/ray.hpp"
#include "accelerate/bounds.hpp"
#include <optional>

struct ShapeSample {
    glm::vec3 point;
    glm::vec3 normal;
    float pdf;
};

struct Shape {
    virtual std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const = 0;
    virtual Bounds getBounds() const { return {}; }
    virtual float getArea() const { return -1; }
    virtual std::optional<ShapeSample> sampleShape(const RNG &rng) const { return {}; }
};
