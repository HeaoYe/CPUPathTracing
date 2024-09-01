#pragma once

#include "material.hpp"

class SpecularMaterial : public Material {
public:
    SpecularMaterial(const glm::vec3 &albedo) : albedo(albedo) {}
    glm::vec3 sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const override;
private:
    glm::vec3 albedo {};
};
