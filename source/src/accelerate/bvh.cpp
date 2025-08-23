#include "accelerate/bvh.hpp"
#include "util/debug_macro.hpp"
#include "thread/thread_pool.hpp"
#include <array>
#include <iostream>

void BVH::build(std::vector<Triangle> &&triangles) {
    ordered_triangles = std::move(triangles);

    root = allocator.allocate();
    root->start = 0;
    root->end = ordered_triangles.size();
    root->bounds = {};
    for (const auto &triangle : ordered_triangles) {
        root->bounds.expand(triangle.getBounds());
    }
    root->depth = 1;

    BVHState state {};
    size_t triangle_count = ordered_triangles.size();
    recursiveSplit(root, state);
    thread_pool.wait();

    std::cout << "Total Node Count: " << state.total_node_count << std::endl;
    std::cout << "Leaf Node Count: " << state.leaf_node_count << std::endl;
    std::cout << "Triangle Count: " << triangle_count << std::endl;
    std::cout << "Mean Leaf Node Triangle Count: " << static_cast<float>(triangle_count) / static_cast<float>(state.leaf_node_count) << std::endl;
    std::cout << "Max Leaf Node Triangle Count: " << state.max_leaf_node_triangle_count << std::endl;
    std::cout << "Max Leaf Node Depth: " << state.max_leaf_node_depth << std::endl;

    nodes.reserve(state.total_node_count);
    recursiveFlatten(root);

    area = 0;
    std::vector<float> areas;
    areas.reserve(ordered_triangles.size());
    for (const auto &triangle : ordered_triangles) {
        areas.push_back(triangle.getArea());
        area += areas.back();
    }
    alias_table.build(areas);
}

void BVH::recursiveSplit(BVHTreeNode *node, BVHState &state) {
    state.total_node_count ++;
    if (((node->end - node->start) == 1) || (node->depth > 32)) {
        state.addLeafNode(node);
        return;
    }

    auto diag = node->bounds.diagonal();
    float min_cost = std::numeric_limits<float>::infinity();
    size_t min_split_index = 0;
    Bounds min_child0_bounds {}, min_child1_bounds {};
    size_t min_child0_triangle_count = 0, min_child1_triangle_count = 0;
    constexpr size_t bucket_count = 12;
    for (size_t axis = 0; axis < 3; axis ++) {
        Bounds bounds_buckets[bucket_count] = {};
        size_t triangle_count_buckets[bucket_count] = {};
        for (size_t triangle_idx = node->start; triangle_idx < node->end; triangle_idx ++) {
            Triangle triangle = ordered_triangles[triangle_idx];
            float triangle_center = (triangle.p0[axis] + triangle.p1[axis] + triangle.p2[axis]) / 3.f;
            size_t bucket_idx = glm::clamp<size_t>(
                glm::floor((triangle_center - node->bounds.b_min[axis]) * bucket_count / diag[axis]),
                0, bucket_count - 1
            );
            bounds_buckets[bucket_idx].expand(triangle.p0);
            bounds_buckets[bucket_idx].expand(triangle.p1);
            bounds_buckets[bucket_idx].expand(triangle.p2);
            triangle_count_buckets[bucket_idx] ++;
        }

        Bounds left_bounds = bounds_buckets[0];
        size_t left_triangle_count = triangle_count_buckets[0];
        for (size_t i = 1; i <= bucket_count - 1; i ++) {
            Bounds right_bounds {};
            size_t right_triangle_count = 0;
            for (size_t j = bucket_count - 1; j >= i; j --) {
                right_bounds.expand(bounds_buckets[j]);
                right_triangle_count += triangle_count_buckets[j];
            }
            if (right_triangle_count == 0) {
                break;
            }
            if (left_triangle_count != 0) {
                float cost = left_triangle_count * left_bounds.area() + right_triangle_count * right_bounds.area();
                if (cost < min_cost) {
                    min_cost = cost;
                    node->split_axis = axis;
                    min_split_index = i;
                    min_child0_bounds = left_bounds;
                    min_child1_bounds = right_bounds;
                    min_child0_triangle_count = left_triangle_count;
                    min_child1_triangle_count = right_triangle_count;
                }
            }
            left_bounds.expand(bounds_buckets[i]);
            left_triangle_count += triangle_count_buckets[i];
        }
    }

    if (min_split_index == 0) {
        state.addLeafNode(node);
        return;
    }

    auto *child0 = allocator.allocate();
    auto *child1 = allocator.allocate();
    node->children[0] = child0;
    node->children[1] = child1;

    size_t head_ptr = node->start;
    size_t tail_ptr = node->end - 1;

    while (head_ptr <= tail_ptr) {
        Triangle triangle_head = ordered_triangles[head_ptr];
        auto triangle_center_head = (triangle_head.p0[node->split_axis] + triangle_head.p1[node->split_axis] + triangle_head.p2[node->split_axis]) / 3.f;
        size_t bucket_idx_head = glm::clamp<size_t>(
            glm::floor((triangle_center_head - node->bounds.b_min[node->split_axis]) * bucket_count / diag[node->split_axis]),
            0, bucket_count - 1
        );
        bool head_is_child0 = bucket_idx_head < min_split_index;

        Triangle triangle_tail = ordered_triangles[tail_ptr];
        auto triangle_center_tail = (triangle_tail.p0[node->split_axis] + triangle_tail.p1[node->split_axis] + triangle_tail.p2[node->split_axis]) / 3.f;
        size_t bucket_idx_tail = glm::clamp<size_t>(
            glm::floor((triangle_center_tail - node->bounds.b_min[node->split_axis]) * bucket_count / diag[node->split_axis]),
            0, bucket_count - 1
        );
        bool tail_is_child0 = bucket_idx_tail < min_split_index;

        if (head_is_child0 && tail_is_child0) {
            head_ptr ++;
        } else if ((!head_is_child0) && (!tail_is_child0)) {
            tail_ptr --;
        } else if ((!head_is_child0) && tail_is_child0) {
            std::swap(ordered_triangles[head_ptr], ordered_triangles[tail_ptr]);
            tail_ptr --;
            head_ptr ++;
        } else {
            tail_ptr --;
            head_ptr ++;
        }
    }
    child0->start = node->start;
    child0->end = head_ptr;
    child1->start = child0->end;
    child1->end = node->end;
    node->end = node->start;

    child0->depth = node->depth + 1;
    child1->depth = node->depth + 1;

    child0->bounds = min_child0_bounds;
    child1->bounds = min_child1_bounds;

    if ((child1->end - child0->start) > (128 * 1024)) {
        thread_pool.parallelFor(2, 1, [&, child0, child1](size_t i, size_t) {
            if (i == 0) {
                recursiveSplit(child0, state);
            } else {
                recursiveSplit(child1, state);
            }
        });
    } else {
        recursiveSplit(child0, state);
        recursiveSplit(child1, state);
    }
}

