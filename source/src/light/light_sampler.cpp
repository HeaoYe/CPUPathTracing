#include "light/light_sampler.hpp"

void LightSampler::build(float scene_radius) {
    std::vector<float> phis;
    phis.reserve(lights.size());
    for (const auto *light : lights) {
        phis.push_back(light->Phi(scene_radius));
    }
    alias_table.build(phis);
}

std::optional<LightSourceSample> LightSampler::sample(float u) const {
    if (lights.empty()) {
        return {};
    }
    auto sample_result = alias_table.sample(u);
    return LightSourceSample { lights[sample_result.index], sample_result.prob };
}
