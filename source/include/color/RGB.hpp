#pragma once

#include <glm/glm.hpp>

class LinearRGB {
public:
    LinearRGB() = default;

    explicit LinearRGB(float r, float g, float b) : r(r), g(g), b(b) {}

    explicit LinearRGB(const glm::vec3 &data) : data(data) {}

    LinearRGB operator+(const LinearRGB &rhs) const { return LinearRGB { data + rhs.data }; }
    LinearRGB operator-(const LinearRGB &rhs) const { return LinearRGB { data - rhs.data }; }
    LinearRGB operator*(float rhs) const { return LinearRGB { data * rhs }; }
    LinearRGB operator/(float rhs) const { return LinearRGB { data / rhs }; }
    LinearRGB &operator+=(const LinearRGB &rhs) { data += rhs.data; return *this; }
    LinearRGB &operator-=(const LinearRGB &rhs) { data -= rhs.data; return *this; }
    LinearRGB &operator*=(float rhs) { data *= rhs; return *this; }
    LinearRGB &operator/=(float rhs) { data /= rhs; return *this; }
public:
    union {
        struct {
            float r {};
            float g {};
            float b {};
        };
        glm::vec3 data;
    };
};

class EncodedRGB {
public:
    EncodedRGB() = default;

    explicit EncodedRGB(float r, float g, float b) : r(r), g(g), b(b) {}

    explicit EncodedRGB(uint32_t r, uint32_t g, uint32_t b, uint32_t bit_nums) {
        float maximum = (1 << bit_nums) - 1;
        this->r = r / maximum;
        this->g = g / maximum;
        this->b = b / maximum;
    }

    explicit EncodedRGB(const glm::vec3 &data) : data(data) {}

    glm::u32vec3 toBytes(uint32_t bit_nums) {
        int32_t maximum = (1 << bit_nums) - 1;
        return {
            glm::clamp<int>(r * maximum, 0, maximum),
            glm::clamp<int>(g * maximum, 0, maximum),
            glm::clamp<int>(b * maximum, 0, maximum),
        };
    }
public:
    union {
        struct {
            float r {};
            float g {};
            float b {};
        };
        glm::vec3 data;
    };
};

struct TransferFunction {
    std::function<float(float)> encode = [](float value) { return value; };
    std::function<float(float)> decode = [](float value) { return value; };
};
