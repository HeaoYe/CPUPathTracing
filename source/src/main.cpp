#include <iostream>
#include "thread_pool.hpp"
#include "film.hpp"

class SimpleTask : public Task {
public:
    void run() override {
        std::cout << "Hello World !" << std::endl;
    }
};

int main() {
    ThreadPool thread_pool {};

    Film film { 1920, 1080 };
    thread_pool.parallelFor(100, 200, [&](size_t x, size_t y) {
        film.setPixel(x, y, { 0.5, 0.7, 0.2 });
    });
    thread_pool.wait();
    film.save("test.ppm");

    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    return 0;
}
