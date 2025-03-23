#include "material/diffuse_material.hpp"
#include "sample/spherical.hpp"

std::optional<BSDFSample> DiffuseMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const {
    glm::vec3 light_direction = CosineSampleHemisphere({ rng.uniform(), rng.uniform() });
    float pdf = CosineSampleHemispherePDF(light_direction);
    glm::vec3 bsdf = albedo / PI;
    return BSDFSample { bsdf, pdf, light_direction };
}
