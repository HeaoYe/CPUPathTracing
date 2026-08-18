#include "camera/camera.hpp"
#include "shape/sphere.hpp"
#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "image/image.hpp"
#include "material/diffuse_material.hpp"
#include "material/specular_material.hpp"
#include "material/dielectric_material.hpp"
#include "material/conductor_material.hpp"
#include "material/ground_material.hpp"
#include "light/uniform_infinite_light.hpp"
#include "light/image_infinite_light.hpp"
#include "renderer/path_tracing_renderer.hpp"
#include "renderer/simple_path_tracing_renderer.hpp"
#include "renderer/previewer.hpp"
#include "sample/spherical.hpp"

#include "spectrum/constant_spectrum.hpp"
#include "spectrum/piecewise_linear_spectrum.hpp"
#include "spectrum/densely_sampled_spectrum.hpp"
#include "spectrum/rgb_illuminant_spectrum.hpp"
#include "spectrum/illuminant_spectrum.hpp"
#include "spectrum/illuminant.hpp"
#include "color/color_space.hpp"
#include "color/color_lut.hpp"

int main() {
    InitColorSpace();

    Film film { 192 * 10, 108 * 10 };
    Camera camera { film, { 0, 1.25, -10 }, { 0, 3.95, 2 }, 48 };

    Scene scene {};

    Model buddha("models/buddha.obj");
    Model dragon("models/dragon_871k.obj");

    ConstantSpectrum constant_1 { 1 };

    auto copper_eta = PiecewiseLinearSpectrum::LoadCSV(
        "spectrums/Johnson-copper.csv", "wl", 1e3, "n");
    auto copper_k = PiecewiseLinearSpectrum::LoadCSV(
        "spectrums/Johnson-copper.csv", "wl", 1e3, "k");
    ConductorMaterial copper { &copper_eta, &copper_k, 0.5, 0.42 };
    auto glass_eta = PiecewiseLinearSpectrum::LoadCSV(
        "spectrums/Zelmon-glass.csv", "wl", 1e3, "n");
    auto glass_color = ColorLUT_sRGB->look(210, 210, 184);
    DielectricMaterial glass { &glass_eta, &glass_color, 0.21, 0.08 };

    scene.addShape(buddha, &copper, { -5.0, 1.78, 1.7 }, { 4, 4, 4 });
    scene.addShape(buddha, &glass, { -2.1, 1.78, 1.7 }, { 4, 4, 4 });
    scene.addShape(buddha, &glass, { 2.1, 1.78, 1.7 }, { 4, 4, 4 });
    scene.addShape(buddha, &copper, { 5.0, 1.78, 1.7 }, { 4, 4, 4 });

    Sphere sphere { { 0, 0, 0 }, 0.45 };
    PiecewiseLinearSpectrum sphere_eta {
        {
            {
                { 360, 1.60 },
                { 400, 1.57 },
                { 525, 1.52 },
                { 650, 1.48 },
                { 830, 1.45 },
            }
        }
    };
    DielectricMaterial sphere_mat { &sphere_eta, &constant_1 };
    scene.addShape(sphere, &sphere_mat, { -1, 0.55, -1.65 });
    scene.addShape(sphere, &sphere_mat, { 1, 0.55, -1.65 });

    auto metameric_A_reflectance = DenselySampledSpectrum::LoadCSV(
        "spectrums/Metameric_A_reflectance.csv", "wavelength_nm", "reflectance");
    DiffuseMaterial metameric_A { &metameric_A_reflectance };
    auto metameric_B_reflectance = DenselySampledSpectrum::LoadCSV(
        "spectrums/Metameric_B_reflectance.csv", "wavelength_nm", "reflectance");
    DiffuseMaterial metameric_B { &metameric_B_reflectance };

    scene.addShape(dragon, &metameric_A, { -4.3, 0.7, -1.65 }, { 2, 2, 2 }, { 0, -90, 0 });
    scene.addShape(dragon, &metameric_B, { -2.4, 0.7, -1.65 }, { 2, 2, 2 }, { 0, -90, 0 });
    scene.addShape(dragon, &metameric_B, { 2.4, 0.7, -1.65 }, { 2, 2, 2 }, { 0, 90, 0 });
    scene.addShape(dragon, &metameric_A, { 4.3, 0.7, -1.65 }, { 2, 2, 2 }, { 0, 90, 0 });

    constexpr size_t dragon_count = 1200;

    std::vector<SigmoidPolynomialSpectrum> reflectance_spctral;
    reflectance_spctral.reserve(dragon_count);
    RNG rng {};
    rng.setState(dragon_count, 0);

    for (size_t i = 0; i < dragon_count; i ++) {
        reflectance_spctral.push_back(ColorLUT_sRGB->look(LinearRGB(rng.uniform(), rng.uniform(), rng.uniform())));

        auto disk = UniformSampleUnitDisk({ rng.uniform(), rng.uniform() }) * 12.f;

        scene.addShape(
            dragon,
            new DiffuseMaterial { &reflectance_spctral.back() },
            { disk.x, glm::abs(disk.y), 4 },
            { 1, 1, 1 },
            { rng.uniform() * 360, rng.uniform() * 360, rng.uniform() * 360 }
        );
    }

    Plane ground {
        { 0, 0, 0 },
        { 0, 1, 0 },
        100,
    };
    scene.addShape(ground, new GroundMaterial { &constant_1 });

    scene.addInfiniteLight(new ImageInfiniteLight {
        { "hdris/kloppenheim_07_puresky_4k.exr" }, 80
    });
    // RGBIlluminantSpectrum illumt { ColorLUT_sRGB, LinearRGB { 1, 0.9, 0.8} };
    // scene.addInfiniteLight(new UniformInfiniteLight { &illumt });

    scene.build();

    PathTracingRenderer path_tracing_renderer { camera, scene };
    Previewer previewer(path_tracing_renderer);
    if (previewer.preview()) {
        path_tracing_renderer.render(64, "RGB2SPECTRAL_TEST_64.exr", ColorSpace_sRGB);
    }

    return 0;
}

