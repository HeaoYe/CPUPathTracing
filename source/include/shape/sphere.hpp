#pragma once

#include "shape_sample.hpp"
#include "camera/ray.hpp"
#include "accelerate/bounds.hpp"
#include <optional>

struct Sphere {
    Sphere (const glm::vec3 &center, float radius) : center(center), radius(radius) {}

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const;

    Bounds getBounds() const { return { center - radius, center + radius }; }

    float getArea() const;
    std::optional<ShapeSample> sampleShape(const RNG &rng) const;
    float PDF(const glm::vec3 &point, const glm::vec3 &normal) const { return 1.f / getArea(); }

    glm::vec3 center;
    float radius;
};
