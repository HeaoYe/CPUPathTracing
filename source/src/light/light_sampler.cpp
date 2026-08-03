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
}

std::optional<LightSourceSample> LightSampler::sample(float u, const WavelengthSamples &wavelength) const {
    if (lights.empty()) {
        return {};
    }
    auto sample_result = alias_table.sample(u);
    return LightSourceSample { lights[sample_result.index], SpectrumSamples(sample_result.prob) };
}
