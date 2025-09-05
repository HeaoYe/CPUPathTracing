#pragma once

#include <glm/glm.hpp>

struct BSDFSample {
    glm::vec3 bsdf;
    float pdf;
    glm::vec3 light_direction;
    float eta_scale { 1.f };
};
