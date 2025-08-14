#include "camera/camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(Film &film, const glm::vec3 &pos, const glm::vec3 &viewpoint, float fovy)
    : film(film), pos(pos), fovy(fovy) {
    view_direction = glm::normalize(viewpoint - pos);
    update();
    theta = glm::degrees(glm::acos(view_direction.y));
    if (glm::abs(view_direction.y) == 1) {
        phi = 0;
    } else {
        phi = glm::degrees(glm::acos(view_direction.x / glm::sqrt(view_direction.x * view_direction.x + view_direction.z * view_direction.z)));
    }
}

Ray Camera::generateRay(const glm::ivec2 &pixel_coord, const glm::vec2 &offset) const {
    glm::vec2 ndc = (glm::vec2(pixel_coord) + offset) / glm::vec2(film.getWidth(), film.getHeight());
    ndc.y = 1.f - ndc.y;
    // [0, 1] --> [-1, 1]
    ndc = 2.f * ndc - 1.f;
    glm::vec4 clip { ndc, 0, 1 };
    glm::vec3 world = world_from_camera * camera_from_clip * clip;
    return Ray {
        pos,
        glm::normalize(world - pos)
    };
}

void Camera::move(float dt, Direction direction) {
    glm::vec3 forward = view_direction;
    forward.y = 0;
    forward = glm::normalize(forward);
    glm::vec3 move_direction {};

    switch (direction) {
    case Direction::Forward:
        move_direction = forward;
        break;
    case Direction::Backward:
        move_direction = -forward;
        break;
    case Direction::Left:
        move_direction = glm::cross(forward, glm::vec3(0, 1, 0));
        break;
    case Direction::Right:
        move_direction = -glm::cross(forward, glm::vec3(0, 1, 0));
        break;
    case Direction::Up:
        move_direction = glm::vec3(0, 1, 0);
        break;
    case Direction::Down:
        move_direction = glm::vec3(0, -1, 0);
        break;
    }

    pos += dt * move_speed * move_direction;
    update();
}

void Camera::turn(const glm::vec2 &delta) {
    phi -= delta.x * turn_speed.x;
    if (phi > 360) phi -= 360;
    if (phi < 0) phi += 360;
    theta += delta.y * turn_speed.y;
    theta = glm::clamp(theta, -179.f, 179.f);

    float sin_theta = glm::sin(glm::radians(theta));
    float cos_theta = glm::cos(glm::radians(theta));
    float sin_phi = glm::sin(glm::radians(phi));
    float cos_phi = glm::cos(glm::radians(phi));
    view_direction = {
        sin_theta * cos_phi,
        cos_theta,
        sin_theta * sin_phi
    };

    update();
}

void Camera::zoom(float delta) {
    fovy = glm::clamp(fovy - delta, 1.f, 179.f);
    update();
}

void Camera::print() {
    auto viewpoint = pos + view_direction;
    printf("Camera:\n");
    printf("\tFilm Resolution: (%lld, %lld)\n", film.getWidth(), film.getHeight());
    printf("\tPosition: (%f, %f, %f)\n", pos.x, pos.y, pos.z);
    printf("\tViewpoint: (%f, %f, %f)\n", viewpoint.x, viewpoint.y, viewpoint.z);
    printf("\tFovy: %f\n", fovy);
}

void Camera::update() {
    auto viewpoint = pos + view_direction;
    camera_from_clip = glm::inverse(glm::perspective(
        glm::radians(fovy),
        static_cast<float>(film.getWidth()) / static_cast<float>(film.getHeight()),
        1.f, 2.f
    ));
    world_from_camera = glm::inverse(glm::lookAt(pos, viewpoint, { 0, 1, 0 }));
}
