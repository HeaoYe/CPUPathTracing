#pragma once

#include "light_sample.hpp"
#include "util/rng.hpp"
#include "util/generalized_ptr.hpp"

#include "area_light.hpp"
#include "uniform_infinite_light.hpp"
#include "image_infinite_light.hpp"

#include <glm/glm.hpp>
#include <optional>

class Light : public GeneralizedPtr<AreaLight, UniformInfiniteLight, ImageInfiniteLight> {
public:
    bool impossible() const {
        return DISPATCH_CONST(impossible);
    }

    float Phi(float scene_radius) const {
        return DISPATCH_CONST(Phi, scene_radius);
    }

    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const {
        return DISPATCH_CONST(sampleLight, surface_point, scene_radius, rng, allow_mis_compensation);
    }

    glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const {
        return DISPATCH_CONST(getRadiance, surface_point, light_point, normal);
    }

    float getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const {
        return DISPATCH_CONST(getPDF, surface_point, light_point, normal, allow_mis_compensation);
    }
};
