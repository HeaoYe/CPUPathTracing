#pragma once

#include "camera/ray.hpp"
#include "accelerate/bounds.hpp"
#include "util/generalized_ptr.hpp"

#include "shape_sample.hpp"
#include "sphere.hpp"
#include "plane.hpp"
#include "triangle.hpp"
#include "model.hpp"
#include "accelerate/bvh.hpp"

#include <optional>

struct Shape : public GeneralizedPtr<Sphere, Plane, Triangle, Model, BVH> {
    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const {
        return DISPATCH_CONST(intersect, ray, t_min, t_max);
    }

    Bounds getBounds() const {
        return DISPATCH_CONST(getBounds);
    }

    float getArea() const {
        return DISPATCH_CONST(getArea);
    }

    std::optional<ShapeSample> sampleShape(const RNG &rng) const {
        return DISPATCH_CONST(sampleShape, rng);
    }

    float PDF(const glm::vec3 &point, const glm::vec3 &normal) const {
        return DISPATCH_CONST(PDF, point, normal);
    }
};
