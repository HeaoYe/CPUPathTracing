#pragma once

#include "material.hpp"

class GroundMaterial : public Material {
public:
    GroundMaterial(const glm::vec3 &albedo) : albedo(albedo) {}
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const override;
private:
    glm::vec3 albedo {};
};
