#pragma once

/**
 * @file eulerian-trail.hpp
 * @brief オイラー路
 */

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/graph/eulerian-trail.hpp

namespace internal_eulerian_trail {

/**
 * @brief s から Hierholzer 法でたどり、使った辺をたどる順に並べる
 *
 * 辺を番号で使用済みにするので、無向辺 (同じ番号の辺が 2 本ある) も 1 回しか通らない
 * すべての辺を使えなければ (連結でなければ) std::nullopt を返す
 */
template <typename Cost>
std::optional<std::vector<typename Graph<Cost>::Edge>> hierholzer(const Graph<Cost>& graph, long long s) {
    using Edge = typename Graph<Cost>::Edge;
    std::vector<int> iter(graph.n, 0);
    std::vector<bool> used(graph.m, false);
    std::vector<Edge> res;
    // (今いる頂点, そこへ来るのに使った辺)。始点では辺の src == -1
    std::vector<std::pair<long long, Edge>> st = {{s, Edge()}};
    while (!st.empty()) {
        long long v = st.back().first;
        int& it = iter[v];
        while (it < static_cast<int>(graph[v].size()) && used[graph[v][it].id]) it++;
        if (it == static_cast<int>(graph[v].size())) {
            if (st.back().second.src != -1) res.push_back(st.back().second);
            st.pop_back();
        } else {
            const Edge& e = graph[v][it++];
            used[e.id] = true;
            st.push_back({e.dst, e});
        }
    }
    if (static_cast<long long>(res.size()) != graph.m) return std::nullopt;
    std::reverse(res.begin(), res.end());
    return res;
}

// 辺がないグラフのオイラー路。頂点 0 だけの路 (頂点もなければ空)
template <typename Cost>
Path<Cost> empty_trail(const Graph<Cost>& graph) {
    if (graph.n == 0) return {Cost{}, {}, {}};
    return {Cost{}, {0}, {}};
}

}  // namespace internal_eulerian_trail

/**
 * @brief 有向グラフのオイラー路 (すべての辺をちょうど 1 回ずつ通る路) を 1 つ求める
 *
 * Hierholzer 法 O(n + m)
 * @return std::optional<Path<Cost>> 路 (長さはすべての辺のコストの和、たどる頂点、たどる辺)。オイラー路がなければ std::nullopt
 * 辺がないときは、頂点 0 だけの路を返す (頂点もなければ頂点の列も空)。始点と終点が同じになるのは、閉路として回れるときだけ
 */
template <typename Cost = long long>
std::optional<Path<Cost>> eulerian_trail_directed(const Graph<Cost>& graph) {
    if (graph.m == 0) return internal_eulerian_trail::empty_trail(graph);
    std::vector<long long> diff(graph.n, 0);  // 出次数 - 入次数
    for (long long v = 0; v < graph.n; v++) {
        for (long long u : graph[v]) diff[v]++, diff[u]--;
    }
    // 出次数が 1 多い頂点があればそこから始める。なければ辺を持つ頂点から
    long long s = -1, plus = 0;
    for (long long v = 0; v < graph.n; v++) {
        if (std::abs(diff[v]) > 1) return std::nullopt;
        if (diff[v] == 1) s = v, plus++;
    }
    if (plus > 1) return std::nullopt;
    for (long long v = 0; s == -1; v++) {
        if (!graph[v].empty()) s = v;
    }
    auto trail = internal_eulerian_trail::hierholzer(graph, s);
    if (!trail) return std::nullopt;
    return internal_graph::make_path<Cost>(s, std::move(*trail));
}

/**
 * @brief 無向グラフのオイラー路 (すべての辺をちょうど 1 回ずつ通る路) を 1 つ求める
 *
 * Hierholzer 法 O(n + m)
 * @return std::optional<Path<Cost>> 路 (長さはすべての辺のコストの和、たどる頂点、辺はたどる向きにしたもの)。オイラー路がなければ std::nullopt
 * 辺がないときは、頂点 0 だけの路を返す (頂点もなければ頂点の列も空)。自己ループ・多重辺があってもよい
 */
template <typename Cost = long long>
std::optional<Path<Cost>> eulerian_trail_undirected(const Graph<Cost>& graph) {
    if (graph.m == 0) return internal_eulerian_trail::empty_trail(graph);
    // 次数が奇数の頂点が 2 個ならその一方から、0 個なら辺を持つ頂点から始める
    long long s = -1, odd = 0;
    for (long long v = 0; v < graph.n; v++) {
        if (graph[v].size() % 2 == 1) s = v, odd++;
    }
    if (odd > 2) return std::nullopt;
    for (long long v = 0; s == -1; v++) {
        if (!graph[v].empty()) s = v;
    }
    auto trail = internal_eulerian_trail::hierholzer(graph, s);
    if (!trail) return std::nullopt;
    return internal_graph::make_path<Cost>(s, std::move(*trail));
}
