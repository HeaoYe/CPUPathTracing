#pragma once

#include "bounds.hpp"
#include "shape/triangle.hpp"
#include "sample/alias_table.hpp"
#include "thread/spin_lock.hpp"

struct BVHTreeNode {
    Bounds bounds {};
    size_t start, end;
    BVHTreeNode *children[2];
    size_t depth;
    size_t split_axis;
};

struct alignas(32) BVHNode {
    Bounds bounds {};
    union {
        int child1_index;
        int triangle_index;
    };
    uint16_t triangle_count;
    uint8_t split_axis;
};

struct BVHState {
    std::atomic<size_t> total_node_count {};
    size_t leaf_node_count {};
    size_t max_leaf_node_triangle_count {};
    size_t max_leaf_node_depth {};
    SpinLock spin_lock {};

    void addLeafNode(BVHTreeNode *node) {
        Guard guard(spin_lock);
        leaf_node_count ++;
        max_leaf_node_triangle_count = glm::max(max_leaf_node_triangle_count, node->end - node->start);
        max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);
    }
};

class BVHTreeNodeAllocator {
public:
    BVHTreeNodeAllocator() : ptr(4096) {}

    BVHTreeNode *allocate() {
        Guard guard(spin_lock);
        if (ptr == 4096) {
            nodes_list.push_back(new BVHTreeNode[4096]);
            ptr = 0;
        }
        return &(nodes_list.back()[ptr++]);
    }

    ~BVHTreeNodeAllocator() {
        for (auto *nodes : nodes_list) {
            delete[] nodes;
        }
        nodes_list.clear();
    }
private:
    SpinLock spin_lock {};
    size_t ptr;
    std::vector<BVHTreeNode *> nodes_list;
};

class BVH {
public:
    void build(std::vector<Triangle> &&triangles);
    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const;
    Bounds getBounds() const { return nodes[0].bounds; }
    float getArea() const { return area; }
    std::optional<ShapeSample> sampleShape(const RNG &rng) const;
    float PDF(const glm::vec3 &point, const glm::vec3 &normal) const { return 1.f / getArea(); }
private:
    void recursiveSplit(BVHTreeNode *node, BVHState &state);
    size_t recursiveFlatten(BVHTreeNode *node);
private:
    BVHTreeNodeAllocator allocator {};
    BVHTreeNode *root;
    std::vector<BVHNode> nodes;
    std::vector<Triangle> ordered_triangles;
    float area;
    AliasTable alias_table;
};
