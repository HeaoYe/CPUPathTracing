#pragma once

#include "shape.hpp"
#include "../accelerate/scene_bvh.hpp"

struct Scene : public Shape {
public:
    void addShape(
        const Shape &shape,
        const Material *material = nullptr,
        const glm::vec3 &pos = { 0, 0, 0 },
        const glm::vec3 &scale = { 1, 1, 1 },
        const glm::vec3 &rotate = { 0, 0, 0 }
    );

    std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min = 1e-5,
        float t_max = std::numeric_limits<float>::infinity()
    ) const override;

    void build() { scene_bvh.build(std::move(instances)); }
private:
    std::vector<ShapeInstance> instances;
    SceneBVH scene_bvh {};
};
