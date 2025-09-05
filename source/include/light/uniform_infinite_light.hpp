#pragma once

#include "light_sample.hpp"
#include "util/rng.hpp"
#include <optional>

class UniformInfiniteLight {
public:
    UniformInfiniteLight(const glm::vec3 &Le) : Le(Le) {}
    bool impossible() const { return true; }

    float Phi(float scene_radius) const;
    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const;
    glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const;
    float getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const;
private:
    glm::vec3 Le;
};
