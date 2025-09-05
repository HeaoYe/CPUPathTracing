#include "renderer/path_tracing_renderer.hpp"
#include "util/frame.hpp"
#include "util/rng.hpp"

float PowerHeuristic(float pdf_j, float pdf_k) {
    return pdf_j * pdf_j / (pdf_j * pdf_j + pdf_k * pdf_k);
}

glm::vec3 PathTracingRenderer::renderPixel(const glm::ivec3 &pixel_coord) {
    thread_local RNG rng {};
    rng.setSeed(pixel_coord.x + pixel_coord.y * 10000 + pixel_coord.z * 10000 * 10000);

    auto ray = camera.generateRay(pixel_coord, { rng.uniform(), rng.uniform() });
    glm::vec3 beta = { 1, 1, 1 };
    glm::vec3 L = { 0, 0, 0 };
    bool last_is_specular = true;
    float last_bsdf_pdf = 0;
    float eta_scale = 1.f;
    bool allow_mis_compensation = true;
    const LightSampler &light_sampler = scene.getLightSampler(allow_mis_compensation);

    while (true) {
        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
            if (hit_info->material.isValid() && hit_info->material.getAreaLight()) {
                float weight_bsdf = 1;
                if (!last_is_specular) {
                    float light_source_prob = light_sampler.getProb({ hit_info->material.getAreaLight() });
                    float light_pdf = hit_info->material.getAreaLight()->getPDF(ray.origin, hit_info->hit_point, hit_info->normal, allow_mis_compensation);
                    weight_bsdf = PowerHeuristic(last_bsdf_pdf, light_source_prob * light_pdf);
                }
                L += weight_bsdf * beta * hit_info->material.getAreaLight()->getRadiance(ray.origin, hit_info->hit_point, hit_info->normal);
            }

            glm::vec3 beta_q = beta * eta_scale;
            float q = glm::max(beta_q.r, glm::max(beta_q.g, beta_q.b));
            q = glm::min(q, 0.9f);
            if (q < 1) {
                if (rng.uniform() > q) {
                    break;
                }
                beta /= q;
            }

            Frame frame(hit_info->normal);
            glm::vec3 light_direction;
            if (hit_info->material.isValid()) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                if (view_direction.y == 0) {
                    ray.origin = hit_info->hit_point;
                    continue;
                }

                last_is_specular = hit_info->material.isDeltaDistribution();
                if (!last_is_specular) {
                    auto light_source_sample = light_sampler.sample(rng.uniform());
                    if (light_source_sample.has_value()) {
                        auto light_sample = light_source_sample->light.sampleLight(hit_info->hit_point, scene.getRadius(), rng, allow_mis_compensation);
                        if (light_sample.has_value() && (!scene.intersect({ hit_info->hit_point, light_sample->light_point - hit_info->hit_point }, 1e-5, 1.f - 1e-5))) {
                            glm::vec3 light_direction_local = frame.localFromWorld(light_sample->light_direction);
                            float bsdf_pdf = hit_info->material.PDF(hit_info->hit_point, light_direction_local, view_direction);
                            float weight_light = PowerHeuristic(light_sample->pdf * light_source_sample->prob, bsdf_pdf);
                            L += weight_light * beta * hit_info->material.BSDF(hit_info->hit_point, light_direction_local, view_direction)
                                * glm::abs(light_direction_local.y) * light_sample->Le / (light_sample->pdf * light_source_sample->prob);
                        }
                    }
                }

                auto bsdf_sample = hit_info->material.sampleBSDF(hit_info->hit_point, view_direction, rng);
                if (!bsdf_sample.has_value()) {
                    break;
                }
                last_bsdf_pdf = bsdf_sample->pdf;
                eta_scale *= bsdf_sample->eta_scale;
                beta *= bsdf_sample->bsdf * glm::abs(bsdf_sample->light_direction.y) / bsdf_sample->pdf;
                light_direction = bsdf_sample->light_direction;
            } else {
                break;
            }

            ray.origin = hit_info->hit_point;
            ray.direction = frame.worldFromLocal(light_direction);
        } else {
            glm::vec3 light_direction = glm::normalize(ray.direction);
            glm::vec3 light_point = ray.origin + scene.getRadius() * 2 * light_direction;
            if (last_is_specular) {
                for (auto infinite_light : scene.getInfiniteLights()) {
                    glm::vec3 light_direction = glm::normalize(ray.direction);
                    L += beta * infinite_light.getRadiance(ray.origin, light_point, -light_direction);
                }
            } else {
                for (auto infinite_light : scene.getInfiniteLights()) {
                    float light_source_prob = light_sampler.getProb(infinite_light);
                    float light_pdf = infinite_light.getPDF(ray.origin, light_point, -light_direction, allow_mis_compensation);
                    float weight_bsdf = PowerHeuristic(last_bsdf_pdf, light_source_prob * light_pdf);
                    L += weight_bsdf * beta * infinite_light.getRadiance(ray.origin, light_point, -light_direction);
                }
            }
            break;
        }
    }

    return L;
}
