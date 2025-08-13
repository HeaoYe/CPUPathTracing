#pragma once

#include "bounds.hpp"
#include "shape/shape.hpp"

struct ShapeInstance {
    const Shape &shape;
    const Material *materail;
    glm::mat4 world_from_object;
    glm::mat4 object_from_world;

    Bounds bounds {};
    glm::vec3 center {};

    void updateBounds() {
        bounds = {};
        auto bounds_object = shape.getBounds();
        for (size_t idx = 0; idx < 8; idx ++) {
            auto corner_object = bounds_object.getCorner(idx);
            glm::vec3 corner_world = world_from_object * glm::vec4(corner_object, 1.f);
            bounds.expand(corner_world);
        }
        center = (bounds.b_max + bounds.b_min) * 0.5f;
    }
};

struct SceneBVHTreeNode {
    Bounds bounds {};
    std::vector<ShapeInstance> instances;
    SceneBVHTreeNode *children[2];
    size_t depth;
    size_t split_axis;

    void updateBounds() {
        bounds = {};
        for (const auto &instance : instances) {
            bounds.expand(instance.bounds);
        }
    }
};

struct alignas(32) SceneBVHNode {
    Bounds bounds {};
    union {
        int child1_index;
        int instance_index;
    };
    uint16_t instance_count;
    uint8_t split_axis;
};

struct SceneBVHState {
    size_t total_node_count {};
    size_t leaf_node_count {};
    size_t max_leaf_node_instance_count {};
    size_t max_leaf_node_depth {};

    void addLeafNode(SceneBVHTreeNode *node) {
        leaf_node_count ++;
        max_leaf_node_instance_count = glm::max(max_leaf_node_instance_count, node->instances.size());
        max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);
    }
};

class SceneBVHTreeNodeAllocator {
public:
    SceneBVHTreeNodeAllocator() : ptr(4096) {}

    SceneBVHTreeNode *allocate() {
        if (ptr == 4096) {
            nodes_list.push_back(new SceneBVHTreeNode[4096]);
            ptr = 0;
        }
        return &(nodes_list.back()[ptr++]);
    }

    ~SceneBVHTreeNodeAllocator() {
        for (auto *nodes : nodes_list) {
            delete[] nodes;
        }
        nodes_list.clear();
    }
private:
    size_t ptr;
    std::vector<SceneBVHTreeNode *> nodes_list;
};

class SceneBVH : public Shape {
public:
    void build(std::vector<ShapeInstance> &&instances);
    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
    Bounds getBounds() const override { return nodes[0].bounds; }
private:
    void recursiveSplit(SceneBVHTreeNode *node, SceneBVHState &state);
    size_t recursiveFlatten(SceneBVHTreeNode *node);
private:
    SceneBVHTreeNodeAllocator allocator {};
    SceneBVHTreeNode *root;
    std::vector<SceneBVHNode> nodes;
    std::vector<ShapeInstance> ordered_instances;
    std::vector<ShapeInstance> infinity_instances;
};
