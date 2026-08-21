#include "light/image_infinite_light.hpp"
#include "sample/spherical.hpp"
#include <cmath>

ImageInfiniteLight::ImageInfiniteLight(const Image &image_, float start_phi) : image(ColorLUT_sRGB, image_), start_phi(start_phi) {
    precompute_phi = 0;
    gird_count = girdIdxFromImagePoint(image.getResolution()) + 1;
    std::vector<float> girds_phi(gird_count.x * gird_count.y);
    for (size_t y = 0; y < image.getHeight(); y ++) {
        for (size_t x = 0; x < image.getWidth(); x ++) {
            LinearRGB radiance = image_.getPixel(x, y);
            float pixel_phi = glm::max(radiance.r(), glm::max(radiance.g(), radiance.b())) * (glm::cos(y * PI / image.getHeight()) - glm::cos((y + 1) * PI / image.getHeight()));
            precompute_phi += pixel_phi;
            auto gird_idx = girdIdxFromImagePoint({ x, y });
            girds_phi[gird_idx.y * gird_count.x + gird_idx.x] += pixel_phi;
        }
    }
    float average_phi = precompute_phi / (gird_count.x * gird_count.y);
    precompute_phi *= 2 * PI * PI / image.getWidth();
    alias_table.build(girds_phi);
    skip_mis_compensation = true;
    for (float &gird_phi : girds_phi) {
        if (gird_phi > average_phi) {
            skip_mis_compensation = false;
            gird_phi -= average_phi;
        } else {
            gird_phi = 0;
        }
    }
    if (!skip_mis_compensation) {
        alias_table_compensated.build(girds_phi);
    }
}

std::optional<LightSample> ImageInfiniteLight::sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, const WavelengthSamples &wavelength, bool allow_mis_compensation) const {
    auto result = (allow_mis_compensation && (!skip_mis_compensation) ? alias_table_compensated : alias_table).sample(rng.uniform());
    size_t gird_x = result.index % gird_count.x;
    size_t gird_y = result.index / gird_count.x;
    float w = glm::min<float>(gird_side_length, image.getWidth() - gird_x * gird_side_length);
    float h = glm::min<float>(gird_side_length, image.getHeight() - gird_y * gird_side_length);
    glm::vec2 image_point {
        gird_x * gird_side_length + w * rng.uniform(),
        gird_y * gird_side_length + h * rng.uniform(),
    };

    glm::vec3 light_direction = directionFromImagePoint(image_point);
    if (glm::abs(light_direction.y) == 1) {
        return {};
    }

    return LightSample {
        surface_point + 2 * scene_radius * light_direction,
        light_direction,
        image.sample(image_point, wavelength),
        SpectrumSamples(result.prob * image.getWidth() * image.getHeight() / (2 * PI * PI * glm::sqrt(1 - light_direction.y * light_direction.y) * w * h))
    };
}

SpectrumSamples ImageInfiniteLight::getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, const WavelengthSamples &wavelength) const {
    glm::vec3 light_direction = glm::normalize(light_point - surface_point);
    if (glm::abs(light_direction.y) == 1) {
        return {};
    }
    return image.sample(imagePointFromDirection(light_direction), wavelength);
}

SpectrumSamples ImageInfiniteLight::getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, const WavelengthSamples &wavelength, bool allow_mis_compensation) const {
    glm::vec3 light_direction = glm::normalize(light_point - surface_point);
    if (glm::abs(light_direction.y) == 1) {
        return {};
    }

    glm::vec2 image_point = imagePointFromDirection(light_direction);
    glm::ivec2 gird_idx = girdIdxFromImagePoint(image_point);
    float w = glm::min<float>(gird_side_length, image.getWidth() - gird_idx.x * gird_side_length);
    float h = glm::min<float>(gird_side_length, image.getHeight() - gird_idx.y * gird_side_length);

    float gird_prob = (allow_mis_compensation && (!skip_mis_compensation) ? alias_table_compensated : alias_table).getProbs()[gird_idx.y * gird_count.x + gird_idx.x];

    return SpectrumSamples(gird_prob * image.getWidth() * image.getHeight() / (2 * PI * PI * glm::sqrt(1 - light_direction.y * light_direction.y) * w * h));
}

glm::vec2 ImageInfiniteLight::imagePointFromDirection(const glm::vec3 &direction) const {
    glm::vec3 normalized_direction = glm::normalize(direction);
    float theta = glm::degrees(glm::acos(glm::clamp(normalized_direction.y, -1.f, 1.f)));
    float phi = glm::degrees(glm::atan(normalized_direction.z, normalized_direction.x)) + start_phi;
    phi = std::fmod(phi, 360.f);
    if (phi < 0) {
        phi += 360;
    }
    return { image.getWidth() * phi / 360, image.getHeight() * theta / 180 };
}

glm::vec3 ImageInfiniteLight::directionFromImagePoint(const glm::vec2 &image_point) const {
    float theta = glm::radians(180 * image_point.y / image.getHeight());
    float phi = glm::radians(360 * image_point.x / image.getWidth() - start_phi);

    float sin_theta = glm::sin(theta);
    float cos_theta = glm::cos(theta);
    float sin_phi = glm::sin(phi);
    float cos_phi = glm::cos(phi);

    return {
        sin_theta * cos_phi,
        cos_theta,
        sin_theta * sin_phi
    };
}

glm::ivec2 ImageInfiniteLight::girdIdxFromImagePoint(const glm::vec2 &image_point) const {
    glm::ivec2 point_discrete {
        glm::clamp<int>(image_point.x, 0, image.getWidth() - 1),
        glm::clamp<int>(image_point.y, 0, image.getHeight() - 1),
    };

    return point_discrete / static_cast<int>(gird_side_length);
}
