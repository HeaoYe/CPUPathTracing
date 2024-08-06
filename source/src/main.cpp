#include <iostream>
#include "thread_pool.hpp"
#include "film.hpp"

int main() {
    ThreadPool thread_pool {};

    Film film { 1920, 1080 };

    thread_pool.parallelFor(200, 100, [&](size_t x, size_t y) {
        film.setPixel(x, y, { 0.5, 0.7, 0.2 });
    });

    thread_pool.wait();
    film.save("test.ppm");
    return 0;
}