size_t BVH::recursiveFlatten(BVHTreeNode *node) {
    BVHNode bvh_node {
        node->bounds,
        0,
        static_cast<uint16_t>(node->end - node->start),
        static_cast<uint8_t>(node->split_axis),
    };
    auto idx = nodes.size();
    nodes.push_back(bvh_node);
    if (bvh_node.triangle_count == 0) {
        recursiveFlatten(node->children[0]);
        nodes[idx].child1_index = recursiveFlatten(node->children[1]);
    } else {
        nodes[idx].triangle_index = node->start;
    }
    return idx;
}

std::optional<HitInfo> BVH::intersect(const Ray &ray, float t_min, float t_max) const {
    std::optional<HitInfo> closest_hit_info;

    DEBUG_LINE(size_t bounds_test_count = 0, triangle_test_count = 0)

    glm::bvec3 dir_is_neg = {
        ray.direction.x < 0,
        ray.direction.y < 0,
        ray.direction.z < 0,
    };
    glm::vec3 inv_direction = 1.f / ray.direction;

    std::array<int, 32> stack;
    auto ptr = stack.begin();
    size_t current_node_index = 0;

    while (true) {
        auto &node = nodes[current_node_index];
        DEBUG_LINE(bounds_test_count ++)
        if (!node.bounds.hasIntersection(ray, inv_direction, t_min, t_max)) {
            if (ptr == stack.begin()) break;
            current_node_index = *(--ptr);
            continue;
        }

        if (node.triangle_count == 0) {
            if (dir_is_neg[node.split_axis]) {
                *(ptr++) = current_node_index + 1;
                current_node_index = node.child1_index;
            } else {
                current_node_index ++;
                *(ptr++) = node.child1_index;
            }
        } else {
            auto triangle_iter = ordered_triangles.begin() + node.triangle_index;
            DEBUG_LINE(triangle_test_count += node.triangle_count)
            for (size_t i = 0; i < node.triangle_count; i ++) {
                auto hit_info = triangle_iter->intersect(ray, t_min, t_max);
                ++triangle_iter;
                if (hit_info) {
                    t_max = hit_info->t;
                    closest_hit_info = hit_info;
                }
            }
            if (ptr == stack.begin()) break;
            current_node_index = *(--ptr);
        }
    }

    DEBUG_LINE(ray.bounds_test_count += bounds_test_count)
    DEBUG_LINE(ray.triangle_test_count += triangle_test_count)

    return closest_hit_info;
}

std::optional<ShapeSample> BVH::sampleShape(const RNG &rng) const {
    auto sample_result = alias_table.sample(rng.uniform());
    const auto &triangle = ordered_triangles[sample_result.index];
    auto triangle_sample = triangle.sampleShape(rng);
    return ShapeSample { triangle_sample->point, triangle_sample->normal, triangle_sample->pdf * sample_result.prob };
}
