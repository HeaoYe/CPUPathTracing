#pragma once

#include "renderer.hpp"
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class Previewer {
public:
    Previewer(Renderer renderer, float fps = 30);
    bool preview();
private:
    void renderFrame();
    void setResolution(float scale);
    void adjustResolution(float dt);
private:
    Renderer renderer;
    std::vector<Renderer> render_modes;
    size_t render_mode_idx = 0;

    float scale;
    float fps;
    glm::ivec2 film_resolution;

    size_t current_spp = 0;

    std::shared_ptr<sf::RenderWindow> window;
    std::shared_ptr<sf::Texture> texture;
    std::shared_ptr<sf::Sprite> sprite;
};
