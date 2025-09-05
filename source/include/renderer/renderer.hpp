#pragma once

#include "util/generalized_ptr.hpp"

#include "debug_renderer.hpp"
#include "normal_renderer.hpp"
#include "path_tracing_renderer.hpp"
#include "simple_path_tracing_renderer.hpp"

class Renderer : public GeneralizedPtr<BoundsTestCountRenderer, TriangleTestCountRenderer, NormalRenderer, PathTracingRenderer, SimplePathTracingRenderer> {
public:
    void render(size_t spp, const std::filesystem::path &filename) {
        return DISPATCH(render, spp, filename);
    }

    glm::vec3 renderPixel(const glm::ivec3 &pixel_coord) {
        return DISPATCH(renderPixel, pixel_coord);
    }

    Camera &getCamera() {
        return * DISPATCH(getCamera);
    }

    const Scene &getScene() const {
        return * DISPATCH_CONST(getScene);
    }
};
