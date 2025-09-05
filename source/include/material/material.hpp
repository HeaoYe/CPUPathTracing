#pragma once

#include "util/rng.hpp"
#include "util/generalized_ptr.hpp"

#include "material/diffuse_material.hpp"
#include "material/ground_material.hpp"
#include "material/specular_material.hpp"
#include "material/conductor_material.hpp"
#include "material/dielectric_material.hpp"

#include <glm/glm.hpp>
#include <optional>

class Material : public GeneralizedPtr<DiffuseMaterial, GroundMaterial, SpecularMaterial, ConductorMaterial, DielectricMaterial> {
public:
    std::optional<BSDFSample> sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const {
        return DISPATCH_CONST(sampleBSDF, hit_point, view_direction, rng);
    }

    glm::vec3 BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
        return DISPATCH_CONST(BSDF, hit_point, light_direction, view_direction);
    }

    float PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
        return DISPATCH_CONST(PDF, hit_point, light_direction, view_direction);
    }

    bool isDeltaDistribution() const {
        return DISPATCH_CONST(isDeltaDistribution);
    }

    const class AreaLight* getAreaLight() const {
        return DISPATCH_CONST(getAreaLight);
    }
};
