#pragma once

#include "util/rng.hpp"
#include <glm/glm.hpp>
#include <optional>

struct BSDFSample {
    glm::vec3 bsdf;
    float pdf;
    glm::vec3 light_direction;
};

class Material {
public:
    virtual std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const = 0;
    virtual glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const = 0;
    virtual bool isDeltaDistribution() const = 0;
public:
    const class AreaLight *area_light { nullptr };
};
