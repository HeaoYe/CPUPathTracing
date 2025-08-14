#pragma once

#include <vector>

class AliasTable {
private:
    struct Item {
        float q;
        union {
            float p;
            int alias;
        };
    };

    struct SampleResult {
        int index;
        float prob;
    };
public:
    AliasTable() = default;
    void build(const std::vector<float> &values);
    SampleResult sample(float u) const;
private:
    std::vector<float> probs;
    std::vector<Item> items;
};
