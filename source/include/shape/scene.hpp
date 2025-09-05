#pragma once

#include "shape.hpp"
#include "accelerate/scene_bvh.hpp"
#include "light/area_light.hpp"
#include "light/light_sampler.hpp"

struct Scene : public Shape {
public:
    void addShape(
        const Shape &shape,
        const Material &material = {},
        const glm::vec3 &pos = { 0, 0, 0 },
        const glm::vec3 &scale = { 1, 1, 1 },
        const glm::vec3 &rotate = { 0, 0, 0 }
    );

    template <typename MaterialType>
    void addAreaLight(const AreaLight *area_light, MaterialType *material) {
        material->area_light = area_light;
        addShape(area_light->getShape(), material);
        light_sampler.addLight(area_light);
        light_sampler_compensated.addLight(area_light);
    }

    void addInfiniteLight(const Light *infinite_light) {
        light_sampler.addLight(infinite_light);
        if (!infinite_light->impossible()) {
            light_sampler_compensated.addLight(infinite_light);
        }
        infinite_lights.push_back(infinite_light);
    }

    std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min = 1e-5,
        float t_max = std::numeric_limits<float>::infinity()
    ) const override;

    void build() {
        scene_bvh.build(std::move(instances));
        auto scene_bounds = scene_bvh.getBounds();
        radius = 0.5 * glm::distance(scene_bounds.b_max, scene_bounds.b_min);
        light_sampler.build(radius);
        light_sampler_compensated.build(radius);
    }

    const LightSampler &getLightSampler(bool allow_mis_compensation) const { return allow_mis_compensation ? light_sampler_compensated : light_sampler; }
    float getRadius() const { return radius; }
    const std::vector<const Light *> &getInfiniteLights() const { return infinite_lights; }
private:
    std::vector<ShapeInstance> instances;
    SceneBVH scene_bvh {};
    LightSampler light_sampler;
    LightSampler light_sampler_compensated;
    float radius;
    std::vector<const Light *> infinite_lights;
};
