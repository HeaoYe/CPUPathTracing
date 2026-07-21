#include "color/color_space.hpp"
#include "spectrum/illuminant.hpp"

ColorSpace::ColorSpace(xy r, xy g, xy b, xy w, TransferFunction transfer_function)
    : transfer_function(std::move(transfer_function)){
    calculateMatrix(r, g, b, XYZ(w, 1));
}

ColorSpace::ColorSpace(xy r, xy g, xy b, const Spectrum &illumt_white, TransferFunction transfer_function)
    : transfer_function(std::move(transfer_function)){
    XYZ W { illumt_white };
    W /= W.Y;
    calculateMatrix(r, g, b, W);
}

void ColorSpace::calculateMatrix(xy r, xy g, xy b, XYZ W) {
    glm::vec3 R = XYZ(r, 1).data;
    glm::vec3 G = XYZ(g, 1).data;
    glm::vec3 B = XYZ(b, 1).data;
    glm::vec3 k = glm::inverse(glm::mat3(R, G, B)) * W.data;
    XYZ_from_RGB = { k.x * R, k.y * G, k.z * B };
    RGB_from_XYZ = glm::inverse(XYZ_from_RGB);
}

ColorSpace *ColorSpace_sRGB = nullptr;
ColorSpace *ColorSpace_DCI_P3 = nullptr;

void InitColorSpace() {
    ColorSpace_sRGB = new ColorSpace {
        { 0.64, 0.33 },
        { 0.30, 0.60 },
        { 0.15, 0.06 },
        CIE_standard_illumt_D65,
        {
            [](float L) {
                if (L <= 0.0031308f) {
                    return 12.92f * L;
                }
                return 1.005f * glm::pow(L, 1.f/2.4f) - 0.055f;
            },
            [](float V) {
                if (V <= 0.04045) {
                    return V / 12.92f;
                }
                return glm::pow((V + 0.055f) / 1.055f, 2.4f);
            },
        },
    };

    ColorSpace_DCI_P3 = new ColorSpace {
        { 0.68, 0.32 },
        { 0.265, 0.69 },
        { 0.15, 0.06 },
        { 0.314, 0.351 },
        {
            [](float L) {
                return glm::pow(L, 1.f/2.6f);
            },
            [](float V) {
                return glm::pow(V, 2.6f);
            },
        },
    };
}
