#pragma once

#include <glm/glm.hpp>
#include <array>

class LinearRGB {
public:
    LinearRGB() = default;

    explicit LinearRGB(float r, float g, float b) : data(r, g, b) {}

    explicit LinearRGB(const glm::vec3 &data) : data(data) {}

    LinearRGB operator+(const LinearRGB &rhs) const { return LinearRGB { data + rhs.data }; }
    LinearRGB operator-(const LinearRGB &rhs) const { return LinearRGB { data - rhs.data }; }
    LinearRGB operator*(float rhs) const { return LinearRGB { data * rhs }; }
    LinearRGB operator/(float rhs) const { return LinearRGB { data / rhs }; }
    LinearRGB &operator+=(const LinearRGB &rhs) { data += rhs.data; return *this; }
    LinearRGB &operator-=(const LinearRGB &rhs) { data -= rhs.data; return *this; }
    LinearRGB &operator*=(float rhs) { data *= rhs; return *this; }
    LinearRGB &operator/=(float rhs) { data /= rhs; return *this; }

    float r() const { return data.x; }
    float g() const { return data.y; }
    float b() const { return data.z; }
    float &r() { return data.x; }
    float &g() { return data.y; }
    float &b() { return data.z; }
public:
    glm::vec3 data {};
};

class EncodedRGB {
public:
    inline static EncodedRGB GenerateHeatmapRGB(float t) {
        std::array<EncodedRGB, 25> color_pallet {
            EncodedRGB { 68, 1, 84 },
            EncodedRGB { 71, 17, 100 },
            EncodedRGB { 72, 31, 112 },
            EncodedRGB { 71, 45, 123 },
            EncodedRGB { 68, 58, 131 },

            EncodedRGB { 64, 70, 136 },
            EncodedRGB { 59, 82, 139 },
            EncodedRGB { 54, 93, 141 },
            EncodedRGB { 49, 104, 142 },
            EncodedRGB { 44, 114, 142 },

            EncodedRGB { 40, 124, 142 },
            EncodedRGB { 36, 134, 142 },
            EncodedRGB { 33, 144, 140 },
            EncodedRGB { 31, 154, 138 },
            EncodedRGB { 32, 164, 134 },

            EncodedRGB { 39, 173, 129 },
            EncodedRGB { 53, 183, 121 },
            EncodedRGB { 71, 193, 110 },
            EncodedRGB { 93, 200, 99 },
            EncodedRGB { 117, 208, 84 },

            EncodedRGB { 143, 215, 68 },
            EncodedRGB { 170, 220, 50 },
            EncodedRGB { 199, 224, 32 },
            EncodedRGB { 227, 228, 24 },
            EncodedRGB { 253, 231, 37 },
        };

        if (t < 0 || t >= 1) {
            return EncodedRGB { 255, 0, 0 };
        }
        float idx_float = t * (color_pallet.size() - 1);
        size_t idx = glm::floor(idx_float);
        float s = glm::fract(idx_float);
        return EncodedRGB { color_pallet[idx].data * (1 - s) + color_pallet[idx + 1].data * s };
    }
public:
    EncodedRGB() = default;

    explicit EncodedRGB(uint32_t r, uint32_t g, uint32_t b, uint32_t bit_nums = 8) : data(r, g, b) {
        float maximum = (1 << bit_nums) - 1;
        data /= maximum;
    }

    explicit EncodedRGB(const glm::vec3 &data) : data(data) {}

    glm::u32vec3 toBytes(uint32_t bit_nums) {
        int32_t maximum = (1 << bit_nums) - 1;
        return {
            glm::clamp<int>(glm::round(r() * maximum), 0, maximum),
            glm::clamp<int>(glm::round(g() * maximum), 0, maximum),
            glm::clamp<int>(glm::round(b() * maximum), 0, maximum),
        };
    }

    float r() const { return data.x; }
    float g() const { return data.y; }
    float b() const { return data.z; }
    float &r() { return data.x; }
    float &g() { return data.y; }
    float &b() { return data.z; }
public:
    glm::vec3 data {};
};

struct TransferFunction {
    std::function<float(float)> encode = [](float value) { return value; };
    std::function<float(float)> decode = [](float value) { return value; };
};
