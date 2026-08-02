#pragma once

#include "XYZ.hpp"
#include "RGB.hpp"
#include "spectrum/spectrum.hpp"

class ColorSpace {
public:
    ColorSpace(xy r, xy g, xy b, xy w, TransferFunction transfer_function);

    ColorSpace(xy r, xy g, xy b, const Spectrum &illumt_white, TransferFunction transfer_function);

    LinearRGB RGBFromXYZ(const XYZ &xyz) const { return LinearRGB { RGB_from_XYZ * xyz.data }; }

    XYZ XYZFromRGB(const LinearRGB &rgb) const { return XYZ { XYZ_from_RGB * rgb.data }; }

    LinearRGB decode(const EncodedRGB &encoded_rgb) const {
        return LinearRGB {
            transfer_function.decode(encoded_rgb.r()),
            transfer_function.decode(encoded_rgb.g()),
            transfer_function.decode(encoded_rgb.b()),
        };
    }

    EncodedRGB encode(const LinearRGB &linear_rgb) const {
        return EncodedRGB { {
            transfer_function.encode(linear_rgb.r()),
            transfer_function.encode(linear_rgb.g()),
            transfer_function.encode(linear_rgb.b()),
        } };
    }
private:
    void calculateMatrix(xy r, xy g, xy b, XYZ W);
private:
    xy r {}, g {}, b {}, w {};
    glm::mat3 RGB_from_XYZ;
    glm::mat3 XYZ_from_RGB;
    TransferFunction transfer_function;
};

void InitColorSpace();

extern ColorSpace *ColorSpace_sRGB;
extern ColorSpace *ColorSpace_DCI_P3;
