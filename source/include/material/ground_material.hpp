#pragma once

#include "material.hpp"

class GroundMaterial : public Material {
public:
    GroundMaterial(const Spectrum *albedo) : albedo(albedo) {}

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

    bool isDeltaDistribution() const override { return false; }
private:
    const Spectrum *albedo {};
};
