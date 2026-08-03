#pragma once

#include "light.hpp"

class UniformInfiniteLight : public Light {
public:
    UniformInfiniteLight(const Spectrum *Le) : Le(Le) {}
    bool skipMISCompensation() const override { return true; }

    float Phi(float scene_radius) const override;

    std::optional<LightSample> sampleLight(
        const glm::vec3 &surface_point,
        float scene_radius,
        const RNG &rng,
        const WavelengthSamples &wavelength,
        bool allow_mis_compensation
    ) const override;

    SpectrumSamples getRadiance(
        const glm::vec3 &surface_point,
        const glm::vec3 &light_point,
        const glm::vec3 &normal,
        const WavelengthSamples &wavelength
    ) const override;

    float getPDF(
        const glm::vec3 &surface_point,
        const glm::vec3 &light_point,
        const glm::vec3 &normal,
        bool allow_mis_compensation
    ) const override;
private:
    const Spectrum *Le {};
};
