#pragma once

#include "material.hpp"

class DielectricMaterial : public Material {
public:
    DielectricMaterial(float ior, const glm::vec3 &albedo)
        : ior(ior), albedo_r(albedo), albedo_t(albedo) {}

    DielectricMaterial(float ior, const glm::vec3 &albedo_r, const glm::vec3 &albedo_t)
        : ior(ior), albedo_r(albedo_r), albedo_t(albedo_t) {}

    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const override;
private:
    float ior;
    glm::vec3 albedo_r, albedo_t;
};
