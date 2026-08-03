#include "light/uniform_infinite_light.hpp"
#include "sample/spherical.hpp"

float UniformInfiniteLight::Phi(float scene_radius) const {
    return 4 * PI * PI * scene_radius * scene_radius * Le->max();
}

std::optional<LightSample> UniformInfiniteLight::sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, const WavelengthSamples &wavelength, bool allow_mis_compensation) const {
    if (allow_mis_compensation) {
        return {};
    }
    glm::vec3 light_direction = UniformSampleSphere(rng);
    return LightSample {
        surface_point + 2.f * scene_radius * light_direction,
        light_direction,
        Le->sample(wavelength),
        1.f / (4.f * PI)
    };
}

SpectrumSamples UniformInfiniteLight::getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, const WavelengthSamples &wavelength) const {
    return Le->sample(wavelength);
}

float UniformInfiniteLight::getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const {
    if (allow_mis_compensation) {
        return 0;
    }
    return 1.f / (4.f * PI);
}
