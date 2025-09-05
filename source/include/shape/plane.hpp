#pragma once

#include "shape_sample.hpp"
#include "camera/ray.hpp"
#include "accelerate/bounds.hpp"
#include <optional>

struct Plane {
    Plane(const glm::vec3 &point, const glm::vec3 &normal, float radius) : point(point), normal(glm::normalize(normal)), bounds(), radius(radius) {
        glm::vec3 up = glm::abs(this->normal.y) < 0.99999 ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);
        x_axis = glm::normalize(glm::cross(this->normal, up));
        z_axis = glm::normalize(glm::cross(x_axis, this->normal));

        Bounds bounds_local { { -radius, -0.001, -radius }, { radius, 0.001, radius } };
        for (size_t i = 0; i < 8; i ++) {
            glm::vec3 corner = bounds_local.getCorner(i);
            bounds.expand(point + corner.x * x_axis + corner.y * normal + corner.z * z_axis);
        }
    }

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const;

    Bounds getBounds() const { return bounds; }

    float getArea() const;
    std::optional<ShapeSample> sampleShape(const RNG &rng) const;
    float PDF(const glm::vec3 &point, const glm::vec3 &normal) const { return 1.f / getArea(); }

    glm::vec3 point;
    glm::vec3 normal, x_axis, z_axis;
    Bounds bounds;
    float radius;
};
