#pragma once

#include "material.hpp"
#include "material/microfacet_theory.hpp"

class DielectricMaterial : public Material {
public:
    DielectricMaterial(float ior, const glm::vec3 &albedo, float alpha_x = 0, float alpha_z = 0)
        : ior(ior), albedo_r(albedo), albedo_t(albedo), microfacet_theory(alpha_x, alpha_z) {}

    DielectricMaterial(float ior, const glm::vec3 &albedo_r, const glm::vec3 &albedo_t, float alpha_x = 0, float alpha_z = 0)
        : ior(ior), albedo_r(albedo_r), albedo_t(albedo_t), microfacet_theory(alpha_x, alpha_z) {}

    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const override;
    glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const override;
    bool isDeltaDistribution() const override { return (ior == 1) || microfacet_theory.isDeltaDistribution(); }
private:
    float ior;
    glm::vec3 albedo_r, albedo_t;
    MicrofacetTheory microfacet_theory;
};
