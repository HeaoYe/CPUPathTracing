#include "thread_pool.hpp"
#include "film.hpp"
#include "camera.hpp"
#include "sphere.hpp"
#include "model.hpp"
#include "plane.hpp"
#include "scene.hpp"
#include "frame.hpp"
#include "rgb.hpp"

#include <random>
#include <iostream>

int main() {
    ThreadPool thread_pool {};

    Film film { 192 * 4, 108 * 4 };
    Camera camera { film, { -3.6, 0, 0 }, { 0, 0, 0 }, 45 };
    std::atomic<int> count = 0;

    Model model("models/simple_dragon.obj");
    Sphere sphere {
        { 0, 0, 0 },
        1
    };
    Plane plane {
        { 0, 0, 0 },
        { 0, 1, 0 }
    };

    Scene scene {};
    scene.addShape(
        model,
        { RGB(202, 159, 117) },
        { 0, 0, 0 },
        { 1, 3, 2 }
    );
    scene.addShape(
        sphere,
        { { 1, 1, 1 }, false, RGB(255, 128, 128) },
        { 0, 0, 2.5 }
    );
    scene.addShape(
        sphere,
        { { 1, 1, 1 }, false, RGB(128, 128, 255) },
        { 0, 0, -2.5 }
    );
    scene.addShape(
        sphere,
        { { 1, 1, 1 }, true },
        { 3, 0.5, -2 }
    );
    scene.addShape(plane, { RGB(120, 204, 157) }, { 0, -0.5, 0 });

    std::mt19937 gen(23451334);
    std::uniform_real_distribution<float> uniform(-1, 1);
    int spp = 128;

    thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        for (int i = 0; i < spp; i ++) {
            auto ray = camera.generateRay({ x, y }, { abs(uniform(gen)), abs(uniform(gen)) });
            glm::vec3 beta = { 1, 1, 1 };
            glm::vec3 color = { 0, 0, 0 };

            while (true) {
                auto hit_info = scene.intersect(ray);
                if (hit_info.has_value()) {
                    color += beta * hit_info->material->emissive;
                    beta *= hit_info->material->albedo;

                    ray.origin = hit_info->hit_point;

                    Frame frame(hit_info->normal);
                    glm::vec3 light_direction;
                    if (hit_info->material->is_specular) {
                        glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                        light_direction = { -view_direction.x, view_direction.y, -view_direction.z };
                    } else {
                        do {
                            light_direction = { uniform(gen), uniform(gen), uniform(gen) };
                        } while(glm::length(light_direction) > 1);
                        if (light_direction.y < 0) {
                            light_direction.y = -light_direction.y;
                        }
                    }
                    ray.direction = frame.worldFromLocal(light_direction);
                } else {
                    break;
                }
            }

            film.addSample(x, y, color);
        }

        int n = ++count;
        if (n % film.getWidth() == 0) {
            std::cout << static_cast<float>(n) / (film.getHeight() * film.getWidth()) << std::endl;
        }
    });

    thread_pool.wait();
    film.save("test.ppm");
    return 0;
}
