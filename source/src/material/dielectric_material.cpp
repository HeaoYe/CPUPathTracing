#include "material/dielectric_material.hpp"

float Fresnel(float etai_div_etat, float cos_theta_t, float &cos_theta_i) {
    cos_theta_t = glm::clamp(cos_theta_t, 0.f, 1.f);
    float sin2_theta_t = glm::clamp(1 - cos_theta_t * cos_theta_t, 0.f, 1.f);
    float sin2_theta_i = sin2_theta_t / (etai_div_etat * etai_div_etat);

    if (sin2_theta_i >= 1) {
        return 1;
    }

    cos_theta_i = glm::sqrt(1 - sin2_theta_i);
    float r_parl = (cos_theta_i - etai_div_etat * cos_theta_t) / (cos_theta_i + etai_div_etat * cos_theta_t);
    float r_perp = (etai_div_etat * cos_theta_i - cos_theta_t) / (etai_div_etat * cos_theta_i + cos_theta_t);
    return 0.5 * (r_parl * r_parl + r_perp * r_perp);
}

SpectrumSamples Fresnel(SpectrumSamples etai_div_etat, float cos_theta_t, SpectrumSamples &cos_theta_i) {
    SpectrumSamples result {};

    cos_theta_t = glm::clamp(cos_theta_t, 0.f, 1.f);
    float sin2_theta_t = glm::clamp(1 - cos_theta_t * cos_theta_t, 0.f, 1.f);
    auto sin2_theta_i = sin2_theta_t / (etai_div_etat * etai_div_etat);

    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        if (sin2_theta_i[i] >= 1) {
            result[i] = 1;
            continue;
        }

        cos_theta_i[i] = glm::sqrt(1 - sin2_theta_i[i]);
        float r_parl = (cos_theta_i[i] - etai_div_etat[i] * cos_theta_t) / (cos_theta_i[i] + etai_div_etat[i] * cos_theta_t);
        float r_perp = (etai_div_etat[i] * cos_theta_i[i] - cos_theta_t) / (etai_div_etat[i] * cos_theta_i[i] + cos_theta_t);
        result[i] = 0.5 * (r_parl * r_parl + r_perp * r_perp);
    }

    return result;
}

std::optional<BSDFSample> DielectricMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng, const WavelengthSamples &wavelength) const {
    if (ior->isConstant() && (ior->max() == 1)) {
        return BSDFSample { albedo_t->sample(wavelength) / glm::abs(view_direction.y), 1, -view_direction };
    }
    auto etai_div_etat = ior->sample(wavelength);
    glm::vec3 microfacet_normal { 0, 1, 0 };
    if (!microfacet_theory.isDeltaDistribution()) {
        microfacet_normal = microfacet_theory.sampleVisibleNormal(view_direction, rng);
    }
    float cos_theta_t = glm::dot(view_direction, microfacet_normal);
    float scale = 1;
    if (view_direction.y < 0) {
        etai_div_etat = 1.f / etai_div_etat;
        scale = -1;
        cos_theta_t = -cos_theta_t;
    }

    SpectrumSamples cos_theta_i;
    auto fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    if (rng.uniform() < fr[0]) {
        glm::vec3 light_direction = -view_direction + 2.f * glm::dot(microfacet_normal, view_direction) * microfacet_normal;
        if (light_direction.y * view_direction.y <= 0) {
            return {};
        }

        if (microfacet_theory.isDeltaDistribution()) {
            return BSDFSample { fr * albedo_r->sample(wavelength) / glm::abs(light_direction.y), fr[0], light_direction };
        }

        auto brdf = fr * albedo_r->sample(wavelength) * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            / glm::abs(4.f * light_direction.y * view_direction.y);
        float pdf = fr[0] * microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) / glm::abs(4.f * glm::dot(view_direction, microfacet_normal));
        return BSDFSample { brdf, pdf, light_direction };
    } else {
        wavelength.terminated = !ior->isConstant();
        glm::vec3 light_direction { (-view_direction / etai_div_etat[0] ) + (cos_theta_t / etai_div_etat[0] - cos_theta_i[0]) * scale * microfacet_normal };
        if (light_direction.y * view_direction.y >= 0) {
            return {};
        }

        if (microfacet_theory.isDeltaDistribution()) {
            return BSDFSample { (1.f - fr) * albedo_t->sample(wavelength) / glm::abs(light_direction.y), 1.f - fr[0], light_direction, etai_div_etat * etai_div_etat };
        }

        float det_J = etai_div_etat[0] * etai_div_etat[0] * glm::abs(glm::dot(light_direction, microfacet_normal))
            / glm::pow(
                glm::abs(glm::dot(view_direction, microfacet_normal)) - etai_div_etat[0] * glm::abs(glm::dot(light_direction, microfacet_normal))
                , 2
            );
        auto btdf = (1.f - fr) * albedo_t->sample(wavelength) * det_J * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            * glm::abs(glm::dot(view_direction, microfacet_normal) / (light_direction.y * view_direction.y));
        float pdf = (1.f - fr[0]) * microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) * det_J;
        return BSDFSample { btdf / (etai_div_etat * etai_div_etat), pdf, light_direction, etai_div_etat * etai_div_etat };
    }
}

