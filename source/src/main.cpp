#include "camera/camera.hpp"
#include "shape/sphere.hpp"
#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "util/rgb.hpp"
#include "material/diffuse_material.hpp"
#include "material/specular_material.hpp"
#include "material/dielectric_material.hpp"
#include "material/conductor_material.hpp"
#include "material/ground_material.hpp"
#include "renderer/path_tracing_renderer.hpp"
#include "renderer/simple_path_tracing_renderer.hpp"
#include "renderer/previewer.hpp"

int main() {
    Film film { 192 * 10, 108 * 10 };
    glm::vec3 camera_pos = { 0, 37, -61 };
    Camera camera { film, camera_pos, { 0, 8, 0 }, 16 };

    Scene scene {};
    Triangle triangles[] = {
        { { -17, 0, -1.5 }, { -17, 0, 1.5 }, { 17, 0, 1.5 } },
        { { -17, 0, -1.5 }, { 17, 0, 1.5 }, { 17, 0, -1.5 } },
    };
    Sphere light_sphere_1 { { -15 + 00 / 3, 12, 8 }, 2 };
    Sphere light_sphere_2 { { -15 + 30 / 3, 12, 8 }, 1 };
    Sphere light_sphere_3 { { -15 + 60 / 3, 12, 8 }, 0.5 };
    Sphere light_sphere_4 { { -15 + 90 / 3, 12, 8 }, 0.1 };
    AreaLight area_light_1 { light_sphere_1, { 1, 1, 1 }, false };
    AreaLight area_light_2 { light_sphere_2, { 4, 4, 4 }, false };
    AreaLight area_light_3 { light_sphere_3, { 16, 16, 16 }, false };
    AreaLight area_light_4 { light_sphere_4, { 400, 400, 400 }, false };
    scene.addAreaLight(&area_light_1, new DiffuseMaterial {});
    scene.addAreaLight(&area_light_2, new DiffuseMaterial {});
    scene.addAreaLight(&area_light_3, new DiffuseMaterial {});
    scene.addAreaLight(&area_light_4, new DiffuseMaterial {});
    glm::vec3 light_pos_center = { 0, 12, 8 };

    float alphas[] = { 0.4, 0.25, 0.16, 0.04 };
    for (size_t i = 0; i < 4; i ++) {
        float theta = glm::radians(i * 15.f);
        glm::vec3 center { 0, 17 * (1 - glm::cos(theta)), 17 * glm::sin(theta) };
        glm::vec3 normal = glm::normalize(glm::normalize(light_pos_center - center) + glm::normalize(camera_pos - center));
        float rotate_x = -glm::degrees(glm::acos(normal.y));
        ConductorMaterial *surface_material = new ConductorMaterial { { 2, 2, 1 }, { 3, 3, 15 }, alphas[i], alphas[i] };
        scene.addShape(triangles[0], surface_material, center, { 1, 1, 1 }, { rotate_x, 0, 0 });
        scene.addShape(triangles[1], surface_material, center, { 1, 1, 1 }, { rotate_x, 0, 0 });
    }

    Plane ground {
        { 0, -0.5, 0 },
        { 0, 1, 0 },
        100
    };
    Plane wall {
        { 0, 0, 15 },
        { 0, 0, -1 },
        100
    };
    scene.addShape(ground, new GroundMaterial { { 1, 1, 1 } });
    scene.addShape(wall, new DiffuseMaterial { { 1, 1, 1 } });
    scene.addInfiniteLight(new InfiniteLight { { 0.5, 0.5, 0.5 } });

    scene.build();

    PathTracingRenderer path_tracing_renderer { camera, scene };
    Previewer previewer(path_tracing_renderer);
    if (previewer.preview()) {
        path_tracing_renderer.render(32, "PT_MIS_TEST.ppm");
    }
    // SimplePathTracingRenderer simple_path_tracing_renderer { camera, scene };
    // simple_path_tracing_renderer.render(256, "PT_MIS_TEST_SAMPLE_LIGHT.ppm");

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
