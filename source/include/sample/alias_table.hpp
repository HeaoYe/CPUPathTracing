#pragma once

#include <vector>

class AliasTable {
private:
    struct Item {
        double q;
        union {
            double p;
            size_t alias;
        };
    };

    struct SampleResult {
        size_t index;
        float prob;
    };
public:
    AliasTable() = default;
    void build(const std::vector<float> &values);
    SampleResult sample(float u) const;
    const std::vector<float> &getProbs() const { return probs; }
private:
    std::vector<float> probs;
    std::vector<Item> items;
};
