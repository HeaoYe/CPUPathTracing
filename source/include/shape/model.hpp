#pragma once

#include "accelerate/bounds.hpp"
#include "triangle.hpp"
#include <filesystem>

class Model : public Shape {
public:
    Model(const std::vector<Triangle> &triangles) : triangles(triangles) { build(); }

    Model(const std::filesystem::path &filename);

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
private:
    void build();
private:
    Bounds bounds {};
    std::vector<Triangle> triangles;
};
