#pragma once

#include "light.hpp"

class InfiniteLight : public Light {
public:
    InfiniteLight(const glm::vec3 &Le) : Light(Le) {}

    float Phi(float scene_radius) const override;
    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng) const override;
};