// Debug Mode
// load model 100ms
// parallelFor 700ms
// Film::save 130ms
// render 1spp normal.ppm 5504ms
// render 128spp test.ppm 1222161ms

// Release Mode
// load model 19ms
// parallelFor 400ms
// Film::save 53ms
// render 1spp normal.ppm 1189ms
// render 128spp test.ppm 91556ms

// Change parallelFor
// parallelFor 0ms

// Change Film::save
// Film::save 2ms

// Add Bounds
// load model 29ms
// render 1spp normal.ppm 156ms
// render 128spp test.ppm 30682ms

// Use rapidobj
// load model 2ms

// Add BVH
// render 1spp normal.ppm 146ms
// render 128spp test.ppm 5025ms

// Dragon 87k
// "origin" render 128spp test.ppm 3822ms
// "flatten" render 128spp test.ppm 3867ms
// "triangle index" render 128spp test.ppm 4009ms
// "32bit pack" render 128spp test.ppm 3928ms
// "inv_direction" render 128spp test.ppm 3752ms
// "SAH" render 128spp test.ppm 3865ms
// "871k dragon" render 128spp test.ppm 4032ms

// Without SAH
// Total Node Count: 142643
// Leaf Node Count: 71322
// Triangle Count: 87130
// Mean Leaf Node Triangle Count: 1.22164
// Max Leaf Node Triangle Count: 23
// Profile "Load model models/dragon_87k.obj": 2113ms

// With SAH
// Total Node Count: 164067
// Leaf Node Count: 82034
// Triangle Count: 87130
// Mean Leaf Node Triangle Count: 1.06212
// Max Leaf Node Triangle Count: 8
// Profile "Load model models/dragon_87k.obj": 16023ms

// With 3Dim SAH
// Total Node Count: 174195
// Leaf Node Count: 87098
// Triangle Count: 87130
// Mean Leaf Node Triangle Count: 1.00037
// Max Leaf Node Triangle Count: 3
// Profile "Load model models/dragon_87k.obj": 51655ms

// 871k Dragon BVH
// Total Node Count: 1742423
// Leaf Node Count: 871212
// Triangle Count: 871306
// Mean Leaf Node Triangle Count: 1.00011
// Max Leaf Node Triangle Count: 3
// Profile "Load model models/dragon_871k.obj": 519741ms

// 871k Dragon BVH 12 Buckets
// Total Node Count: 1742425
// Leaf Node Count: 871213
// Triangle Count: 871306
// Mean Leaf Node Triangle Count: 1.00011
// Max Leaf Node Triangle Count: 3
// Profile "Load model models/dragon_871k.obj": 26482ms

// 871k Dragon BVH Change ThreadPool
// Profile "Load model models/dragon_871k.obj": 14564ms

// 871k Dragon BVH Add Allocator
// Profile "Load model models/dragon_871k.obj": 8328ms

// 871k Dragon BVH Vector Reserve
// Profile "Load model models/dragon_871k.obj": 6140ms

// render 100 shape instances scene
// Profile "Render 128spp test.ppm": 18613ms

// render 100 shape instances scene with SceneBVH
// Profile "Render 128spp test.ppm": 7628ms

// non thread local RNG
// render 128spp：9827ms

// thread local RNG
// render 128spp：2917ms

// before optimize bvh
// Load buddha.obj 1084k：7871ms

// remove triangle indices cache
// Load buddha.obj 1084k：2807ms

// order triangle list
// Load buddha.obj 1084k：1337ms

// parallel build bvh
// Load buddha.obj 1084k：587ms

// mt19937
// render 32spp: 17706ms

// PCG32
// render 32spp: 12647ms

// before optimize:
// render 32spp: 16714ms
// Total Node Count: 2174623
// Leaf Node Count: 1087312
// Triangle Count: 1087474
// Mean Leaf Node Triangle Count: 1.00015
// Max Leaf Node Triangle Count: 4
// Max Leaf Node Depth: 28
// Profile "Load model models/buddha.obj": 671ms

// after optimize:
// render 32spp: 13957ms
// Total Node Count: 725607
// Leaf Node Count: 362804
// Triangle Count: 1087474
// Mean Leaf Node Triangle Count: 2.99741
// Max Leaf Node Triangle Count: 8
// Max Leaf Node Depth: 26
// Profile "Load model models/buddha.obj": 375ms
