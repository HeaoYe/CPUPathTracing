#include "renderer/simple_path_tracing_renderer.hpp"
#include "util/frame.hpp"

PixelSample SimplePathTracingRenderer::renderPixel(const glm::ivec3 &pixel_coord, const ColorSpace *target_color_space) {
    thread_local RNG rng {};
    rng.setState(
        pixel_coord.x + pixel_coord.y * camera.getFilm().getWidth(),
        (pixel_coord.x + 1) * (pixel_coord.y + 1) * pixel_coord.z
    );

    WavelengthSamples wavelength = ImportanceSampleWavelength(rng.uniform());
    auto ray = camera.generateRay(pixel_coord, { rng.uniform(), rng.uniform() });
    SpectrumSamples beta { 1 };
    SpectrumSamples L {};
    float q = 0.9;
    bool last_is_specular = true;

    while (true) {
        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
            if (last_is_specular && hit_info->material && hit_info->material->area_light) {
                L += beta * hit_info->material->area_light->getRadiance(ray.origin, hit_info->hit_point, hit_info->normal, wavelength);
            }

            if (rng.uniform() > q) {
                break;
            }
            beta /= q;

            Frame frame(hit_info->normal);
            glm::vec3 light_direction;
            if (hit_info->material) {
                glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
                if (view_direction.y == 0) {
                    ray.origin = hit_info->hit_point;
                    continue;
                }

                last_is_specular = hit_info->material->isDeltaDistribution();
                if (!last_is_specular) {
                    auto light_source_sample = scene.getLightSampler(false).sample(rng.uniform());
                    if (light_source_sample.has_value()) {
                        auto light_sample = light_source_sample->light->sampleLight(hit_info->hit_point, scene.getRadius(), rng, wavelength, false);
                        if (light_sample.has_value() && (!scene.intersect({ hit_info->hit_point, light_sample->light_point - hit_info->hit_point }, 1e-5, 1.f - 1e-5))) {
                            glm::vec3 light_direction_local = frame.localFromWorld(light_sample->light_direction);
                            L += beta * hit_info->material->BSDF(hit_info->hit_point, light_direction_local, view_direction, wavelength)
                                * glm::abs(light_direction_local.y) * light_sample->Le / (light_sample->pdf * light_source_sample->prob);
                        }
                    }
                }

                auto bsdf_sample = hit_info->material->sampleBSDF(hit_info->hit_point, view_direction, rng, wavelength);
                if (!bsdf_sample.has_value()) {
                    break;
                }
                beta *= bsdf_sample->bsdf * glm::abs(bsdf_sample->light_direction.y) / bsdf_sample->pdf;
                light_direction = bsdf_sample->light_direction;
            } else {
                break;
            }

            ray.origin = hit_info->hit_point;
            ray.direction = frame.worldFromLocal(light_direction);
        } else {
            if (last_is_specular) {
                for (const auto *infinite_light : scene.getInfiniteLights()) {
                    glm::vec3 light_direction = glm::normalize(ray.direction);
                    L += beta * infinite_light->getRadiance(ray.origin, ray.origin + scene.getRadius() * 2 * light_direction, -light_direction, wavelength);
                }
            }
            break;
        }
    }

    return { L, wavelength };
}
