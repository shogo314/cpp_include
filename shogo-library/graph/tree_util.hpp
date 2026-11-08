#pragma once

/**
 * @file tree_util.hpp
 * @brief 木に関する関数
 */

#include <algorithm>
#include <cassert>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/graph/tree_util.hpp

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

// 重みなしで c から各頂点への距離と、c のどの隣接頂点の側にあるか (c では -1) を求める
template <typename Cost>
std::pair<std::vector<long long>, std::vector<long long>> branch_depths(const Tree<Cost>& tree, long long c) {
    long long n = tree.size();
    std::vector<long long> dist(n, -1), branch(n, -1);
    std::vector<long long> stk = {c};
    dist[c] = 0;
    while (!stk.empty()) {
        long long u = stk.back();
        stk.pop_back();
        for (const auto& e : tree[u]) {
            if (dist[e.dst] != -1) continue;
            dist[e.dst] = dist[u] + 1;
            branch[e.dst] = u == c ? e.dst : branch[u];
            stk.push_back(e.dst);
        }
    }
    return {dist, branch};
}

// 直径の両端 u, v と、u から各頂点への距離と辺 (tree_distances の結果) を求める
template <typename Cost>
std::tuple<long long, long long, std::vector<Cost>, std::vector<typename Tree<Cost>::Edge>> diameter_endpoints(const Tree<Cost>& tree, bool weighted) {
    assert(tree.size() > 0);
    auto dist0 = tree_distances(tree, 0, weighted).first;
    long long u = std::max_element(dist0.begin(), dist0.end()) - dist0.begin();
    auto [dist, prev] = tree_distances(tree, u, weighted);
    long long v = std::max_element(dist.begin(), dist.end()) - dist.begin();
    return {u, v, std::move(dist), std::move(prev)};
}

}  // namespace internal_tree_util

/**
 * @brief 木の直径の長さ
 *
 * @param tree 木 (頂点数 1 以上、辺の重みは非負)
 * @param weighted = true 重み付きか。false なら辺の本数で測る
 * @return Cost 直径の長さ (頂点数 1 なら 0)
 */
template <typename Cost>
Cost tree_diameter(const Tree<Cost>& tree, bool weighted = true) {
    auto [u, v, dist, prev] = internal_tree_util::diameter_endpoints(tree, weighted);
    return dist[v];
}

/**
 * @brief 木の直径を、長さ・頂点の列・辺の列で求める
 *
 * 一方の端点から他方の端点までの経路を返す。頂点数 1 なら長さ 0、頂点は {0}、辺は空
 * @param tree 木 (頂点数 1 以上、辺の重みは非負)
 * @param weighted = true 重み付きか。false なら辺の本数で測る (cost も辺の本数)
 * @return Path<Cost> 長さ cost・頂点の列 vertices・辺の列 edges
 */
template <typename Cost>
Path<Cost> tree_diameter_restore(const Tree<Cost>& tree, bool weighted = true) {
    auto [u, v, dist, prev] = internal_tree_util::diameter_endpoints(tree, weighted);
    Path<Cost> res{dist[v], {v}, {}};
    for (long long w = v; w != u; w = prev[w].src) {
        res.vertices.push_back(prev[w].src);
        res.edges.push_back(prev[w]);
    }
    std::reverse(res.vertices.begin(), res.vertices.end());
    std::reverse(res.edges.begin(), res.edges.end());
    return res;
}

/**
 * @brief 木の中心 (重みなし)
 *
 * 最も遠い頂点までの辺の本数が最小になる頂点。直径の中点にあたる
 * @param tree 木 (頂点数 1 以上)
 * @return std::vector<long long> 中心 (1 つか 2 つ。2 つなら隣接していて、小さい順)
 */
template <typename Cost>
std::vector<long long> tree_center(const Tree<Cost>& tree) {
    auto path = tree_diameter_restore(tree, false).vertices;
    long long d = path.size() - 1;
    long long c = path[d / 2];
    if (d % 2 == 0) return {c};
    long long c2 = path[d / 2 + 1];
    return {std::min(c, c2), std::max(c, c2)};
}

/**
 * @brief 直径 (重みなし) の端点を、2 点がちょうど直径の両端になるのは異なるグループにあるとき、かつそのときに限るように分ける
 *
 * 直径の長さが偶数なら中心の隣接頂点ごと、奇数なら中心の辺のどちらの側かで分ける
 * @param tree 木 (頂点数 1 以上)
 * @return std::vector<std::vector<long long>> 各グループの頂点 (どれも空でなく、2 つ以上。頂点数 1 なら {{0}})
 */
template <typename Cost>
std::vector<std::vector<long long>> tree_diameter_endpoint_groups(const Tree<Cost>& tree) {
    long long n = tree.size();
    if (n == 1) return {{0}};
    auto center = tree_center(tree);
    long long c = center[0];
    auto [dist, branch] = internal_tree_util::branch_depths(tree, c);
    long long r = *std::max_element(dist.begin(), dist.end());
    std::vector<std::vector<long long>> groups;
    if (center.size() == 2) {
        // c の側は c からの距離が r - 1、もう一方の中心の側は r
        groups.resize(2);
        for (long long v = 0; v < n; v++) {
            bool other = branch[v] == center[1];
            if (dist[v] == (other ? r : r - 1)) groups[other].push_back(v);
        }
        return groups;
    }
    std::vector<long long> id(n, -1);
    for (long long v = 0; v < n; v++) {
        if (dist[v] != r) continue;
        if (id[branch[v]] == -1) {
            id[branch[v]] = groups.size();
            groups.emplace_back();
        }
        groups[id[branch[v]]].push_back(v);
    }
    return groups;
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
