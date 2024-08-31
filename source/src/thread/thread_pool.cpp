#include "thread/thread_pool.hpp"
#include <cmath>

ThreadPool thread_pool {};

void ThreadPool::WorkerThread(ThreadPool *master) {
    while (master->alive == 1) {
        if (master->tasks.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue;
        }
        Task *task = master->getTask();
        if (task != nullptr) {
            task->run();
            delete task;
            master->pending_task_count --;
        } else {
            std::this_thread::yield();
        }
    }
}

ThreadPool::ThreadPool(size_t thread_count) {
    alive = 1;
    pending_task_count = 0;
    if (thread_count == 0) {
        thread_count = std::thread::hardware_concurrency();
    }
    for (size_t i = 0; i < thread_count; i ++) {
        threads.push_back(std::thread(ThreadPool::WorkerThread, this));
    }
}

ThreadPool::~ThreadPool() {
    wait();
    alive = 0;
    for (auto &thread : threads) {
        thread.join();
    }
    threads.clear();
}

class ParallelForTask : public Task {
public:
    ParallelForTask(size_t x, size_t y, size_t chunk_width, size_t chunk_height, const std::function<void(size_t, size_t)> &lambda)
        : x(x), y(y), chunk_width(chunk_width), chunk_height(chunk_height), lambda(lambda) {}

    void run() override {
        for (size_t idx_x = 0; idx_x < chunk_width; idx_x ++) {
            for (size_t idx_y = 0; idx_y < chunk_height; idx_y ++) {
                lambda(x + idx_x, y + idx_y);
            }
        }
    }
private:
    size_t x, y, chunk_width, chunk_height;
    std::function<void(size_t, size_t)> lambda;
};

void ThreadPool::parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda, bool complex) {
    Guard guard(spin_lock);

    float chunk_width_float = static_cast<float>(width) / std::sqrt(threads.size());
    float chunk_height_float = static_cast<float>(height) / std::sqrt(threads.size());
    if (complex) {
        chunk_width_float /= std::sqrt(16);
        chunk_height_float /= std::sqrt(16);
    }
    size_t chunk_width = std::ceil(chunk_width_float);
    size_t chunk_height = std::ceil(chunk_height_float);

    for (size_t x = 0; x < width; x += chunk_width) {
        for (size_t y = 0; y < height; y += chunk_height) {
            pending_task_count ++;
            if (x + chunk_width > width) {
                chunk_width = width - x;
            }
            if (y + chunk_height > height) {
                chunk_height = height - y;
            }
            tasks.push(new ParallelForTask(x, y, chunk_width, chunk_height, lambda));
        }
    }
}

void ThreadPool::wait() const {
    while (pending_task_count > 0) {
        std::this_thread::yield();
    }
}

void ThreadPool::addTask(Task *task) {
    Guard guard(spin_lock);
    pending_task_count ++;
    tasks.push(task);
}

Task *ThreadPool::getTask() {
    Guard guard(spin_lock);
    if (tasks.empty()) {
        return nullptr;
    }
    Task *task = tasks.front();
    tasks.pop();
    return task;
}
