#pragma once

#include "light.hpp"
#include "sample/alias_table.hpp"
#include <optional>

struct LightSourceSample {
    const Light *light;
    float prob;
};

class LightSampler {
public:
    LightSampler() = default;

    void addLight(const Light *light) {
        lights.push_back(light);
    }

    void build(float scene_radius);

    std::optional<LightSourceSample> sample(float u) const;
private:
    std::vector<const Light *> lights;
    AliasTable alias_table;
};
