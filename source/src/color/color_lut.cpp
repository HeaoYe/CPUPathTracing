#include "color/color_lut.hpp"
#include "spectrum/illuminant.hpp"
#include "spectrum/sigmoid_polynomial_spectrum.hpp"
#include "spectrum/analytic_spectrum.hpp"
#include "util/profile.hpp"
#include "util/progress.hpp"
#include "thread/thread_pool.hpp"
#include <cmath>
#include <fstream>

double Sigmoid(double x) {
    return 0.5 * x / std::sqrt(1 + x * x) + 0.5;
}

void SigmoidAndDerivate(double x, double &s, double &ds) {
    float a = 1 + x * x;
    float t = std::sqrt(a);
    s = 0.5 * x / t + 0.5;
    ds = 1 / (2 * t * a);
}

double CIELAB_f(double t) {
    constexpr double delta = 6.0 / 29.0;
    if (t > delta * delta * delta) {
        return std::cbrt(t);
    }
    return t / (3 * delta * delta) + 4.0 / 29.0;
}

void CIELAB_fAndDerivate(double t, double &f, double &df) {
    constexpr double delta = 6.0 / 29.0;
    if (t > delta * delta * delta) {
        double x = std::cbrt(t);
        f = x;
        df = 1 / (3 * x * x);
    } else {
        f = t / (3 * delta * delta) + 4.0 / 29.0;
        df = 1 / (3 * delta * delta);
    }
}

class OptimizeContext {
public:
    OptimizeContext(const ColorSpace *color_space, const Spectrum *illuminant)
        : color_space(color_space), illuminant(illuminant), k(0) {
        for (int lambda = g_lambda_min; lambda <= g_lambda_max; lambda ++) {
            k += (*illuminant)[lambda] * Y_color_matching[lambda];
        }
        k = 1 / k;
        W = XYZ(color_space->getWhite(), 1).data;
    }
public:
    const ColorSpace *color_space {};
    const Spectrum *illuminant {};
    double k {};
    glm::dvec3 W {};
};

struct OptimizeOptions {
    double init_mu = 1e-7;
    double max_mu = 1e12;
    double max_iterations = 1024;
    double residual_epsilon = 1e-8;
};

struct Evaluation {
    glm::dvec3 theta {};

    glm::dvec3 cielab {};
    glm::dmat3 J {};

    void eval(const OptimizeContext &ctx) {
        glm::dvec3 XYZ {};
        glm::dmat3 dXYZ_dtheta {};
        glm::dmat3 dLab_dXYZ {};

        for (int lambda = g_lambda_min; lambda < g_lambda_max; lambda ++) {
            double u = static_cast<double>(lambda - g_lambda_min) / (g_lambda_max - g_lambda_min);
            double q = ((theta[2] * u) + theta[1]) * u + theta[0];
            double s, ds;
            SigmoidAndDerivate(q, s, ds);
            double kI = ctx.k * (*ctx.illuminant)[lambda];
            glm::dvec3 XYZ_cmfs {
                X_color_matching[lambda],
                Y_color_matching[lambda],
                Z_color_matching[lambda],
            };
            XYZ += kI * s * XYZ_cmfs;
            dXYZ_dtheta[0] += kI * ds * XYZ_cmfs;
            dXYZ_dtheta[1] += kI * ds * XYZ_cmfs * u;
            dXYZ_dtheta[2] += kI * ds * XYZ_cmfs * u * u;
        }

        glm::dvec3 xyz = XYZ / ctx.W;
        glm::dvec3 fxyz, dfxyz;
        CIELAB_fAndDerivate(xyz[0], fxyz[0], dfxyz[0]);
        CIELAB_fAndDerivate(xyz[1], fxyz[1], dfxyz[1]);
        CIELAB_fAndDerivate(xyz[2], fxyz[2], dfxyz[2]);
        dfxyz /= ctx.W;
        cielab = {
            116 * fxyz[1] - 16,
            500 * (fxyz[0] - fxyz[1]),
            200 * (fxyz[1] - fxyz[2]),
        };
        dLab_dXYZ[0] = { 0, 500 * dfxyz[0], 0 };
        dLab_dXYZ[1] = { 116 * dfxyz[1], -500 * dfxyz[1], 200 * dfxyz[1] };
        dLab_dXYZ[2] = { 0, 0, -200 * dfxyz[2] };

        J = - dLab_dXYZ * dXYZ_dtheta;
    }
};

