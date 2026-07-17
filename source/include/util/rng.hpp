#pragma once

#include <random>

class PCG32 {
public:
    void setState(uint64_t init_state, uint64_t init_seq) {
        state = 0u;
        inc = (init_seq << 1u) | 1u;
        (*this)();
        state += init_state;
        (*this)();
    }

    uint32_t operator()() {
        uint64_t old_state = state;
        state = old_state * 6364136223846793005ull + inc;
        uint32_t xor_shifted = ((old_state >> 18u) ^ old_state) >> 27u;
        uint32_t rot = old_state >> 59u;
        return (xor_shifted >> rot) | (xor_shifted << ((-rot) & 31));
    }

    static constexpr uint32_t min() { return 0; }
    static constexpr uint32_t max() { return std::numeric_limits<uint32_t>::max(); }
private:
    uint64_t state {}, inc {};
};

class RNG {
public:
    RNG(uint64_t init_state, uint64_t init_seq) { setState(init_state, init_seq); }
    RNG() : RNG(0, 0) {}

    void setState(uint64_t init_state, uint64_t init_seq) { gen.setState(init_state, init_seq); }
    float uniform() const { return uniform_distribution(gen); }
private:
    mutable PCG32 gen;
    mutable std::uniform_real_distribution<float> uniform_distribution { 0, 1 };
};
