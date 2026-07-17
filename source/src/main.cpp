#include "camera/camera.hpp"
#include "shape/sphere.hpp"
#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "util/rgb.hpp"
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

int main() {
    Film film { 192 * 10, 108 * 10 };
    Camera camera { film, { 0, 1.25, -6 }, { 0, 1.95, 0 }, 45 };

    Scene scene {};

    Model model("models/buddha.obj");
    scene.addShape(
        model,
        new SpecularMaterial { RGB(241, 191, 79) },
        { -3, 1.75, 0 },
        { 4, 4, 4 }
    );
    scene.addShape(
        model,
        new ConductorMaterial {
            { 1.2, 1.2, 5.3 },
            { 3.4, 3.4, 2.1 },
            0.8, 0.2
        },
        { -1, 1.75, 0 },
        { 4, 4, 4 }
    );
    scene.addShape(
        model,
        new DielectricMaterial {
            1.4,
            { 1, 1, 1 },
            RGB(180, 180, 154),
            0.1, 0.3
        },
        { 1, 1.75, 0 },
        { 4, 4, 4 }
    );
    scene.addShape(
        model,
        new DiffuseMaterial { RGB(241, 191, 79) },
        { 3, 1.75, 0 },
        { 4, 4, 4 }
    );

    Sphere sphere {
        { 0, 0, 0 },
        1
    };
    scene.addShape(sphere, new SpecularMaterial { { 1, 1, 1 } }, { 0, 3.75, 3 });

    Plane ground {
        { 0, 0, 0 },
        { 0, 1, 0 },
        100
    };
    scene.addShape(ground, new GroundMaterial { { 1, 1, 1 } });

    // Image env_image { "hdris/HdrOutdoorSnowMountainsEveningClear001_HDR_4K.exr" };
    // Image env_image { "hdris/qwantani_night_puresky_4k.exr" };
    Image env_image { "hdris/kloppenheim_07_puresky_4k.exr" };
    scene.addInfiniteLight(new ImageInfiniteLight { &env_image });

    scene.build();

    PathTracingRenderer path_tracing_renderer { camera, scene };
    Previewer previewer(path_tracing_renderer);
    if (previewer.preview()) {
        path_tracing_renderer.render(32, "PT_MIS_TEST.exr");
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
