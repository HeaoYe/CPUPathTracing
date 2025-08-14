#pragma once

#include "shape.hpp"

struct Plane : public Shape {
    Plane(const glm::vec3 &point, const glm::vec3 &normal, float radius) : point(point), normal(glm::normalize(normal)), radius(radius) {
        glm::vec3 up = glm::abs(normal.y) < 0.99999 ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, -1);
        x_axis = glm::normalize(-glm::cross(up, normal));
        z_axis = glm::normalize(glm::cross(x_axis, normal));

        Bounds bounds_local { { -radius, -0.001, -radius }, { radius, 0.001, radius } };
        for (size_t i = 0; i < 8; i ++) {
            glm::vec3 corner = bounds_local.getCorner(i);
            bounds.expand(corner.x * x_axis + corner.y * normal + corner.z * z_axis);
        }
    }

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;

    float getArea() const override;
    std::optional<ShapeSample> sampleShape(const RNG &rng) const override;

    glm::vec3 point;
    glm::vec3 normal, x_axis, z_axis;
    Bounds bounds;
    float radius;
};
