#pragma once

#include "material.hpp"
#include "material/microfacet_theory.hpp"

class DielectricMaterial : public Material {
public:
    DielectricMaterial(const Spectrum *ior, const Spectrum *albedo, float alpha_x = 0, float alpha_z = 0)
        : ior(ior), albedo_r(albedo), albedo_t(albedo), microfacet_theory(alpha_x, alpha_z) {}

    DielectricMaterial(const Spectrum *ior, const Spectrum *albedo_r, const Spectrum *albedo_t, float alpha_x = 0, float alpha_z = 0)
        : ior(ior), albedo_r(albedo_r), albedo_t(albedo_t), microfacet_theory(alpha_x, alpha_z) {}


    std::optional<BSDFSample> sampleBSDF(
        const glm::vec3 &hit_point,
        const glm::vec3 &view_direction,
        const RNG &rng,
        const WavelengthSamples &wavelength
    ) const override;

    SpectrumSamples BSDF(
        const glm::vec3 &hit_point,
        const glm::vec3 &light_direction,
        const glm::vec3 &view_direction,
        const WavelengthSamples &wavelength
    ) const override;

    SpectrumSamples PDF(
        const glm::vec3 &hit_point,
        const glm::vec3 &light_direction,
        const glm::vec3 &view_direction,
        const WavelengthSamples &wavelength
    ) const override;

    bool isDeltaDistribution() const override { return (ior->isConstant() && (ior->max() == 1)) || microfacet_theory.isDeltaDistribution(); }
private:
    const Spectrum *ior {};
    const Spectrum *albedo_r {}, *albedo_t {};
    MicrofacetTheory microfacet_theory;
};
