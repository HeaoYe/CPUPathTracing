#include "light/image_infinite_light.hpp"
#include "sample/spherical.hpp"

ImageInfiniteLight::ImageInfiniteLight(const Image *image, float start_phi) : image(image), start_phi(start_phi) {
    precompute_phi = 0;
    gird_count = girdIdxFromImagePoint(image->getResolution()) + 1;
    std::vector<float> girds_phi(gird_count.x * gird_count.y);
    for (size_t y = 0; y < image->getHeight(); y ++) {
        for (size_t x = 0; x < image->getWidth(); x ++) {
            glm::vec3 radiance = image->getPixel(x, y);
            float pixel_phi = glm::max(radiance.r, glm::max(radiance.g, radiance.b)) * (glm::cos(y * PI / image->getHeight()) - glm::cos((y + 1) * PI / image->getHeight()));
            precompute_phi += pixel_phi;
            auto gird_idx = girdIdxFromImagePoint({ x, y });
            girds_phi[gird_idx.y * gird_count.x + gird_idx.x] += pixel_phi;
        }
    }
    float average_phi = precompute_phi / (gird_count.x * gird_count.y);
    precompute_phi *= 2 * PI * PI / image->getWidth();
    alias_table.build(girds_phi);
    impossible_compensated = true;
    for (float &gird_phi : girds_phi) {
        if (gird_phi > average_phi) {
            impossible_compensated = false;
            gird_phi -= average_phi;
        } else {
            gird_phi = 0;
        }
    }
    if (!impossible_compensated) {
        alias_table_compensated.build(girds_phi);
    }
}

std::optional<LightSample> ImageInfiniteLight::sampleLight(const glm::vec3 &surface_point, float scene_radius, const RNG &rng, bool allow_mis_compensation) const {
    auto result = ((allow_mis_compensation && (!impossible_compensated)) ? alias_table_compensated : alias_table).sample(rng.uniform());
    size_t gird_x = result.index % gird_count.x;
    size_t gird_y = result.index / gird_count.x;
    float w = glm::min<float>(gird_side_length, image->getWidth() - gird_x * gird_side_length);
    float h = glm::min<float>(gird_side_length, image->getHeight() - gird_y * gird_side_length);
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
        image->getPixel(image_point),
        result.prob * image->getWidth() * image->getHeight() / (2 * PI * PI * glm::sqrt(1 - light_direction.y * light_direction.y) * w * h)
    };
}

glm::vec3 ImageInfiniteLight::getRadiance(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal) const {
    glm::vec3 light_direction = glm::normalize(light_point - surface_point);
    if (glm::abs(light_direction.y) == 1) {
        return {};
    }
    return image->getPixel(imagePointFromDirection(light_direction));
}

float ImageInfiniteLight::getPDF(const glm::vec3 &surface_point, const glm::vec3 &light_point, const glm::vec3 &normal, bool allow_mis_compensation) const {
    glm::vec3 light_direction = glm::normalize(light_point - surface_point);
    if (glm::abs(light_direction.y) == 1) {
        return {};
    }

    glm::vec2 image_point = imagePointFromDirection(light_direction);
    glm::ivec2 gird_idx = girdIdxFromImagePoint(image_point);
    float w = glm::min<float>(gird_side_length, image->getWidth() - gird_idx.x * gird_side_length);
    float h = glm::min<float>(gird_side_length, image->getHeight() - gird_idx.y * gird_side_length);

    float gird_prob = ((allow_mis_compensation && (!impossible_compensated)) ? alias_table_compensated : alias_table).getProbs()[gird_idx.y * gird_count.x + gird_idx.x];

    return gird_prob * image->getWidth() * image->getHeight() / (2 * PI * PI * glm::sqrt(1 - light_direction.y * light_direction.y) * w * h);
}

glm::vec2 ImageInfiniteLight::imagePointFromDirection(const glm::vec3 &direction) const {
    float theta = 0;
    float phi = 0;
    glm::vec3 normalized_direction = glm::normalize(direction);
    if (glm::abs(normalized_direction.y) < 0.99999) {
        theta = glm::degrees(glm::acos(normalized_direction.y));
        float sin_phi = glm::abs(normalized_direction.z / glm::sqrt(1 - normalized_direction.y * normalized_direction.y));
        phi = glm::degrees(glm::asin(sin_phi));
        if ((direction.x <= 0) && (direction.z > 0)) {
            phi = 180 - phi;
        } else if ((direction.x < 0) && (direction.z <= 0)) {
            phi = 180 + phi;
        } else if ((direction.x >= 0) && (direction.z < 0)) {
            phi = 360 - phi;
        }
    } else {
        theta = (normalized_direction.y > 0) ? 0 : 180;
    }

    phi += start_phi;
    if (phi > 360) {
        phi -= 360;
    }

    return { image->getWidth() * phi / 360, image->getHeight() * theta / 180 };
}

glm::vec3 ImageInfiniteLight::directionFromImagePoint(const glm::vec2 &image_point) const {
    float theta = glm::radians(180 * image_point.y / image->getHeight());
    float phi = glm::radians(360 * image_point.x / image->getWidth() - start_phi);

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
        glm::clamp<int>(image_point.x, 0, image->getWidth() - 1),
        glm::clamp<int>(image_point.y, 0, image->getHeight() - 1),
    };

    return point_discrete / static_cast<int>(gird_side_length);
}
