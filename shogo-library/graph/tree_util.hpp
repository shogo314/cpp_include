#pragma once

/**
 * @file tree_util.hpp
 * @brief 木に関する関数
 */

#include <algorithm>
#include <cassert>
#include <map>
#include <utility>
#include <vector>

#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/graph/tree_util.hpp

namespace internal_tree_util {

// s から各頂点への距離と、各頂点へ向かう辺 (s では Edge()) を DFS で求める
template <typename Cost>
std::pair<std::vector<Cost>, std::vector<typename Tree<Cost>::Edge>> tree_distances(const Tree<Cost>& tree, long long s, bool weighted) {
    using Edge = typename Tree<Cost>::Edge;
    long long n = tree.size();
    std::vector<Cost> dist(n);
    std::vector<Edge> prev(n);
    std::vector<long long> stk = {s};
    while (!stk.empty()) {
        long long u = stk.back();
        stk.pop_back();
        for (const auto& e : tree[u]) {
            if (e.id == prev[u].id) continue;
            prev[e.dst] = e;
            dist[e.dst] = dist[u] + (weighted ? e.cost : Cost{1});
            stk.push_back(e.dst);
        }
    }
    return {dist, prev};
}

}  // namespace internal_tree_util

/**
 * @brief 木の直径
 *
 * @param tree 木 (頂点数 1 以上)
 * @param weighted = true 重み付きか
 * @return std::vector<typename Tree<Cost>::Edge> 直径を構成する辺のリスト (端点から順に)
 */
template <typename Cost>
std::vector<typename Tree<Cost>::Edge> tree_diameter(const Tree<Cost>& tree, bool weighted = true) {
    assert(tree.size() > 0);
    auto [dist0, prev0] = internal_tree_util::tree_distances(tree, 0, weighted);
    long long u = std::max_element(dist0.begin(), dist0.end()) - dist0.begin();
    auto [dist, prev] = internal_tree_util::tree_distances(tree, u, weighted);
    long long v = std::max_element(dist.begin(), dist.end()) - dist.begin();
    std::vector<typename Tree<Cost>::Edge> path;
    for (long long w = v; w != u; w = prev[w].src) path.push_back(prev[w]);
    std::reverse(path.begin(), path.end());
    return path;
}

/**
 * @brief root を根とする根付き木で、各頂点を根とする部分木を同型なものごとに分類する O(n log n)
 *
 * ハッシュではなく、子の分類番号を並べた列に番号を振るので、衝突しない
 * @return 各頂点の分類番号 (0 から順に振る)。2 つの頂点の部分木が同型なとき、かつそのときに限り同じ番号
 */
template <typename Cost>
std::vector<long long> rooted_tree_isomorphism(const Tree<Cost>& tree, long long root) {
    const long long n = tree.size();
    assert(0 <= root && root < n);
    // BFS の順に並べ、葉の側から番号を決める
    std::vector<long long> order = {root}, parent(n, -1);
    for (long long i = 0; i < static_cast<long long>(order.size()); i++) {
        long long u = order[i];
        for (const auto& e : tree[u]) {
            if (e.dst == parent[u]) continue;
            parent[e.dst] = u;
            order.push_back(e.dst);
        }
    }
    std::vector<std::vector<long long>> children(n);
    std::map<std::vector<long long>, long long> ids;
    std::vector<long long> res(n);
    for (long long i = n - 1; i >= 0; i--) {
        long long u = order[i];
        auto& c = children[u];
        std::sort(c.begin(), c.end());
        auto [it, inserted] = ids.emplace(std::move(c), static_cast<long long>(ids.size()));
        res[u] = it->second;
        if (parent[u] != -1) children[parent[u]].push_back(res[u]);
    }
    return res;
}
