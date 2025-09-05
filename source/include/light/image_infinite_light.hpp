#pragma once

#include "light/light_sample.hpp"
#include "util/rng.hpp"
#include "image/image.hpp"
#include "sample/alias_table.hpp"
#include <optional>

class ImageInfiniteLight {
public:
    ImageInfiniteLight(const Image *image, float start_phi = 0);
    bool impossible() const { return impossible_compensated; }

    float Phi(float scene_radius) const { return precompute_phi * scene_radius * scene_radius; }
    std::optional<LightSample> sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const;

    glm::vec3 getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const;
    float getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const;
private:
    glm::vec2 imagePointFromDirection(const glm::vec3 &direction) const;
    glm::vec3 directionFromImagePoint(const glm::vec2 &image_point) const;
    glm::ivec2 girdIdxFromImagePoint(const glm::vec2 &image_point) const;
private:
    const Image *image;
    bool impossible_compensated;
    float start_phi;
    float precompute_phi;
    AliasTable alias_table, alias_table_compensated;
    glm::ivec2 gird_count;
    static constexpr size_t gird_side_length = 50;
};
