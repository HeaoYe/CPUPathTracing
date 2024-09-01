#pragma once

#include "material.hpp"

class ConductorMaterial : public Material {
public:
    ConductorMaterial(const glm::vec3 &ior, const glm::vec3 &k) : ior(ior), k(k) {}
    glm::vec3 sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const override;
private:
    glm::vec3 ior, k;
};
