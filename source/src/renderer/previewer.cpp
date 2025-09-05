#include "renderer/previewer.hpp"
#include "renderer/normal_renderer.hpp"
#include "renderer/debug_renderer.hpp"
#include "thread/thread_pool.hpp"

Previewer::Previewer(Renderer renderer, float fps) : renderer(renderer), fps(fps) {
    auto &film = renderer.getCamera().getFilm();
    film_resolution = { film.getWidth(), film.getHeight() };

    render_modes.push_back(renderer);
    render_modes.push_back({ new NormalRenderer(renderer.getCamera(), renderer.getScene()) });
    DEBUG_LINE(render_modes.push_back({ new BoundsTestCountRenderer(renderer.getCamera(), renderer.getScene()) }));
    DEBUG_LINE(render_modes.push_back({ new TriangleTestCountRenderer(renderer.getCamera(), renderer.getScene()) }));

    scale = 1;
}

bool Previewer::preview() {
    window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode(sf::Vector2u(film_resolution.x, film_resolution.y)),
        "Previewer",
        sf::Style::None,
        sf::State::Windowed
    );
    texture = std::make_shared<sf::Texture>(sf::Vector2u(film_resolution.x, film_resolution.y));
    texture->setSmooth(true);
    sprite = std::make_shared<sf::Sprite>(*texture);
    setResolution(0.1);
    auto &camera = renderer.getCamera();
    auto &film = camera.getFilm();

    bool grabbed = false;
    sf::Vector2i center { window->getSize() / 2u };
    float dt = 0;
    bool render_final_result = false;

    while (window->isOpen()) {
        while (auto event = window->pollEvent()) {
            if (auto key_released = event->getIf<sf::Event::KeyReleased>()) {
                if (key_released->scancode == sf::Keyboard::Scancode::Escape) {
                    window->close();
                } else if (key_released->scancode == sf::Keyboard::Scancode::Enter) {
                    window->close();
                    render_final_result = true;
                } else if (key_released->scancode == sf::Keyboard::Scancode::Tab) {
                    render_mode_idx = (render_mode_idx + 1) % render_modes.size();
                    current_spp = 0;
                } else if (key_released->scancode == sf::Keyboard::Scancode::Equal) {
                    fps += 1;
                    printf("FPS: %f\n", fps);
                } else if (key_released->scancode == sf::Keyboard::Scancode::Hyphen) {
                    fps -= 1;
                    if (fps <= 0) {
                        fps = 1;
                    }
                    printf("FPS: %f\n", fps);
                } else if (key_released->scancode == sf::Keyboard::Scancode::CapsLock) {
                    grabbed = !grabbed;
                    window->setMouseCursorGrabbed(grabbed);
                    window->setMouseCursorVisible(!grabbed);
                    sf::Mouse::setPosition(center, *window);
                }
            } else if (auto *mouse_moved = event->getIf<sf::Event::MouseMoved>()) {
                if (!grabbed) {
                    continue;
                }
                auto delta = mouse_moved->position - center;
                if (delta.x == 0 && delta.y == 0) {
                    continue;
                }
                camera.turn({ delta.x, delta.y });
                current_spp = 0;
                sf::Mouse::setPosition(center, *window);
            } else if (auto *mouse_wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (!grabbed) {
                    continue;
                }
                camera.zoom(mouse_wheel->delta);
                current_spp = 0;
            }
        }

        if (grabbed) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                camera.move(dt, Direction::Forward);
                current_spp = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                camera.move(dt, Direction::Backward);
                current_spp = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                camera.move(dt, Direction::Left);
                current_spp = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                camera.move(dt, Direction::Right);
                current_spp = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                camera.move(dt, Direction::Up);
                current_spp = 0;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
                camera.move(dt, Direction::Down);
                current_spp = 0;
            }
        }

        auto start = std::chrono::high_resolution_clock::now();
        renderFrame();
        auto duration = std::chrono::high_resolution_clock::now() - start;

        auto buffer = film.generateRGBABuffer();
        texture->update(buffer.data());

        window->clear();
        window->draw(*sprite);
        window->display();

        dt = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() * 0.001f;
        adjustResolution(dt);
    }

    film.setResolution(film_resolution.x, film_resolution.y);
    camera.print();
    return render_final_result;
}

void Previewer::renderFrame() {
    auto current_renderer = render_modes[render_mode_idx];
    size_t render_spp = render_mode_idx == 0 ? 4 : 1;
    auto &film = current_renderer.getCamera().getFilm();

    if (current_spp == 0) {
        film.clear();
    }

    thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        for (size_t i = current_spp; i < current_spp + render_spp; i ++) {
            film.addSample(x, y, current_renderer.renderPixel({ x, y, i }));
        }
    });
    thread_pool.wait();
    current_spp += render_spp;
}

void Previewer::setResolution(float scale) {
    scale = glm::clamp(scale, 0.f, 1.f);
    if (scale == this->scale) {
        return;
    }
    this->scale = scale;
    glm::ivec2 resolution = scale * glm::vec2(film_resolution);
    if (resolution.x == 0) resolution.x = 1;
    if (resolution.y == 0) resolution.y = 1;
    renderer.getCamera().getFilm().setResolution(resolution.x, resolution.y);
    auto res = texture->resize(sf::Vector2u(resolution.x, resolution.y));
    sprite->setTexture(*texture);
    sprite->setScale(sf::Vector2f(static_cast<float>(film_resolution.x) / resolution.x, static_cast<float>(film_resolution.y) / resolution.y));
    current_spp = 0;
}

void Previewer::adjustResolution(float dt) {
    float expected_dt = 1.f / fps;
    if (glm::abs(expected_dt - dt) / expected_dt > 0.4f) {
        float new_scale = scale * (1.f + 0.1f * (glm::sqrt(expected_dt / dt) - 1.f));
        setResolution(new_scale);
    }
}
