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
    Film film { 1920, 1080 };
    for (int i = 0; i < 100; i ++) {
        for (int j = 0; j < 200; j ++) {
            film.setPixel(j, i, { 0.5, 0.7, 0.2});
        }
    }
    film.save("test.ppm");
    ThreadPool thread_pool {};
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    thread_pool.addTask(new SimpleTask());
    return 0;
}
