#include "camera/ray.hpp"

Ray Ray::objectFromWorld(const glm::mat4 &object_from_world) const {
    glm::vec3 o = object_from_world * glm::vec4(origin, 1.f);
    glm::vec3 d = object_from_world * glm::vec4(direction, 0.f);
    return Ray { o, d };
}
