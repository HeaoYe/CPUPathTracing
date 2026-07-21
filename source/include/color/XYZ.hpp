#pragma once

#include "spectrum/densely_sampled_spectrum.hpp"
#include <glm/glm.hpp>

extern DenselySampledSpectrum X_color_matching;
extern DenselySampledSpectrum Y_color_matching;
extern DenselySampledSpectrum Z_color_matching;

class XYZ {
public:
    XYZ() = default;

    XYZ(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}

    explicit XYZ(const glm::vec3 &data) : data(data) {}

    explicit XYZ(const class xy &xy, float Y = 1);

    explicit XYZ(const Spectrum &spectrum);

    XYZ operator+(const XYZ &rhs) const { return XYZ { data + rhs.data }; }
    XYZ operator-(const XYZ &rhs) const { return XYZ { data - rhs.data }; }
    XYZ operator*(float rhs) const { return XYZ { data * rhs }; }
    XYZ operator/(float rhs) const { return XYZ { data / rhs }; }
    XYZ &operator+=(const XYZ &rhs) { data += rhs.data; return *this; }
    XYZ &operator-=(const XYZ &rhs) { data -= rhs.data; return *this; }
    XYZ &operator*=(float rhs) { data *= rhs; return *this; }
    XYZ &operator/=(float rhs) { data /= rhs; return *this; }
public:
    union {
        struct {
            float X {};
            float Y {};
            float Z {};
        };
        glm::vec3 data;
    };
};

class xy {
public:
    xy() = default;

    xy(float x, float y) : x(x), y(y) {}

    explicit xy(const XYZ &xyz);
public:
    float x {}, y {};
};
