#include "material/diffuse_material.hpp"
#include "sample/spherical.hpp"

glm::vec3 DiffuseMaterial::sampleBRDF(const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const {
    beta *= albedo;
    return CosineSampleHemisphere({ rng.uniform(), rng.uniform() });
}
