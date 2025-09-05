#pragma once

#include "light_sample.hpp"
#include "shape/shape.hpp"

class AreaLight {
public:
    AreaLight(const Shape shape, const glm::vec3 &Le, bool double_side) : shape(shape), Le(Le), double_side(double_side) {}
    bool impossible() const { return false; }

    float Phi(float scene_radius) const;
    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const;
    glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const;
    float getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const;

    const Shape getShape() const { return shape; }
private:
    const Shape shape;
    glm::vec3 Le;
    bool double_side;
};