glm::dvec3 cholesky(const glm::dmat3 &JTJ, const glm::dvec3 &muD, const glm::dvec3 &b) {
    double l00 = std::sqrt(JTJ[0][0] + muD[0]);
    double l10 = JTJ[1][0] / l00;
    double l20 = JTJ[2][0] / l00;
    double l11 = std::sqrt(JTJ[1][1] + muD[1] - l10 * l10);
    double l21 = (JTJ[2][1] - l10 * l20) / l11;
    double l22 = std::sqrt(JTJ[2][2] + muD[2] - l20 * l20 - l21 * l21);

    double y0 = b[0] / l00;
    double y1 = (b[1] - l10 * y0) / l11;
    double y2 = (b[2] - l20 * y0 - l21 * y1) / l22;

    double dtheta2 = y2 / l22;
    double dtheta1 = (y1 - l21 * dtheta2) / l11;
    double dtheta0 = (y0 - l10 * dtheta1 - l20 * dtheta2) / l00;

    return { dtheta0, dtheta1, dtheta2 };
}

Evaluation Optimize(const OptimizeContext &ctx, const LinearRGB &rgb_target, const Evaluation &initial, const OptimizeOptions &options = {}) {
    glm::dvec3 xyz_target = glm::dvec3(ctx.color_space->XYZFromRGB(rgb_target).data) / ctx.W;
    glm::dvec3 cief_target { CIELAB_f(xyz_target[0]), CIELAB_f(xyz_target[1]), CIELAB_f(xyz_target[2]) };
    glm::dvec3 cielab_target {
        116 * cief_target.y - 16,
        500 * (cief_target.x - cief_target.y),
        200 * (cief_target.y - cief_target.z),
    };

    auto current = initial;
    double mu = options.init_mu;
    for (size_t iteration = 0; iteration < options.max_iterations; iteration ++) {
        glm::dvec3 r = cielab_target - current.cielab;
        double rTr = glm::dot(r, r);
        if (rTr < options.residual_epsilon) {
            return current;
        }
        glm::dmat3 JT = glm::transpose(current.J);
        glm::dmat3 JTJ = JT * current.J;
        glm::dvec3 D {
            glm::max(JTJ[0][0], 1e-3),
            glm::max(JTJ[1][1], 1e-3),
            glm::max(JTJ[2][2], 1e-3),
        };
        glm::dvec3 b = -JT * r;

        double v = 2;
        while (true) {
            if (mu > options.max_mu) {
                return current;
            }
            glm::dvec3 dtheta = cholesky(JTJ, mu * D, b);

            glm::dvec3 r_pred = r + current.J * dtheta;
            double dF_pred = rTr - glm::dot(r_pred, r_pred);

            Evaluation candicate {};
            candicate.theta = current.theta + dtheta;
            candicate.eval(ctx);

            glm::dvec3 r_actual = cielab_target - candicate.cielab;
            double dF_actual = rTr - glm::dot(r_actual, r_actual);

            double rho = dF_actual / dF_pred;
            if (rho > 0) {
                current = candicate;
                double t = 2 * rho - 1;
                mu *= glm::max(1.0 / 3.0, 1 - t * t * t);
                break;
            } else {
                mu *= v;
                v *= 2;
            }
        }
    }
    return current;
}

ColorLUT::ColorLUT(const std::filesystem::path &filename, const ColorSpace *color_space, const Spectrum *illuminant)
    : color_space(color_space), illuminant(illuminant) {
    alpha_nodes = new AlphaNodes {};
    data = new LUTData {};

    auto smooth = [](double t) { return t * t * (3 - 2 * t); };
    for (size_t i = 0; i < lut_resolution; i ++) {
        (*alpha_nodes)[i] = smooth(smooth(static_cast<double>(i) / (lut_resolution - 1)));
    }

    if (!load(filename)) {
        genetate();
        save(filename);
    }
}

ColorLUT::~ColorLUT() {
    delete data;
    delete alpha_nodes;
}

