#pragma once

#include "spin_lock.hpp"
#include <functional>
#include <vector>
#include <thread>
#include <list>

class Task {
public:
    virtual void run() = 0;
};

class ThreadPool {
public:
    static void WorkerThread(ThreadPool *master);

    ThreadPool(size_t thread_count = 0);
    ~ThreadPool();

    void parallelFor(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda);
    void wait() const;

    void addTask(Task *task);
    Task *getTask();
private:
    std::atomic<int> alive;
    std::vector<std::thread> threads;
    std::list<Task *> tasks;
    SpinLock spin_lock {};
};
