#include "material/dielectric_material.hpp"

float Fresnel(float etai_div_etat, float cos_theta_t, float &cos_theta_i) {
    float sin2_theta_t = 1 - cos_theta_t * cos_theta_t;
    float sin2_theta_i = sin2_theta_t / (etai_div_etat * etai_div_etat);

    if (sin2_theta_i >= 1) {
        return 1;
    }

    cos_theta_i = glm::sqrt(1 - sin2_theta_i);
    float r_parl = (cos_theta_i - etai_div_etat * cos_theta_t) / (cos_theta_i + etai_div_etat * cos_theta_t);
    float r_perp = (etai_div_etat * cos_theta_i - cos_theta_t) / (etai_div_etat * cos_theta_i + cos_theta_t);
    return 0.5 * (r_parl * r_parl + r_perp * r_perp);
}

std::optional<BSDFSample> DielectricMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const {
    if (ior == 1) {
        return BSDFSample { albedo_t / glm::abs(view_direction.y), 1, -view_direction };
    }
    float etai_div_etat = ior;
    glm::vec3 microfacet_normal { 0, 1, 0 };
    if (!microfacet_theory.isDeltaDistribution()) {
        microfacet_normal = microfacet_theory.sampleVisibleNormal(view_direction, rng);
    }
    float cos_theta_t = view_direction.y;
    float scale = 1;
    if (cos_theta_t < 0) {
        etai_div_etat = 1.f / ior;
        scale = -1;
        cos_theta_t = -cos_theta_t;
    }

    float cos_theta_i;
    float fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    if (rng.uniform() <= fr) {
        glm::vec3 light_direction = -view_direction + 2.f * glm::dot(microfacet_normal, view_direction) * microfacet_normal;
        if (microfacet_theory.isDeltaDistribution()) {
            return BSDFSample { albedo_r / glm::abs(light_direction.y), 1, light_direction };
        }
        glm::vec3 brdf = albedo_r * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            / glm::abs(4.f * light_direction.y * view_direction.y);
        float pdf = microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) / glm::abs(4.f * glm::dot(view_direction, microfacet_normal));
        return BSDFSample { brdf, pdf, light_direction };
    } else {
        glm::vec3 light_direction { (-view_direction / etai_div_etat ) + (cos_theta_t / etai_div_etat - cos_theta_i) * scale * microfacet_normal };
        float det_J = etai_div_etat * etai_div_etat * glm::abs(glm::dot(light_direction, microfacet_normal))
            / glm::pow(
                glm::abs(glm::dot(view_direction, microfacet_normal)) - etai_div_etat * etai_div_etat * glm::abs(glm::dot(light_direction, microfacet_normal))
                , 2
            );
        glm::vec3 btdf = albedo_t * det_J * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            * glm::abs(glm::dot(view_direction, microfacet_normal) / (light_direction.y * view_direction.y));
        float pdf = microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) * det_J;
        return BSDFSample { btdf / (etai_div_etat * etai_div_etat), pdf, light_direction };
    }
}

glm::vec3 DielectricMaterial::BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
    if (isDeltaDistribution()) {
        return {};
    }
    float lv = light_direction.y * view_direction.y;
    if (lv == 0) {
        return {};
    }

    float etai_div_etat = ior;
    float cos_theta_t = view_direction.y;
    float scale = 1;
    if (cos_theta_t < 0) {
        etai_div_etat = 1.f / ior;
        scale = -1;
        cos_theta_t = -cos_theta_t;
    }
    float cos_theta_i;
    float fr = Fresnel(etai_div_etat, cos_theta_t, cos_theta_i);

    if (lv < 0) {
        glm::vec3 microfacet_normal = (light_direction + view_direction / etai_div_etat) * scale / ((cos_theta_t / etai_div_etat - cos_theta_i));
        float det_J = etai_div_etat * etai_div_etat * glm::abs(glm::dot(light_direction, microfacet_normal))
            / glm::pow(
                glm::abs(glm::dot(view_direction, microfacet_normal)) - etai_div_etat * etai_div_etat * glm::abs(glm::dot(light_direction, microfacet_normal))
                , 2
            );
        glm::vec3 btdf = (1 - fr) * albedo_t * det_J * microfacet_theory.normalDistribution(microfacet_normal)
            * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
            * glm::abs(glm::dot(view_direction, microfacet_normal) / lv);
        return btdf / (etai_div_etat * etai_div_etat);
    }
    glm::vec3 microfacet_normal = glm::normalize(light_direction + view_direction);
    if (microfacet_normal.y < 0) {
        microfacet_normal = -microfacet_normal;
    }
    glm::vec3 brdf = fr * albedo_r * microfacet_theory.normalDistribution(microfacet_normal)
        * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
        / glm::abs(4.f * lv);
    return brdf;
}
