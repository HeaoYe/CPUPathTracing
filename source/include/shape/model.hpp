#pragma once

#include "accelerate/bvh.hpp"
#include "triangle.hpp"
#include <filesystem>

class Model : public Shape {
public:
    Model(const std::vector<Triangle> &triangles) {
        auto ts = triangles;
        bvh.build(std::move(ts));
    }

    Model(const std::filesystem::path &filename);

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;

    Bounds getBounds() const override { return bvh.getBounds(); }
    float getArea() const override { return bvh.getArea(); }
    std::optional<ShapeSample> sampleShape(const RNG &rng) const override { return bvh.sampleShape(rng); }
private:
    BVH bvh {};
};
