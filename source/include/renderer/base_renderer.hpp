#pragma once

#include "camera/camera.hpp"
#include "shape/scene.hpp"

#define DEFINE_RENDERER(Name) \
    class Name##Renderer : public BaseRenderer { \
    public: \
        Name##Renderer(Camera &camera, const Scene &scene) : BaseRenderer(camera, scene) {} \
    private: \
        PixelSample renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) override; \
    }; \

class BaseRenderer {
    friend class Previewer;
public:
    BaseRenderer(Camera &camera, const Scene &scene) : camera(camera), scene(scene) {}
    void render(size_t spp, const std::filesystem::path &filename, const ColorSpace *target_color_space);
private:
    virtual PixelSample renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) = 0;
protected:
    Camera &camera;
    const Scene &scene;
};
