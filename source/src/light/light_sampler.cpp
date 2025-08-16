#include "light/light_sampler.hpp"

void LightSampler::build(float scene_radius) {
    std::vector<float> phis;
    phis.reserve(lights.size());
    for (const auto *light : lights) {
        phis.push_back(light->Phi(scene_radius));
    }
    alias_table.build(phis);
    const auto &probs = alias_table.getProbs();
    for (size_t i = 0; i < lights.size(); i ++) {
        light2prob.insert(std::make_pair(lights[i], probs[i]));
    }

    phis.clear();
    for (const auto *light : lights_mis_compensation) {
        phis.push_back(light->Phi(scene_radius));
    }
    alias_table_mis_compensation.build(phis);
    const auto &probs_mis_compensation = alias_table_mis_compensation.getProbs();
    for (size_t i = 0; i < lights_mis_compensation.size(); i ++) {
        light2prob_mis_compensation.insert(std::make_pair(lights_mis_compensation[i], probs_mis_compensation[i]));
    }
}

std::optional<LightSourceSample> LightSampler::sample(float u, bool allow_mis_compensation) const {
    if (allow_mis_compensation) {
        if (lights_mis_compensation.empty()) {
            return {};
        }
    auto sample_result = alias_table_mis_compensation.sample(u);
        return LightSourceSample { lights_mis_compensation[sample_result.index], sample_result.prob };
    }
    if (lights.empty()) {
        return {};
    }
    auto sample_result = alias_table.sample(u);
    return LightSourceSample { lights[sample_result.index], sample_result.prob };
}
