#pragma once

#include "light.hpp"
#include "shape/shape.hpp"

class AreaLight : public Light {
public:
    AreaLight(const Shape &shape, const glm::vec3 &Le, bool double_side) : shape(shape), Le(Le), double_side(double_side) {}
    bool impossible() const override { return false; }

    float Phi(float scene_radius) const override;
    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const override;
    glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const override;
    float getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const override;

    const Shape &getShape() const { return shape; }
private:
    const Shape &shape;
    glm::vec3 Le;
    bool double_side;
};
