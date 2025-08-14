#pragma once

#include "film.hpp"
#include "ray.hpp"

enum class Direction {
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down,
};

class Camera {
public:
    Camera(Film &film, const glm::vec3 &pos, const glm::vec3 &viewpoint, float fovy);

    Ray generateRay(const glm::ivec2 &pixel_coord, const glm::vec2 &offset = { 0.5, 0.5 }) const;

    Film &getFilm() { return film; }
    const Film &getFilm() const { return film; }

    void move(float dt, Direction direction);
    void turn(const glm::vec2 &delta);
    void zoom(float delta);

    void print();
private:
    void update();
private:
    Film &film;
    glm::vec3 pos;

    float fovy;
    glm::vec3 view_direction;
    float theta, phi;
    float move_speed = 2;
    glm::vec2 turn_speed { 0.15, 0.07 };

    glm::mat4 camera_from_clip;
    glm::mat4 world_from_camera;
};
