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
        if (light->getType() != LightType::eInfiniteLight) {
            lights_mis_compensation.push_back(light);
        }
    }

    void build(float scene_radius);

    std::optional<LightSourceSample> sample(float u, bool allow_mis_compensation) const;
    float getProb(const Light *light, bool allow_mis_compensation) const {
        if (allow_mis_compensation) {
            if (light->getType() == LightType::eInfiniteLight) {
                return 0;
            }
            return light2prob_mis_compensation.at(light);
        }
        return light2prob.at(light);
    }
private:
    std::vector<const Light *> lights;
    AliasTable alias_table;
    std::map<const Light *, float> light2prob;

    std::vector<const Light *> lights_mis_compensation;
    AliasTable alias_table_mis_compensation;
    std::map<const Light *, float> light2prob_mis_compensation;
};
