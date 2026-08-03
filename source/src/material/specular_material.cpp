#include "material/specular_material.hpp"

std::optional<BSDFSample> SpecularMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng, const WavelengthSamples &wavelength) const {
    glm::vec3 light_direction { -view_direction.x, view_direction.y, -view_direction.z };
    auto bsdf = albedo->sample(wavelength) / glm::abs(light_direction.y);
    float pdf = 1;
    return BSDFSample { bsdf, pdf, light_direction };
}
