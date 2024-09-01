#include "material/specular_material.hpp"

glm::vec3 SpecularMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const {
    beta *= albedo;
    return glm::vec3 { -view_direction.x, view_direction.y, -view_direction.z };
}
