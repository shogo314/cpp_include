#pragma once

/**
 * @file chinese-postman.hpp
 * @brief 中国人郵便配達問題
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <optional>
#include <vector>

#include "graph.hpp"
#include "shortest-path.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/graph/chinese-postman.hpp

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
    const long long n = graph.n;
    Cost total{};
    for (const auto& e : graph.edges()) {
        assert(e.cost >= Cost{});
        total += e.cost;
    }
    std::vector<long long> odd;
    long long some = -1;  // 辺を持つ頂点の 1 つ
    for (long long v = 0; v < n; v++) {
        if (!graph[v].empty()) some = v;
        // 自己ループは隣接リストに 2 回入るので、次数に 2 と数えられる
        if (graph[v].size() % 2 == 1) odd.push_back(v);
    }
    if (some == -1) return Cost{};
    // 辺を持つ頂点がすべて連結か
    std::vector<Cost> d = shortest_path(graph, some, true, inf).dist;
    for (long long v = 0; v < n; v++) {
        if (!graph[v].empty() && d[v] == inf) return std::nullopt;
    }
    const int k = static_cast<int>(odd.size());
    assert(k < 31);
    std::vector<std::vector<Cost>> dist(k);
    for (int i = 0; i < k; i++) {
        std::vector<Cost> di = shortest_path(graph, odd[i], true, inf).dist;
        for (int j = 0; j < k; j++) dist[i].push_back(di[odd[j]]);
    }
    // dp[S]: 奇数次数の頂点の集合 S を組にする最小コスト。S の最小の頂点の相手を決める
    std::vector<Cost> dp(1 << k, inf);
    dp[0] = Cost{};
    for (int S = 1; S < (1 << k); S++) {
        if (__builtin_popcount(S) % 2 == 1) continue;
        int i = __builtin_ctz(S);
        for (int j = i + 1; j < k; j++) {
            if (!(S >> j & 1)) continue;
            int T = S ^ (1 << i) ^ (1 << j);
            if (dp[T] != inf) dp[S] = std::min(dp[S], dp[T] + dist[i][j]);
        }
    }
    return total + dp[(1 << k) - 1];
}
