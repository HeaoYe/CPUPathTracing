#pragma once

#include <glm/glm.hpp>

class RGB {
public:
    RGB(int r, int g, int b) : r(r), g(g), b(b) {}

    RGB(const glm::vec3 &color) {
        r = glm::clamp<int>(glm::pow(color.x, 1.0 / 2.2) * 255, 0, 255);
        g = glm::clamp<int>(glm::pow(color.y, 1.0 / 2.2) * 255, 0, 255);
        b = glm::clamp<int>(glm::pow(color.z, 1.0 / 2.2) * 255, 0, 255);
    }

    operator glm::vec3() const  {
        return glm::vec3 {
            glm::pow(r / 255.f, 2.2),
            glm::pow(g / 255.f, 2.2),
            glm::pow(b / 255.f, 2.2),
        };
    }
public:
    int r, g, b;
};
