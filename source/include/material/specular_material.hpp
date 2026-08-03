#pragma once

#include "material.hpp"

class SpecularMaterial : public Material {
public:
    SpecularMaterial(const Spectrum *albedo) : albedo(albedo) {}

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

    float PDF(
        const glm::vec3 &hit_point,
        const glm::vec3 &light_direction,
        const glm::vec3 &view_direction,
        const WavelengthSamples &wavelength
    ) const override;

    bool isDeltaDistribution() const override { return true; }
private:
    const Spectrum *albedo {};
};
