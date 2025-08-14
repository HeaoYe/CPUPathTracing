#pragma once

#include "light.hpp"
#include "shape/shape.hpp"

class AreaLight : public Light {
public:
    AreaLight(const Shape &shape, const glm::vec3 &Le, bool double_side) : Light(Le), shape(shape), double_side(double_side) {}

    float Phi(float scene_radius) const override;
    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng) const override;
    glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const override;

    const Shape &getShape() const { return shape; }
private:
    const Shape &shape;
    bool double_side;
};
