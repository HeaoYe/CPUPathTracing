#pragma once

#include "util/rng.hpp"
#include <glm/glm.hpp>

constexpr float PI = 3.14159265359;

inline glm::vec2 UniformSampleUnitDisk(const glm::vec2 &u) {
    float r = glm::sqrt(u.x);
    float theta = 2 * PI * u.y;
    return { r * glm::cos(theta), r * glm::sin(theta) };
}

inline glm::vec3 CosineSampleHemisphere(const glm::vec2 &u) {
    float r = glm::sqrt(u.x);
    float phi = 2 * PI * u.y;
    return { r * glm::cos(phi), glm::sqrt(1 - r * r), r * glm::sin(phi) };
}

inline glm::vec3 UniformSampleHemisphere(const RNG &rng) {
    glm::vec3 result;
    do {
        result = { rng.uniform(), rng.uniform(), rng.uniform() };
        result = result * 2.f - 1.f;
    } while(glm::length(result) > 1);
    if (result.y < 0) {
        result.y = -result.y;
    }
    return glm::normalize(result);
}
