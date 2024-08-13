#pragma once

#include <glm/glm.hpp>

class Frame {
public:
    Frame(const glm::vec3 &normal);

    glm::vec3 localFromWorld(const glm::vec3 &direction_world) const;
    glm::vec3 worldFromLocal(const glm::vec3 &direction_local) const;
private:
    glm::vec3 x_axis, y_axis, z_axis;
};
