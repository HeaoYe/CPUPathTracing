#include "thread_pool.hpp"
#include "film.hpp"
#include "camera.hpp"
#include "sphere.hpp"
#include "model.hpp"

#include <iostream>

int main() {
    ThreadPool thread_pool {};

    Film film { 1920, 1080 };
    Camera camera { film, { -0.6, 0, 0 }, { 0, 0, 0 }, 90 };
    std::atomic<int> count = 0;

    Model model("models/simple_dragon.obj");
    Sphere sphere {
        { 0, 0, 0 },
        0.5f
    };
    Shape &shape = model;

    glm::vec3 light_pos { -1, 2, 1 };

    thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        auto ray = camera.generateRay({ x, y });
        auto hit_info = shape.intersect(ray);
        if (hit_info.has_value()) {
            auto L = glm::normalize(light_pos - hit_info->hit_point);
            float cosine = glm::max(0.f, glm::dot(hit_info->normal, L));

            film.setPixel(x, y, { cosine, cosine, cosine });
        }

        count ++;
        if (count % film.getWidth() == 0) {
            std::cout << static_cast<float>(count) / (film.getHeight() * film.getWidth()) << std::endl;
        }
    });

    thread_pool.wait();
    film.save("test.ppm");
    return 0;
}
