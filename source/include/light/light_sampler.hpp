#pragma once

#include "light.hpp"
#include "sample/alias_table.hpp"
#include <optional>
#include <map>

struct LightSourceSample {
    const Light light;
    float prob;
};

class LightSampler {
public:
    LightSampler() = default;

    void addLight(Light light) {
        lights.push_back(light);
    }

    void build(float scene_radius);

    std::optional<LightSourceSample> sample(float u) const;
    float getProb(Light light) const {
        auto result = light2prob.find(light.ptr());
        if (result == light2prob.end()) {
            return 0;
        }
        return result->second;
    }
private:
    std::vector<Light> lights;
    AliasTable alias_table;
    std::map<uintptr_t, float> light2prob;
};
