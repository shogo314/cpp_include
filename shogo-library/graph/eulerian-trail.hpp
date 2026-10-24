#pragma once

/**
 * @file eulerian-trail.hpp
 * @brief オイラー路
 */

#include <algorithm>
#include <cstdlib>
#include <utility>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/graph/eulerian-trail.hpp

namespace internal_eulerian_trail {

/**
 * @brief s から Hierholzer 法でたどり、使った辺をたどる順に並べる
 *
 * 辺を番号で使用済みにするので、無向辺 (同じ番号の辺が 2 本ある) も 1 回しか通らない
 * すべての辺を使えなければ (連結でなければ) 空を返す
 */
template <typename Cost>
std::vector<typename Graph<Cost>::Edge> hierholzer(const Graph<Cost>& graph, long long s) {
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
    if (static_cast<long long>(res.size()) != graph.m) return {};
    std::reverse(res.begin(), res.end());
    return res;
}

}  // namespace internal_eulerian_trail

/**
 * @brief 有向グラフのオイラー路 (すべての辺をちょうど 1 回ずつ通る路) を 1 つ求める
 *
 * Hierholzer 法 O(n + m)
 * @return std::vector<Edge> 路の辺を、たどる順に並べたもの (e[k].dst == e[k + 1].src)。オイラー路がなければ空
 * 辺がないときも空 (オイラー路は自明に存在する)。始点と終点が同じになるのは、閉路として回れるときだけ
 */
template <typename Cost = long long>
std::vector<typename Graph<Cost>::Edge> eulerian_trail_directed(const Graph<Cost>& graph) {
    if (graph.m == 0) return {};
    std::vector<long long> diff(graph.n, 0);  // 出次数 - 入次数
    for (long long v = 0; v < graph.n; v++) {
        for (long long u : graph[v]) diff[v]++, diff[u]--;
    }
    // 出次数が 1 多い頂点があればそこから始める。なければ辺を持つ頂点から
    long long s = -1, plus = 0;
    for (long long v = 0; v < graph.n; v++) {
        if (std::abs(diff[v]) > 1) return {};
        if (diff[v] == 1) s = v, plus++;
    }
    if (plus > 1) return {};
    for (long long v = 0; s == -1; v++) {
        if (!graph[v].empty()) s = v;
    }
    return internal_eulerian_trail::hierholzer(graph, s);
}

/**
 * @brief 無向グラフのオイラー路 (すべての辺をちょうど 1 回ずつ通る路) を 1 つ求める
 *
 * Hierholzer 法 O(n + m)
 * @return std::vector<Edge> 路の辺を、たどる向きにして並べたもの (e[k].dst == e[k + 1].src)。オイラー路がなければ空
 * 辺がないときも空 (オイラー路は自明に存在する)。自己ループ・多重辺があってもよい
 */
template <typename Cost = long long>
std::vector<typename Graph<Cost>::Edge> eulerian_trail_undirected(const Graph<Cost>& graph) {
    if (graph.m == 0) return {};
    // 次数が奇数の頂点が 2 個ならその一方から、0 個なら辺を持つ頂点から始める
    long long s = -1, odd = 0;
    for (long long v = 0; v < graph.n; v++) {
        if (graph[v].size() % 2 == 1) s = v, odd++;
    }
    if (odd > 2) return {};
    for (long long v = 0; s == -1; v++) {
        if (!graph[v].empty()) s = v;
    }
    return internal_eulerian_trail::hierholzer(graph, s);
}
