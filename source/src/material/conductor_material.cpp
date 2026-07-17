#include "material/conductor_material.hpp"
#include "util/complex.hpp"

glm::vec3 Fresnel(const glm::vec3 &ior, const glm::vec3 &k, float cos_theta_i) {
    glm::vec3 fr {};
    for (size_t i = 0; i < 3; i ++) {
        Complex etat_div_etai { ior[i], k[i] };
        cos_theta_i = glm::clamp(cos_theta_i, 0.f, 1.f);
        float sin2_theta_i = 1.f - cos_theta_i * cos_theta_i;
        Complex sin2_theta_t = sin2_theta_i / (etat_div_etai * etat_div_etai);
        Complex cos_theta_t = sqrt(1.f - sin2_theta_t);

        Complex r_parl = (etat_div_etai * cos_theta_i - cos_theta_t) / (etat_div_etai * cos_theta_i + cos_theta_t);
        Complex r_perp = (cos_theta_i - etat_div_etai * cos_theta_t) / (cos_theta_i + etat_div_etai * cos_theta_t);

        fr[i] = 0.5 * (lengthSquared(r_parl) + lengthSquared(r_perp));
    }
    return fr;
}

std::optional<BSDFSample> ConductorMaterial::sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, const RNG &rng) const {
    glm::vec3 microfacet_normal { 0, 1, 0 };
    if (!microfacet_theory.isDeltaDistribution()) {
        microfacet_normal = microfacet_theory.sampleVisibleNormal(view_direction, rng);
    }
    glm::vec3 fr = Fresnel(ior, k, glm::abs(glm::dot(view_direction, microfacet_normal)));
    glm::vec3 light_direction = -view_direction + 2.f * glm::dot(microfacet_normal, view_direction) * microfacet_normal;
    if (microfacet_theory.isDeltaDistribution()) {
        return BSDFSample { fr / glm::abs(light_direction.y), 1, light_direction };
    }
    glm::vec3 brdf = fr * microfacet_theory.normalDistribution(microfacet_normal)
        * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
        / glm::abs(4.f * light_direction.y * view_direction.y);
    float pdf = microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) / glm::abs(4.f * glm::dot(view_direction, microfacet_normal));
    return BSDFSample { brdf, pdf, light_direction };
}

glm::vec3 ConductorMaterial::BSDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
    if (microfacet_theory.isDeltaDistribution()) {
        return {};
    }
    float lv = light_direction.y * view_direction.y;
    if (lv <= 0) {
        return {};
    }

    glm::vec3 microfacet_normal = glm::normalize(light_direction + view_direction);
    if (microfacet_normal.y < 0) {
        microfacet_normal = -microfacet_normal;
    }
    glm::vec3 fr = Fresnel(ior, k, glm::abs(glm::dot(view_direction, microfacet_normal)));
    glm::vec3 brdf = fr * microfacet_theory.normalDistribution(microfacet_normal)
        * microfacet_theory.heightCorrelatedMaskingShadowing(light_direction, view_direction, microfacet_normal)
        / glm::abs(4.f * lv);
    return brdf;
}

float ConductorMaterial::PDF(const glm::vec3 &hit_point, const glm::vec3 &light_direction, const glm::vec3 &view_direction) const {
    if (microfacet_theory.isDeltaDistribution()) {
        return 0;
    }
    float lv = light_direction.y * view_direction.y;
    if (lv <= 0) {
        return 0;
    }

    glm::vec3 microfacet_normal = glm::normalize(light_direction + view_direction);
    if (microfacet_normal.y < 0) {
        microfacet_normal = -microfacet_normal;
    }
    return microfacet_theory.visibleNormalDistribution(view_direction, microfacet_normal) / glm::abs(4.f * glm::dot(view_direction, microfacet_normal));
}
