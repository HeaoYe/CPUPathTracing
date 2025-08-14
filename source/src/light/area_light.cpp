#include "light/area_light.hpp"
#include "sample/spherical.hpp"

float AreaLight::Phi(float scene_radius) const {
    return (double_side ? 2 : 1) * PI * shape.getArea() * glm::max(Le.r, glm::max(Le.g, Le.b));
}

std::optional<LightSample> AreaLight::sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng) const {
    auto shape_sample = shape.sampleShape(rng);
    if (!shape_sample.has_value()) {
        return {};
    }
    glm::vec3 light_direction_raw = shape_sample->point - surface_point;
    glm::vec3 light_direction = glm::normalize(light_direction_raw);
    float cos_theta = glm::dot(shape_sample->normal, -light_direction);
    if ((!double_side) && cos_theta <= 0) {
        return {};
    }
    float det_J = cos_theta / glm::dot(light_direction_raw, light_direction_raw);

    return LightSample { shape_sample->point, light_direction, Le, shape_sample->pdf / det_J };
}

glm::vec3 AreaLight::getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const {
    if ((!double_side) && glm::dot(surface_point - light_point, normal) <= 0) {
        return {};
    }
    return Le;
}
