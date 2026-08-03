#include "material/diffuse_material.hpp"
#include "sample/spherical.hpp"

std::optional<BSDFSample> DiffuseMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng, const WavelengthSamples &wavelength) const {
    if (view_direction.y == 0) {
        return {};
    }
    glm::vec3 light_direction = CosineSampleHemisphere({ rng.uniform(), rng.uniform() });
    float pdf = CosineSampleHemispherePDF(light_direction);
    auto bsdf = albedo->sample(wavelength) / PI;
    return BSDFSample { bsdf, pdf, light_direction * glm::sign(view_direction.y) };
}

SpectrumSamples DiffuseMaterial::BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction, const WavelengthSamples &wavelength) const {
    if (light_direction.y * view_direction.y <= 0) {
        return {};
    }
    return albedo->sample(wavelength) / PI;
}

float DiffuseMaterial::PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction, const WavelengthSamples &wavelength) const {
    if (light_direction.y * view_direction.y <= 0) {
        return 0;
    }
    return CosineSampleHemispherePDF(light_direction);
}