SpectrumSamples DielectricMaterial::BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction, const WavelengthSamples &wavelength) const {
    if (isDeltaDistribution()) {
        return {};
    }
    float lv = light_direction.y * view_direction.y;
    if (lv == 0) {
        return {};
    }

    auto etai_div_etat = ior->sample(wavelength);
    float scale = 1;
    if (view_direction.y < 0) {
        etai_div_etat = 1.f / etai_div_etat;
        scale = -1;
    }

    glm::vec3 microfacet_normal {};
    if (lv > 0) {
        microfacet_normal = light_direction + view_direction;
        if (glm::dot(microfacet_normal, microfacet_normal) == 0) {
            return {};
        }
        if (microfacet_normal.y < 0) {
            microfacet_normal = -microfacet_normal;
        }
        if ((glm::dot(light_direction, microfacet_normal) * light_direction.y <= 0) ||
            (glm::dot(view_direction, microfacet_normal) * view_direction.y <= 0)) {
            return {};
        }
        microfacet_normal = glm::normalize(microfacet_normal);

        float cos_theta_t = glm::dot(view_direction, microfacet_normal * scale);
        SpectrumSamples cos_theta_i;
        auto fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

        auto brdf = fr * albedo_r->sample(wavelength) * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            / glm::abs(4.f * lv);
        return brdf;
    }

    SpectrumSamples btdf = albedo_t->sample(wavelength);
    for (size_t i = 0; i < g_wavelength_sample_count; i ++) {
        microfacet_normal = light_direction + view_direction / etai_div_etat[i];
        if (glm::dot(microfacet_normal, microfacet_normal) == 0) {
            return {};
        }
        if (microfacet_normal.y < 0) {
            microfacet_normal = -microfacet_normal;
        }
        if ((glm::dot(light_direction, microfacet_normal) * light_direction.y <= 0) ||
            (glm::dot(view_direction, microfacet_normal) * view_direction.y <= 0)) {
            return {};
        }
        microfacet_normal = glm::normalize(microfacet_normal);

        float cos_theta_t = glm::dot(view_direction, microfacet_normal * scale);
        float cos_theta_i;
        float fr = Fresnel(etai_div_etat[i], cos_theta_t, cos_theta_i);

        float det_J = etai_div_etat[i] * etai_div_etat[i] * glm::abs(glm::dot(light_direction, microfacet_normal))
            / glm::pow(
                glm::abs(glm::dot(view_direction, microfacet_normal)) - etai_div_etat[i] * glm::abs(glm::dot(light_direction, microfacet_normal))
                , 2
            );
        btdf[i] *= (1 - fr) * det_J * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            * glm::abs(glm::dot(view_direction, microfacet_normal) / lv);
    }
    return btdf / (etai_div_etat * etai_div_etat);
}

float DielectricMaterial::PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction, const WavelengthSamples &wavelength) const {
    if (isDeltaDistribution()) {
        return 0;
    }
    float lv = light_direction.y * view_direction.y;
    if (lv == 0) {
        return 0;
    }

    float etai_div_etat = (*ior)[wavelength.lambdas[0]];
    float scale = 1;
    if (view_direction.y < 0) {
        etai_div_etat = 1.f / etai_div_etat;
        scale = -1;
    }

    glm::vec3 microfacet_normal {};
    if (lv < 0) {
        microfacet_normal = light_direction + view_direction / etai_div_etat;
    } else {
        microfacet_normal = light_direction + view_direction;
    }
    if (glm::dot(microfacet_normal, microfacet_normal) == 0) {
        return {};
    }
    if (microfacet_normal.y < 0) {
        microfacet_normal = -microfacet_normal;
    }
    if ((glm::dot(light_direction, microfacet_normal) * light_direction.y <= 0) ||
        (glm::dot(view_direction, microfacet_normal) * view_direction.y <= 0)) {
        return {};
    }
    microfacet_normal = glm::normalize(microfacet_normal);

    float cos_theta_t = glm::dot(view_direction, microfacet_normal * scale);
    float cos_theta_i;
    float fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    if (lv < 0) {
        float det_J = etai_div_etat * etai_div_etat * glm::abs(glm::dot(light_direction, microfacet_normal))
            / glm::pow(
                glm::abs(glm::dot(view_direction, microfacet_normal)) - etai_div_etat * glm::abs(glm::dot(light_direction, microfacet_normal))
                , 2
            );
        return (1.f - fr) * microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) * det_J;
    }
    return fr * microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) / glm::abs(4.f * glm::dot(view_direction, microfacet_normal));
}
