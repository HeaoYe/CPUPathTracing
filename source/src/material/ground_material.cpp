#include "material/ground_material.hpp"
#include "sample/spherical.hpp"

glm::vec3 GroundMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const {
    beta *= albedo;
    if (
        (static_cast<int>(glm::floor(hit_point.x * 8)) % 8 == 0) ||
        (static_cast<int>(glm::floor(hit_point.z * 8)) % 8 == 0)
    ) {
        beta *= 0.1;
    }
    return CosineSampleHemisphere({ rng.uniform(), rng.uniform() });
}
