#pragma once

#include "triangle.hpp"
#include <filesystem>

class Model : public Shape {
public:
    Model(const std::vector<Triangle> &triangles) : triangles(triangles) {}

    Model(const std::filesystem::path &filename);

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
private:
    std::vector<Triangle> triangles;
};
