#pragma once

// IWYU pragma: begin_exports
#include "camera/camera.hpp"
#include "shape/scene.hpp"
#include "thread/thread_pool.hpp"
#include "util/progress.hpp"
#include "util/profile.hpp"
#include <iostream>
#include <string>
// IWYU pragma: end_exports

#define DEFINE_RENDERER(Name) \
    class Name##Renderer { \
    public: \
        Name##Renderer(Camera &camera, const Scene &scene) : camera(camera), scene(scene) {} \
        glm::vec3 renderPixel(const glm::ivec3 &pixel_coord) ; \
        void render(size_t spp, const std::filesystem::path &filename) { \
            PROFILE("Render " + std::to_string(spp) + "spp " + filename.string()); \
            size_t current_spp = 0, increase = 1; \
            auto &film = camera.getFilm(); \
            film.clear(); \
            Progress progress(film.getWidth() * film.getHeight() * spp, 20); \
            while (current_spp < spp) { \
                thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) { \
                    for (int i = 0; i < increase; i ++) { \
                        film.addSample(x, y, renderPixel({ x, y, current_spp + i })); \
                    } \
                    progress.update(increase); \
                }); \
                thread_pool.wait(); \
                current_spp += increase; \
                increase = std::min<size_t>(current_spp, 32); \
                film.save(filename); \
                std::cout << current_spp << "spp has been saved to " << filename << std::endl; \
            } \
        } \
        Camera *getCamera() { return &camera; } \
        const Scene *getScene() const { return &scene; } \
    private: \
        Camera &camera; \
        const Scene &scene; \
    }; \
