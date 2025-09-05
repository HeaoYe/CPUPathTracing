#pragma once

#include "bsdf_sample.hpp"
#include "util/rng.hpp"
#include <optional>

class DiffuseMaterial {
public:
    DiffuseMaterial(const glm::vec3 &albedo = { 1, 1, 1 }) : albedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const;
    glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const;
    float PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const;
    bool isDeltaDistribution() const { return false; }
public:
    const class AreaLight *area_light { nullptr };
    const class AreaLight* getAreaLight() const { return area_light; }
private:
    glm::vec3 albedo {};
};
