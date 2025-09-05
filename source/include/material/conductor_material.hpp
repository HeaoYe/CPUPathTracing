#pragma once

#include "material/microfacet_theory.hpp"
#include "bsdf_sample.hpp"
#include "util/rng.hpp"
#include <optional>

class ConductorMaterial {
public:
    ConductorMaterial(const glm::vec3 &ior, const glm::vec3 &k, float alpha_x = 0, float alpha_z = 0) : ior(ior), k(k), microfacet_theory(alpha_x, alpha_z) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const;
    glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const;
    float PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const;
    bool isDeltaDistribution() const { return microfacet_theory.isDeltaDistribution(); }
public:
    const class AreaLight *area_light { nullptr };
    const class AreaLight* getAreaLight() const { return area_light; }
private:
    glm::vec3 ior, k;
    MicrofacetTheory microfacet_theory;
};