SigmoidPolynomialSpectrum ColorLUT::look(const LinearRGB &linear_rgb) const {
    auto color = glm::clamp(linear_rgb.data, 0.f, 1.f);

    size_t i {};
    if (color.r >= color.g && color.r >= color.b) {
        i = 0;
    } else if (color.g >= color.b) {
        i = 1;
    } else {
        i = 2;
    }

    float alpha = color[i];
    if (alpha == 0) {
        return { -std::numeric_limits<float>::infinity(), 0, 0 };
    }

    float x = color[(i + 1) % 3] / alpha;
    float y = color[(i + 2) % 3] / alpha;

    size_t alphai = findAlphaNodeIdx(alpha);
    size_t xi = x == 1 ? lut_resolution - 2 : x * (lut_resolution - 1);
    size_t yi = y == 1 ? lut_resolution - 2 : y * (lut_resolution - 1);

    float alpha_t = (alpha - (*alpha_nodes)[alphai]) / ((*alpha_nodes)[alphai + 1] - (*alpha_nodes)[alphai]);
    float x_t = x * (lut_resolution - 1) - xi;
    float y_t = y * (lut_resolution - 1) - yi;

    auto theta = glm::mix(
        glm::mix(
            glm::mix(at(i, xi, yi, alphai), at(i, xi+1, yi, alphai), x_t),
            glm::mix(at(i, xi, yi+1, alphai), at(i, xi+1, yi+1, alphai), x_t),
            y_t
        ),
        glm::mix(
            glm::mix(at(i, xi, yi, alphai+1), at(i, xi+1, yi, alphai+1), x_t),
            glm::mix(at(i, xi, yi+1, alphai+1), at(i, xi+1, yi+1, alphai+1), x_t),
            y_t
        ),
        alpha_t
    );

    return { theta[0], theta[1], theta[2] };
}

struct ColorLUTHeader {
    uint32_t magic { 0x87d4b1a6 };
    uint32_t resolution { lut_resolution };
};

bool ColorLUT::load(const std::filesystem::path &filename) {
    if (!std::filesystem::exists(filename)) {
        return false;
    }
    std::ifstream file(filename, std::ios::binary);
    ColorLUTHeader header {};
    file.read(reinterpret_cast<char *>(&header), sizeof(ColorLUTHeader));
    if (header.magic != ColorLUTHeader{}.magic) {
        return false;
    }
    if (header.resolution != lut_resolution) {
        return false;
    }
    file.read(reinterpret_cast<char *>(data->data()), sizeof(LUTData));
    return true;
}

void ColorLUT::genetate() {
    PROFILE("Generate Color LUT, resolution = " + std::to_string(lut_resolution))

    auto make_rgb_target = [&](size_t i, float x, float y, size_t alphai) {
        glm::vec3 rgb {};
        float alpha = (*alpha_nodes)[alphai];
        rgb[(i + 0) % 3] = alpha;
        rgb[(i + 1) % 3] = alpha * x;
        rgb[(i + 2) % 3] = alpha * y;
        return LinearRGB { rgb };
    };

    auto store = [&](size_t i, size_t x, size_t y, size_t alpha, const Evaluation &data) {
        (*this->data)[i][x][y][alpha][0] = data.theta[0];
        (*this->data)[i][x][y][alpha][1] = data.theta[1];
        (*this->data)[i][x][y][alpha][2] = data.theta[2];
    };

    OptimizeContext context { color_space, illuminant };

    Progress progress(3 * lut_resolution * lut_resolution * lut_resolution, 5);
    for (size_t i = 0; i < 3; i ++) {
        thread_pool.parallelFor(lut_resolution, lut_resolution, [&](size_t xi, size_t yi) {
            float x = static_cast<float>(xi) / (lut_resolution - 1);
            float y = static_cast<float>(yi) / (lut_resolution - 1);

            size_t k_start = lut_resolution / 5;
            Evaluation initial {};
            initial.eval(context);
            initial = Optimize(context, make_rgb_target(i, x, y, k_start), initial);
            store(i, xi, yi, k_start, initial);

            Evaluation up = initial;
            for (size_t k = k_start + 1; k < lut_resolution; k ++) {
                up = Optimize(context, make_rgb_target(i, x, y, k), up);
                store(i, xi, yi, k, up);
            }

            Evaluation down = initial;
            for (size_t k = k_start; k-- > 0;) {
                down = Optimize(context, make_rgb_target(i, x, y, k), down);
                store(i, xi, yi, k, down);
            }

            progress.update(lut_resolution);
        });
        thread_pool.wait();
    }
}

void ColorLUT::save(const std::filesystem::path &filename) {
    std::ofstream file(filename, std::ios::binary);
    ColorLUTHeader header {};
    file.write(reinterpret_cast<const char *>(&header), sizeof(ColorLUTHeader));
    file.write(reinterpret_cast<const char *>(data->data()), sizeof(LUTData));
}
