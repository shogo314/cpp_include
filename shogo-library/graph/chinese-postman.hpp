#pragma once

/**
 * @file chinese-postman.hpp
 * @brief 中国人郵便配達問題
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

#include "eulerian-trail.hpp"
#include "graph.hpp"
#include "shortest-path.hpp"
// https://github.com/shogo314/shogo-library/blob/c965efc39d855af98a1bcf96c2b63f175ea40371/src/graph/chinese-postman.hpp

namespace internal_chinese_postman {
/**
 * @brief 中国人郵便配達問題の DP
 * odd: 次数が奇数の頂点、sp[i]: odd[i] からの最短路、pair_with[S]: 集合 S の最小の頂点の相手 (odd の添字)
 * cost: 最小の長さ。辺がすべて 1 つの連結成分に入っていなければ std::nullopt
 */
template <typename Cost>
struct Solver {
    std::optional<Cost> cost;
    long long start = -1;  // 辺を持つ頂点の 1 つ (辺がなければ -1)
    std::vector<long long> odd;
    std::vector<ShortestPath<Cost>> sp;
    std::vector<int> pair_with;
};
template <typename Cost>
Solver<Cost> solve(const Graph<Cost>& graph, Cost inf) {
    const long long n = graph.n;
    Solver<Cost> res;
    Cost total{};
    for (const auto& e : graph.edges()) {
        assert(e.cost >= Cost{});
        total += e.cost;
    }
    for (long long v = 0; v < n; v++) {
        if (!graph[v].empty()) res.start = v;
        // 自己ループは隣接リストに 2 回入るので、次数に 2 と数えられる
        if (graph[v].size() % 2 == 1) res.odd.push_back(v);
    }
    if (res.start == -1) {
        res.cost = Cost{};
        return res;
    }
    // 辺を持つ頂点がすべて連結か
    std::vector<Cost> d = shortest_path_dijkstra(graph, res.start, inf).dist;
    for (long long v = 0; v < n; v++) {
        if (!graph[v].empty() && d[v] == inf) return res;
    }
    const auto& odd = res.odd;
    const int k = static_cast<int>(odd.size());
    assert(k < 31);
    for (int i = 0; i < k; i++) res.sp.push_back(shortest_path_dijkstra(graph, odd[i], inf));
    // dp[S]: 奇数次数の頂点の集合 S を組にする最小コスト。S の最小の頂点の相手を決める
    std::vector<Cost> dp(1 << k, inf);
    res.pair_with.assign(1 << k, -1);
    dp[0] = Cost{};
    for (int S = 1; S < (1 << k); S++) {
        if (__builtin_popcount(S) % 2 == 1) continue;
        int i = __builtin_ctz(S);
        for (int j = i + 1; j < k; j++) {
            if (!(S >> j & 1)) continue;
            int T = S ^ (1 << i) ^ (1 << j);
            if (dp[T] != inf && dp[T] + res.sp[i].dist[odd[j]] < dp[S]) {
                dp[S] = dp[T] + res.sp[i].dist[odd[j]];
                res.pair_with[S] = j;
            }
        }
    }
    res.cost = total + dp[(1 << k) - 1];
    return res;
}
}  // namespace internal_chinese_postman

/**
 * @brief 無向グラフで、すべての辺を 1 回以上通って始点に戻る閉路の長さの最小値 (中国人郵便配達問題)
 *
 * 次数が奇数の頂点 (k 個) を 2 個ずつ組にして、組の間の最短路を余分に通ればよい。
 * 奇数次数の頂点ごとに Dijkstra 法で距離を求め、組にする最小コストを bit DP で求める。O(k m log m + 2^k k)
 * 始点はどこでもよい (辺を持つ頂点のどれから始めても同じ長さになる)
 * @param inf 到達できないことを表す値 省略すると std::numeric_limits<Cost>::max() / 4
 * @return std::optional<Cost> 辺がすべて 1 つの連結成分に入っていなければ std::nullopt。辺がなければ 0
 * @note 辺のコストは 0 以上であること。k は 20 程度まで。自己ループ・多重辺があってもよい
 */
template <typename Cost = long long>
std::optional<Cost> chinese_postman(const Graph<Cost>& graph, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    return internal_chinese_postman::solve(graph, inf).cost;
}

/**
 * @brief 中国人郵便配達問題の最短の閉路を 1 つ求める
 *
 * 組にした奇数次数の頂点の間の最短路の辺を複製し、すべての次数を偶数にしたグラフでオイラー閉路をたどる。計算量は chinese_postman と同じ
 * @param inf chinese_postman と同じ
 * @return std::optional<Path<Cost>> 閉路 (長さ、たどる頂点、たどる辺)。辺は元のグラフの辺 (番号も元のもの) をたどる向きにしたもので、
 * 複製した辺は同じ辺が何度か現れる。辺がすべて 1 つの連結成分に入っていなければ std::nullopt。辺がなければ頂点 0 だけ (頂点もなければ空) の閉路
 * @note chinese_postman と同じ
 */
template <typename Cost = long long>
std::optional<Path<Cost>> chinese_postman_restore(const Graph<Cost>& graph, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    using Edge = typename Graph<Cost>::Edge;
    auto res = internal_chinese_postman::solve(graph, inf);
    if (!res.cost) return std::nullopt;
    if (res.start == -1) return Path<Cost>{Cost{}, graph.n == 0 ? std::vector<long long>{} : std::vector<long long>{0}, {}};
    // 元の辺と、組の間の最短路の辺を複製したものを並べた多重グラフ。orig[新しい番号] は元の辺の番号
    Graph<Cost> h(graph.n);
    std::vector<long long> orig;
    auto add = [&](const Edge& e) {
        h.add_edge(e.src, e.dst, e.cost);
        orig.push_back(e.id);
    };
    for (const auto& e : graph.edges()) add(e);
    const int k = static_cast<int>(res.odd.size());
    for (int S = (1 << k) - 1; S > 0;) {
        int i = __builtin_ctz(S), j = res.pair_with[S];
        const auto path = res.sp[i].path(res.odd[j]);  // 一時オブジェクトのまま ->edges を回すと先に破棄される
        for (const auto& e : path->edges) add(e);
        S ^= (1 << i) | (1 << j);
    }
    // すべての頂点の次数が偶数なので、オイラー閉路が見つかる
    auto trail = *internal_eulerian_trail::hierholzer(h, res.start);
    for (auto& e : trail) e.id = orig[e.id];
    return internal_graph::make_path<Cost>(res.start, std::move(trail));
}
