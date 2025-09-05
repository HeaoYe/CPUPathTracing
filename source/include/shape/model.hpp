#pragma once

#include "accelerate/bvh.hpp"
#include "triangle.hpp"
#include <filesystem>

class Model {
public:
    Model(const std::vector<Triangle> &triangles) {
        auto ts = triangles;
        bvh.build(std::move(ts));
    }

    Model(const std::filesystem::path &filename);

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const;

    Bounds getBounds() const { return bvh.getBounds(); }
    float getArea() const { return bvh.getArea(); }
    std::optional<ShapeSample> sampleShape(const RNG &rng) const { return bvh.sampleShape(rng); }
    float PDF(const glm::vec3 &point, const glm::vec3 &normal) const { return bvh.PDF(point, normal); }
private:
    BVH bvh {};
};
