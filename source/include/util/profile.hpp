#pragma once

#include <chrono>

#define PROFILE(name) Profile __profile(name);

struct Profile {
    Profile(const std::string &name);
    ~Profile();

    std::string name;
    std::chrono::high_resolution_clock::time_point start;
};
