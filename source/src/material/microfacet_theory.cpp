#include "material/microfacet_theory.hpp"
#include "sample/spherical.hpp"
#include "util/frame.hpp"

MicrofacetTheory::MicrofacetTheory(float alpha_x, float alpha_z) {
    this->alpha_x = glm::clamp(alpha_x * alpha_x, 1e-3f, 1.f);
    this->alpha_z = glm::clamp(alpha_z * alpha_z, 1e-3f, 1.f);
}

float MicrofacetTheory::normalDistribution(const glm::vec3 &microfacet_normal) const {
    glm::vec2 slope { -microfacet_normal.x / microfacet_normal.y, -microfacet_normal.z / microfacet_normal.y };
    slope.x /= alpha_x;
    slope.y /= alpha_z;
    float slope_distribution = slopeDistribution(slope) / (alpha_x * alpha_z);
    return slope_distribution / glm::pow(microfacet_normal.y, 4);
}

float MicrofacetTheory::masking(const glm::vec3 &view_direction, const glm::vec3 &microfacet_normal) const {
    glm::vec3 view_direction_upper = view_direction.y > 0 ? view_direction : -view_direction;
    if (glm::dot(view_direction_upper, microfacet_normal) <= 0) {
        return 0;
    }

    return 1.f / (1.f + Lambda(view_direction_upper));
}

float MicrofacetTheory::heightCorrelatedMaskingShadowing(const glm::vec3 &light_direction, const glm::vec3 &view_direction, const glm::vec3 &microfacet_normal) const {
    glm::vec3 light_direction_upper = light_direction.y > 0 ? light_direction : -light_direction;
    if (glm::dot(light_direction_upper, microfacet_normal) <= 0) {
        return 0;
    }

    glm::vec3 view_direction_upper = view_direction.y > 0 ? view_direction : -view_direction;
    if (glm::dot(view_direction_upper, microfacet_normal) <= 0) {
        return 0;
    }

    return 1.f / (1.f + Lambda(light_direction_upper) + Lambda(view_direction_upper));
}

float MicrofacetTheory::visibleNormalDistribution(const glm::vec3 &view_direction, const glm::vec3 &microfacet_normal) const {
    glm::vec3 view_direction_upper = view_direction.y > 0 ? view_direction : -view_direction;
    float cos_theta_o = glm::dot(view_direction_upper, microfacet_normal);
    if (cos_theta_o <= 0) {
        return 0;
    }
    return normalDistribution(microfacet_normal) * cos_theta_o * masking(view_direction, microfacet_normal) / glm::abs(view_direction.y);
}

glm::vec3 MicrofacetTheory::sampleVisibleNormal(const glm::vec3 &view_direction, const RNG &rng) const {
    glm::vec3 view_direction_upper = view_direction.y > 0 ? view_direction : -view_direction;
    glm::vec3 view_direction_hemi = glm::normalize(glm::vec3(alpha_x * view_direction_upper.x, view_direction_upper.y, alpha_z * view_direction_upper.z));

    glm::vec2 sample = UniformSampleUnitDisk({ rng.uniform(), rng.uniform() });
    float h = glm::sqrt(1.f - sample.x * sample.x);
    float t = 0.5f * (1.f + view_direction_hemi.y);
    sample.y = t * sample.y + (1.f - t) * h;

    Frame frame(view_direction_hemi);
    glm::vec3 microfacet_normal_hemi = frame.worldFromLocal({ sample.x, glm::sqrt(1.f - sample.x * sample.x - sample.y * sample.y), sample.y });

    return glm::normalize(glm::vec3(alpha_x * microfacet_normal_hemi.x, microfacet_normal_hemi.y, alpha_z * microfacet_normal_hemi.z));
}

bool MicrofacetTheory::isDeltaDistribution() const {
    return glm::max(alpha_x, alpha_z) == 1e-3f;
}

float MicrofacetTheory::slopeDistribution(const glm::vec2 &slope) const {
    return 1.f / (PI * glm::pow(1.f + slope.x * slope.x + slope.y * slope.y, 2));
}

float MicrofacetTheory::Lambda(const glm::vec3 &direction_upper) const {
    if (direction_upper.y == 0) {
        return std::numeric_limits<float>::infinity();
    }
    float length2 = direction_upper.x * direction_upper.x + direction_upper.z * direction_upper.z;
    if (length2 == 0) {
        return 0;
    }
    float cos2_phi = direction_upper.x * direction_upper.x / length2;
    float sin2_phi = direction_upper.z * direction_upper.z / length2;
    float tan2_theta = length2 / (direction_upper.y * direction_upper.y);
    float alpha0_2 = alpha_x * alpha_x * cos2_phi + alpha_z * alpha_z * sin2_phi;
    return 0.5f * (glm::sqrt(1.f + alpha0_2 * tan2_theta) - 1.f);
}
