#include "material/ground_material.hpp"
#include "sample/spherical.hpp"

std::optional<BSDFSample> GroundMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const {
    if (view_direction.y == 0) {
        return {};
    }
    glm::vec3 light_direction = CosineSampleHemisphere({ rng.uniform(), rng.uniform() });
    float pdf = CosineSampleHemispherePDF(light_direction);
    glm::vec3 bsdf = albedo / PI;
    if (
        (static_cast<int>(glm::floor(hit_point.x * 8 + 0.5)) % 8 == 0) ||
        (static_cast<int>(glm::floor(hit_point.z * 8 + 0.5)) % 8 == 0)
    ) {
        bsdf *= 0.1;
    }
    return BSDFSample { bsdf, pdf, light_direction * glm::sign(view_direction.y) };
}

glm::vec3 GroundMaterial::BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
    if (light_direction.y * view_direction.y <= 0) {
        return {};
    }
    glm::vec3 bsdf = albedo / PI;
    if (
        (static_cast<int>(glm::floor(hit_point.x * 8 + 0.5)) % 8 == 0) ||
        (static_cast<int>(glm::floor(hit_point.z * 8 + 0.5)) % 8 == 0)
    ) {
        bsdf *= 0.1;
    }
    return bsdf;
}

float GroundMaterial::PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
    if (light_direction.y * view_direction.y <= 0) {
        return 0;
    }
    return CosineSampleHemispherePDF(light_direction);
}
