#pragma once

#include "shape_sample.hpp"
#include "camera/ray.hpp"
#include "accelerate/bounds.hpp"
#include <optional>

struct Triangle {
    Triangle(
        const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2,
        const glm::vec3 &n0, const glm::vec3 &n1, const glm::vec3 &n2
    ) : p0(p0), p1(p1), p2(p2), n0(n0), n1(n1), n2(n2) {}

    Triangle(
        const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2
    ) : p0(p0), p1(p1), p2(p2) {
        glm::vec3 e1 = p1 - p0;
        glm::vec3 e2 = p2 - p0;
        glm::vec3 normal = glm::normalize(glm::cross(e1, e2));
        n0 = normal;
        n1 = normal;
        n2 = normal;
    }

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const;

    Bounds getBounds() const {
        Bounds bounds {};
        bounds.expand(p0);
        bounds.expand(p1);
        bounds.expand(p2);
        return bounds;
    }

    float getArea() const;
    std::optional<ShapeSample> sampleShape(const RNG &rng) const;
    float PDF(const glm::vec3 &point, const glm::vec3 &normal) const { return 1.f / getArea(); }

    glm::vec3 p0, p1, p2;
    glm::vec3 n0, n1, n2;
};
