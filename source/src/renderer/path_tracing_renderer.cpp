#include "renderer/path_tracing_renderer.hpp"
#include "util/frame.hpp"

enum class SampleTech {
    eLight,
    eBSDF,
};

float PowerHeuristic(SampleTech tech, const SpectrumSamples &log_q_light, const SpectrumSamples &log_q_bsdf) {
    float denom = 0;
    float max_log = glm::max(log_q_light.max(), log_q_bsdf.max());
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        denom += std::exp(2 * (log_q_light[i] - max_log)) + std::exp(2 * (log_q_bsdf[i] - max_log));
    }
    switch (tech) {
    case SampleTech::eLight:
        return std::exp(2 * (log_q_light[0] - max_log)) / denom;
    case SampleTech::eBSDF:
        return std::exp(2 * (log_q_bsdf[0] - max_log)) / denom;
    default:
        return 0;
    }
}

PixelSample PathTracingRenderer::renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) {
    thread_local RNG rng {};
    rng.setState(
        pixel_coord.x + pixel_coord.y * camera.getFilm().getWidth(),
        (pixel_coord.x + 1) * (pixel_coord.y + 1) * pixel_coord.z
    );

    WavelengthSamples wavelength = ImportanceSampleWavelength(rng.uniform());
    auto ray = camera.generateRay(pixel_coord, { rng.uniform(), rng.uniform() });
    size_t depth = 0;
    SpectrumSamples beta { 1 };
    SpectrumSamples log_q_pre { 1 };
    SpectrumSamples last_log_q_pre {};
    SpectrumSamples L {};
    bool last_is_delta = true;
    SpectrumSamples eta_scale { 1 };
    bool allow_mis_compensation = true;
    const LightSampler &light_sampler = scene.getLightSampler(allow_mis_compensation);

    while (true) {
        ++ depth;

        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
            if (hit_info->material && hit_info->material->area_light) {
                auto Le = hit_info->material->area_light->getRadiance(ray.origin, hit_info->hit_point, hit_info->normal, wavelength);
                if (depth == 1) {
                    L += Le;
                } else {
                    float weight_bsdf = 1;
                    if (last_is_delta) {
                        weight_bsdf = PowerHeuristic(SampleTech::eBSDF, SpectrumSamples(-std::numeric_limits<float>::infinity()), log_q_pre);
                    } else {
                        auto light_source_prob = light_sampler.getProb(hit_info->material->area_light, wavelength);
                        auto light_pdf = hit_info->material->area_light->getPDF(ray.origin, hit_info->hit_point, hit_info->normal, wavelength, allow_mis_compensation);
                        auto log_q_light = last_log_q_pre + Log(light_source_prob) + Log(light_pdf);
                        weight_bsdf = PowerHeuristic(SampleTech::eBSDF, log_q_light, log_q_pre);
                    }
                    L += weight_bsdf * beta * Le * g_wavelength_sample_count;
                }
            }

            if (depth > 3) {
                SpectrumSamples q {};
                for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
                    auto beta_q = beta * std::exp(log_q_pre[0] - log_q_pre[i]) * eta_scale;
                    q[i] = glm::min(beta_q.max(), 0.9f);
                }
                if (rng.uniform() >= q[0]) {
                    break;
                }
                beta /= q[0];
                log_q_pre += Log(q);
            }

            Frame frame(hit_info->normal);
            glm::vec3 light_direction;
            if (hit_info->material) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                if (view_direction.y == 0) {
                    ray.origin = hit_info->hit_point;
                    continue;
                }

                last_is_delta = hit_info->material->isDeltaDistribution();
                if (!last_is_delta) {
                    auto light_source_sample = light_sampler.sample(rng.uniform(), wavelength);
                    if (light_source_sample.has_value()) {
                        auto light_sample = light_source_sample->light->sampleLight(hit_info->hit_point, scene.getRadius(), rng, wavelength, allow_mis_compensation);
                        if (light_sample.has_value() && (!scene.intersect({ hit_info->hit_point, light_sample->light_point - hit_info->hit_point }, 1e-5, 1.f - 1e-5))) {
                            glm::vec3 light_direction_local = frame.localFromWorld(light_sample->light_direction);
                            auto bsdf_pdf = hit_info->material->PDF(hit_info->hit_point, light_direction_local, view_direction, wavelength);
                            auto log_q_light = log_q_pre + Log(light_sample->pdf) + Log(light_source_sample->prob);
                            float weight_light = PowerHeuristic(SampleTech::eLight, log_q_light, log_q_pre + Log(bsdf_pdf));
                            L += weight_light * beta * hit_info->material->BSDF(hit_info->hit_point, light_direction_local, view_direction, wavelength)
                                * glm::abs(light_direction_local.y) * light_sample->Le / (light_sample->pdf[0] * light_source_sample->prob[0] / g_wavelength_sample_count);
                        }
                    }
                }

                auto bsdf_sample = hit_info->material->sampleBSDF(hit_info->hit_point, view_direction, rng, wavelength);
                if (!bsdf_sample.has_value()) {
                    break;
                }
                last_log_q_pre = log_q_pre;
                eta_scale *= bsdf_sample->eta_scale;
                beta *= bsdf_sample->bsdf * glm::abs(bsdf_sample->light_direction.y) / bsdf_sample->pdf[0];
                log_q_pre += Log(bsdf_sample->pdf);
                light_direction = bsdf_sample->light_direction;
            } else {
                break;
            }

            ray.origin = hit_info->hit_point;
            ray.direction = frame.worldFromLocal(light_direction);
        } else {
            glm::vec3 light_direction = glm::normalize(ray.direction);
            glm::vec3 light_point = ray.origin + scene.getRadius() * 2 * light_direction;
            if (depth == 1) {
                for (const auto *infinite_light : scene.getInfiniteLights()) {
                    L += infinite_light->getRadiance(ray.origin, light_point, -light_direction, wavelength);
                }
            } else {
                if (last_is_delta) {
                    for (const auto *infinite_light : scene.getInfiniteLights()) {
                        float weight_bsdf = PowerHeuristic(SampleTech::eBSDF, SpectrumSamples(-std::numeric_limits<float>::infinity()), log_q_pre);
                        L += weight_bsdf * beta * infinite_light->getRadiance(ray.origin, light_point, -light_direction, wavelength) * g_wavelength_sample_count;
                    }
                } else {
                    for (const auto *infinite_light : scene.getInfiniteLights()) {
                        auto light_source_prob = light_sampler.getProb(infinite_light, wavelength);
                        auto light_pdf = infinite_light->getPDF(ray.origin, light_point, -light_direction, wavelength, allow_mis_compensation);
                        auto log_q_light = last_log_q_pre + Log(light_source_prob) + Log(light_pdf);
                        float weight_bsdf = PowerHeuristic(SampleTech::eBSDF, log_q_light, log_q_pre);
                        L += weight_bsdf * beta * infinite_light->getRadiance(ray.origin, light_point, -light_direction, wavelength) * g_wavelength_sample_count;
                    }
                }
            }
            break;
        }
    }

    return { L, wavelength };
}
