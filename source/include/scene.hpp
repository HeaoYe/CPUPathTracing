#pragma once

#include "shape.hpp"

struct ShapeInstance {
    const Shape &shape;
    Material materail;
    glm::mat4 world_from_object;
    glm::mat4 object_from_world;
};

struct Scene : public Shape {
public:
    void addShape(
        const Shape &shape,
        const Material &material = {},
        const glm::vec3 &pos = { 0, 0, 0 },
        const glm::vec3 &scale = { 1, 1, 1 },
        const glm::vec3 &rotate = { 0, 0, 0 }
    );

    std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min = 1e-5,
        float t_max = std::numeric_limits<float>::infinity()
    ) const override;
private:
    std::vector<ShapeInstance> instances;
};
