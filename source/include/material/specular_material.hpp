#pragma once

#include "material.hpp"

class SpecularMaterial : public Material {
public:
    SpecularMaterial(const glm::vec3 &albedo) : albedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const override;
    glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const override { return {}; }
    float PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const override { return 0; }
    bool isDeltaDistribution() const override { return true; }
private:
    glm::vec3 albedo {};
};
