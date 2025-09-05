#pragma once

#include "glm/glm.hpp"

struct ShapeSample {
    glm::vec3 point;
    glm::vec3 normal;
    float pdf;
};
