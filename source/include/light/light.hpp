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
    virtual bool skipMISCompensation() const = 0;

    virtual float Phi(float scene_radius) const = 0;
    virtual std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const = 0;

    virtual glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const = 0;
    virtual float getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const = 0;
};
