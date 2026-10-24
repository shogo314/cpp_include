#pragma once

/**
 * @file shortest-path.hpp
 * @brief 最短路
 */

#include <algorithm>
#include <cassert>
#include <deque>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/b94a98acb19db49fa99249c824311583b1eb13e3/src/graph/shortest-path.hpp

/**
 * @brief 最短路の結果
 *
 * auto [dist, prev] = shortest_path(g, s); のように受け取れる
 */
template <typename Cost>
struct ShortestPath {
    using Edge = typename Graph<Cost>::Edge;
    std::vector<Cost> dist;  //!< 各頂点への最短路長
    std::vector<Edge> prev;  //!< 各頂点への最短路上の直前の辺 (始点と未到達の頂点では src == -1)
    /**
     * @brief 始点から t への最短路を構成する辺のリスト (始点側から順に)
     * @note t が始点か未到達の頂点なら空。負閉路の影響を受ける頂点では最短路が定まらない (assert で落ちる)
     */
    std::vector<Edge> path(long long t) const {
        std::vector<Edge> res;
        while (prev[t].src != -1) {
            res.push_back(prev[t]);
            assert(res.size() < prev.size() && "path: negative cycle");
            t = prev[t].src;
        }
        std::reverse(res.begin(), res.end());
        return res;
    }
};

namespace internal_shortest_path {

template <typename Cost>
ShortestPath<Cost> bfs(const Graph<Cost>& graph, long long s, Cost inf) {
    std::vector<Cost> dist(graph.n, inf);
    std::vector<typename Graph<Cost>::Edge> prev(graph.n);
    std::queue<long long> que;
    dist[s] = 0;
    que.push(s);
    while (!que.empty()) {
        long long u = que.front();
        que.pop();
        for (auto& e : graph[u]) {
            if (dist[e.dst] == inf) {
                dist[e.dst] = dist[e.src] + 1;
                prev[e.dst] = e;
                que.push(e.dst);
            }
        }
    }
    return {dist, prev};
}

template <typename Cost>
ShortestPath<Cost> dijkstra(const Graph<Cost>& graph, long long s, Cost inf) {
    std::vector<Cost> dist(graph.n, inf);
    std::vector<typename Graph<Cost>::Edge> prev(graph.n);
    using Node = std::pair<Cost, long long>;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> que;
    dist[s] = 0;
    que.push({0, s});
    while (!que.empty()) {
        auto [d, u] = que.top();
        que.pop();
        if (d > dist[u])
            continue;
        for (auto& e : graph[u]) {
            if (dist[e.dst] == inf || dist[e.dst] > dist[e.src] + e.cost) {
                dist[e.dst] = dist[e.src] + e.cost;
                prev[e.dst] = e;
                que.push({dist[e.dst], e.dst});
            }
        }
    }
    return {dist, prev};
}

}  // namespace internal_shortest_path

/**
 * @brief ある頂点から各頂点への最短路
 *
 * @param s 始点
 * @param weighted 1以外のコストの辺が存在するか 省略するとtrue
 * @param inf コストのminの単位元 未到達の頂点への距離はinfになる 省略すると std::numeric_limits<Cost>::max() / 4
 * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
 */
template <typename Cost = long long>
ShortestPath<Cost> shortest_path(const Graph<Cost>& graph, long long s, bool weighted = true, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    assert(0 <= s && s < graph.n);
    if (weighted)
        return internal_shortest_path::dijkstra(graph, s, inf);
    return internal_shortest_path::bfs(graph, s, inf);
}

/**
 * @brief ある頂点から各頂点への最短路
 *
 * @param s 始点
 * @param inf コストのminの単位元 未到達の頂点への距離はinfになる 省略すると std::numeric_limits<Cost>::max() / 4 負閉路の影響を受ける点は -inf
 * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
 */
template <typename Cost = long long>
ShortestPath<Cost> bellman_ford(const Graph<Cost>& graph, long long s, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    assert(0 <= s && s < graph.n);
    const long long n = graph.n;
    std::vector<Cost> dist(n, inf);
    std::vector<typename Graph<Cost>::Edge> prev(n);
    dist[s] = Cost{};

    for (long long i = 1; i < n; i++) {
        bool changed = false;
        for (long long j = 0; j < n; j++)
            for (const auto& e : graph[j]) {
                if (dist[e.src] == inf)
                    continue;
                if (dist[e.dst] > dist[e.src] + e.cost) {
                    dist[e.dst] = dist[e.src] + e.cost;
                    if (dist[e.dst] < -inf)
                        dist[e.dst] = -inf;
                    prev[e.dst] = e;
                    changed = true;
                }
            }
        if (!changed)
            break;
    }
    for (long long i = 0; i < n; i++) {
        bool changed = false;
        for (long long j = 0; j < n; j++)
            for (const auto& e : graph[j]) {
                if (dist[e.src] == inf)
                    continue;
                if (dist[e.dst] > dist[e.src] + e.cost) {
                    dist[e.dst] = -inf;
                    prev[e.dst] = e;
                    changed = true;
                }
            }
        if (!changed)
            break;
    }
    return {dist, prev};
}

/**
 * @brief 辺のコストが 0 か 1 だけのグラフで、ある頂点から各頂点への最短路 (01-BFS) O(n + m)
 *
 * @param s 始点
 * @param inf 未到達の頂点への距離 省略すると std::numeric_limits<Cost>::max() / 4
 * @return ShortestPath 各頂点への最短路長 (dist) と、各頂点への最短路上の直前の辺 (prev)
 */
template <typename Cost = long long>
ShortestPath<Cost> shortest_path_01bfs(const Graph<Cost>& graph, long long s, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    assert(0 <= s && s < graph.n);
    std::vector<Cost> dist(graph.n, inf);
    std::vector<typename Graph<Cost>::Edge> prev(graph.n);
    std::deque<long long> que;
    dist[s] = 0;
    que.push_back(s);
    while (!que.empty()) {
        long long u = que.front();
        que.pop_front();
        for (auto& e : graph[u]) {
            assert(e.cost == 0 || e.cost == 1);
            if (dist[e.dst] == inf || dist[e.dst] > dist[u] + e.cost) {
                dist[e.dst] = dist[u] + e.cost;
                prev[e.dst] = e;
                if (e.cost == 0)
                    que.push_front(e.dst);
                else
                    que.push_back(e.dst);
            }
        }
    }
    return {dist, prev};
}

/**
 * @brief 全点対間の最短路長 (Floyd-Warshall) O(n^3)
 *
 * @param inf 未到達を表す値 省略すると std::numeric_limits<Cost>::max() / 4
 * @return std::vector<std::vector<Cost>> dist[i][j] が i から j への最短路長 (到達できなければ inf)
 * @note 負の辺があってもよい。dist[i][i] < 0 となる i があれば負閉路がある
 */
template <typename Cost = long long>
std::vector<std::vector<Cost>> floyd_warshall(const Graph<Cost>& graph, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    const long long n = graph.n;
    std::vector<std::vector<Cost>> dist(n, std::vector<Cost>(n, inf));
    for (long long i = 0; i < n; i++) dist[i][i] = Cost{};
    for (long long i = 0; i < n; i++) {
        for (auto& e : graph[i]) dist[i][e.dst] = std::min(dist[i][e.dst], e.cost);
    }
    for (long long k = 0; k < n; k++) {
        for (long long i = 0; i < n; i++) {
            if (dist[i][k] == inf) continue;
            for (long long j = 0; j < n; j++) {
                if (dist[k][j] == inf) continue;
                dist[i][j] = std::min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }
    return dist;
}
