#pragma once

#include "util/rng.hpp"
#include <glm/glm.hpp>
#include <optional>

struct LightSample {
    glm::vec3 light_point;
    glm::vec3 light_direction;
    glm::vec3 Le;
    float pdf;
};

class Light {
public:
    Light(const glm::vec3 &Le) : Le(Le) {}

    virtual float Phi(float scene_radius) const = 0;
    virtual std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng) const = 0;

    virtual glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const { return Le; }
protected:
    glm::vec3 Le;
};
