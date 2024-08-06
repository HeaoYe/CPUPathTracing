#include <iostream>
#include "thread_pool.hpp"
#include "film.hpp"
#include "camera.hpp"
#include "sphere.hpp"

int main() {
    ThreadPool thread_pool {};
    Film film { 1920, 1080 };
    Camera camera { film, { 0, 0, 0 }, { 0, 0, 1 }, 90 };

    Sphere sphere {
        { 0, 0, 2 },
        1.3f,
    };

    glm::vec3 light_pos { 3, 4, -3 };

    thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        auto ray = camera.generateRay({ x, y });
        auto hit_t = sphere.intersect(ray);
        if (hit_t.has_value()) {
            glm::vec3 hit_point = ray.origin + ray.direction * hit_t.value();
            glm::vec3 normal = glm::normalize(hit_point - sphere.center);
            glm::vec3 L = glm::normalize(light_pos - hit_point);
            float color = glm::max(glm::dot(normal, L), 0.f);
            film.setPixel(x, y, { color, color, color });
        }
    });

    thread_pool.wait();
    film.save("test.ppm");
    return 0;
}
