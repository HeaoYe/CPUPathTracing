#pragma once

#include "bsdf_sample.hpp"
#include "util/rng.hpp"
#include <optional>

class SpecularMaterial {
public:
    SpecularMaterial(const glm::vec3 &albedo) : albedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const;
    glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const { return {}; }
    float PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const { return 0; }
    bool isDeltaDistribution() const { return true; }
public:
    const class AreaLight *area_light { nullptr };
    const class AreaLight* getAreaLight() const { return area_light; }
private:
    glm::vec3 albedo {};
};
