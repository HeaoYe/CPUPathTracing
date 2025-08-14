#pragma once

#include "shape.hpp"

struct Sphere : public Shape {
    Sphere (const glm::vec3 &center, float radius) : center(center), radius(radius) {}

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;

    Bounds getBounds() const override { return { center - radius, center + radius }; }

    float getArea() const override;
    std::optional<ShapeSample> sampleShape(const RNG &rng) const override;

    glm::vec3 center;
    float radius;
};
