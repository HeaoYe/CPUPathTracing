#pragma once

#include "thread/spin_lock.hpp"

class Progress {
public:
    Progress(size_t total, size_t step = 1);

    void update(size_t count);
private:
    size_t total, current;
    int percent, last_percent, step;
    SpinLock spin_lock;
};
