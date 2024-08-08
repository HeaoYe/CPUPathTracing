#pragma once

#include "shape.hpp"

struct Plane : public Shape {
    Plane(const glm::vec3 &point, const glm::vec3 &normal) : point(point), normal(glm::normalize(normal)) {}

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;

    glm::vec3 point;
    glm::vec3 normal;
};
