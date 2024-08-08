#pragma once

#include <glm/glm.hpp>

struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 hit(float t) const { return origin + t * direction; }

    Ray objectFromWorld(const glm::mat4 &object_from_world) const;
};

struct HitInfo {
    float t;
    glm::vec3 hit_point;
    glm::vec3 normal;
};
