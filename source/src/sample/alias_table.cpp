#include "sample/alias_table.hpp"
#include <glm/glm.hpp>

void AliasTable::build(const std::vector<float> &values) {
    float sum = 0;
    for (float value : values) {
        sum += value;
    }

    probs.resize(values.size());
    items.resize(values.size());
    std::vector<size_t> less, greater;
    for (size_t i = 0; i < values.size(); i ++) {
        probs[i] = values[i] / sum;

        items[i].q = 1;
        items[i].p = probs[i] * items.size();

        if (items[i].p < 1) {
            less.push_back(i);
        } else if (items[i].p > 1) {
            greater.push_back(i);
        }
    }

    while ((!less.empty()) && (!greater.empty())) {
        auto &item_l = items[less.back()];
        auto &item_g = items[greater.back()];
        size_t item_g_idx = greater.back();
        less.pop_back();
        greater.pop_back();

        item_l.q = item_l.p;
        item_l.alias = item_g_idx;
        item_g.p -= 1.f - item_l.q;

        if (item_g.p < 1) {
            less.push_back(item_g_idx);
        } else if (item_g.p > 1) {
            greater.push_back(item_g_idx);
        }
    }
}

AliasTable::SampleResult AliasTable::sample(float u) const {
    int idx = glm::floor(glm::clamp<int>(u * items.size(), 0, items.size() - 1));
    u = glm::clamp<float>(u * items.size() - idx, 0, 1);
    const auto &item = items[idx];
    if (u <= item.q) {
        return SampleResult { idx, probs[idx] };
    }
    return { item.alias, probs[item.alias] };
}
