#pragma once

#include "material/material.hpp"
#include "util/debug_macro.hpp"
#include <glm/glm.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 hit(float t) const { return origin + t * direction; }

    Ray objectFromWorld(const glm::mat4 &object_from_world) const;

    DEBUG_LINE(mutable size_t bounds_test_count = 0)
    DEBUG_LINE(mutable size_t triangle_test_count = 0)
};

struct HitInfo {
    float t;
    glm::vec3 hit_point;
    glm::vec3 normal;
    const Material *material = nullptr;
};
