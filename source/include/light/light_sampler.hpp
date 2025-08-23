#pragma once

#include "light.hpp"
#include "sample/alias_table.hpp"
#include <optional>
#include <map>

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
    float getProb(const Light *light) const {
        auto result = light2prob.find(light);
        if (result == light2prob.end()) {
            return 0;
        }
        return result->second;
    }
private:
    std::vector<const Light *> lights;
    AliasTable alias_table;
    std::map<const Light *, float> light2prob;
};
