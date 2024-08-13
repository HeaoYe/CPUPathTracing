#pragma once

#include <random>

class RNG {
public:
    RNG(size_t seed) { setSeed(seed); }
    RNG() : RNG(0) {}

    void setSeed(size_t seed) { gen.seed(seed); }
    float uniform() { return uniform_distribution(gen); }
private:
    std::mt19937 gen;
    std::uniform_real_distribution<float> uniform_distribution { 0, 1 };
};
