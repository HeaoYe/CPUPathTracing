#pragma once

#include <glm/glm.hpp>

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
