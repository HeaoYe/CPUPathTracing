#include "light/infinite_light.hpp"
#include "sample/spherical.hpp"

float InfiniteLight::Phi(float scene_radius) const {
    return 4 * PI * PI * scene_radius * scene_radius * glm::max(Le.r, glm::max(Le.g, Le.b));
}

std::optional<LightSample> InfiniteLight::sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng) const {
    glm::vec3 light_direction = UniformSampleSphere(rng);
    return LightSample {
        surface_point + 2.f * scene_radius * light_direction,
        light_direction,
        Le,
        1.f / (4.f * PI)
    };
}
