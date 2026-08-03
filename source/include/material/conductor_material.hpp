#pragma once

#include "material.hpp"
#include "material/microfacet_theory.hpp"

class ConductorMaterial : public Material {
public:
    ConductorMaterial(const Spectrum *eta, const Spectrum *k, float alpha_x = 0, float alpha_z = 0) : eta(eta), k(k), microfacet_theory(alpha_x, alpha_z) {}

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

    bool isDeltaDistribution() const override { return microfacet_theory.isDeltaDistribution(); }
private:
    const Spectrum *eta {}, *k {};
    MicrofacetTheory microfacet_theory;
};
