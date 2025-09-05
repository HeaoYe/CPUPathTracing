#pragma once

#include <glm/glm.hpp>

struct LightSample {
    glm::vec3 light_point;
    glm::vec3 light_direction;
    glm::vec3 Le;
    float pdf;
};
