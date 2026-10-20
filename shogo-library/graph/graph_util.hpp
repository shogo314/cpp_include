#pragma once

/**
 * @file graph_util.hpp
 * @brief グラフに関する関数
 */

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <stack>
#include <utility>
#include <vector>

#include "../data_structures/unionfind.hpp"
#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/5923a8f8c364ef7144ec85fa5ef4504a9e4f5c62/src/graph/graph_util.hpp

/**
 * @brief 無向グラフについて、二部グラフなら0と1に彩色した結果をひとつ返し、二部グラフでないなら空のvectorを返す。
 * 連結成分のうち、インデックスの最も小さいものを0にする。
 * @return std::vector<int> 各頂点の彩色結果
 */
template <typename Cost = int>
std::vector<int> bipartite_coloring(const Graph<Cost>& graph) {
    std::vector<int> color(graph.n, -1);
    for (long long i = 0; i < graph.n; i++) {
        if (color[i] != -1) continue;
        std::stack<long long> stk;
        stk.push(i);
        color[i] = 0;
        while (!stk.empty()) {
            long long u = stk.top();
            stk.pop();
            for (long long v : graph[u]) {
                if (color[v] == -1) {
                    color[v] = color[u] ^ 1;
                    stk.push(v);
                } else {
                    if (color[u] == color[v]) return {};
                }
            }
        }
    }
    return color;
}

/**
 * @brief 無向グラフについて、二部グラフかどうかを判定する。
 * @return bool 二部グラフならtrue、二部グラフでないならfalseを返す。
 */
template <typename Cost = int>
bool is_bipartite(const Graph<Cost>& graph) {
    return !bipartite_coloring(graph).empty();
}

/**
 * @brief 無向グラフについて、連結成分分解する。
 * @return std::vector<std::vector<long long>> 「同じ連結成分となる頂点のリスト」のリスト
 */
template <typename Cost = int>
std::vector<std::vector<long long>> connected_components(const Graph<Cost>& graph) {
    std::vector<std::vector<long long>> groups;
    std::vector<bool> visited(graph.n);
    for (long long i = 0; i < graph.n; i++) {
        if (visited[i]) continue;
        std::stack<long long> stk;
        stk.push(i);
        visited[i] = true;
        groups.push_back({i});
        while (!stk.empty()) {
            long long u = stk.top();
            stk.pop();
            for (long long v : graph[u]) {
                if (visited[v]) continue;
                visited[v] = true;
                stk.push(v);
                groups.back().push_back(v);
            }
        }
    }
    return groups;
}

/**
 * @brief 無向グラフについて、連結グラフかどうかを判定する。
 * @return bool 連結グラフならtrue、連結グラフでないならfalseを返す。
 */
template <typename Cost = int>
bool is_connected(const Graph<Cost>& graph) {
    return connected_components(graph).size() == 1;
}

/**
 * @brief 無向グラフについて、木かどうかを判定する。
 * @return bool 木ならtrue、木でないならfalseを返す。
 */
template <typename Cost = int>
bool is_tree(const Graph<Cost>& graph) {
    return graph.m == graph.n - 1 && is_connected(graph);
}

/**
 * @brief 有向グラフをトポロジカルソートする
 * @return std::vector<long long> トポロジカル順に並べた頂点のリスト。DAG でなければ空
 */
template <typename Cost = int>
std::vector<long long> topological_sort(const Graph<Cost>& graph) {
    std::vector<int> indeg(graph.n);
    std::vector<long long> sorted;
    std::queue<long long> q;
    for (long long i = 0; i < graph.n; i++) {
        for (long long dst : graph[i]) indeg[dst]++;
    }
    for (long long i = 0; i < graph.n; i++) {
        if (!indeg[i]) q.push(i);
    }
    while (!q.empty()) {
        long long cur = q.front();
        q.pop();
        for (long long dst : graph[cur]) {
            if (!--indeg[dst]) q.push(dst);
        }
        sorted.push_back(cur);
    }
    if (static_cast<long long>(sorted.size()) < graph.n) return {};
    return sorted;
}

/**
 * @brief 有向グラフについて、サイクルをもつかどうかを判定する。
 * @return bool サイクルをもつならtrue、非巡回ならfalseを返す。
 */
template <typename Cost = int>
bool has_cycle(const Graph<Cost>& graph) {
    return graph.n > 0 && topological_sort(graph).empty();
}
/**
 * @deprecated has_cycle を使うこと(名前と意味が逆に読めるため改名した)
 */
template <typename Cost = int>
[[deprecated("use has_cycle")]] bool has_acyclic(const Graph<Cost>& graph) {
    return has_cycle(graph);
}

/**
 * @brief 有向グラフの閉路を 1 つ求める
 *
 * DFS を再帰を使わずに行う。O(n + m)
 * @return std::vector<Edge> 閉路の辺を、たどる順に並べたもの (e[k].dst == e[k + 1].src、末尾の dst は先頭の src)。閉路がなければ空
 * 自己ループは長さ 1 の閉路になる。頂点の列は各辺の src で得られる
 */
template <typename Cost = int>
std::vector<typename Graph<Cost>::Edge> find_cycle_directed(const Graph<Cost>& graph) {
    using Edge = typename Graph<Cost>::Edge;
    // state: 0 は未訪問、1 は DFS のスタック上、2 は探索済み
    std::vector<int> state(graph.n, 0), iter(graph.n, 0);
    std::vector<Edge> in(graph.n);  // DFS 木で v に入る辺
    for (long long root = 0; root < graph.n; root++) {
        if (state[root]) continue;
        std::vector<long long> st = {root};
        state[root] = 1;
        while (!st.empty()) {
            long long v = st.back();
            if (iter[v] == static_cast<int>(graph[v].size())) {
                state[v] = 2;
                st.pop_back();
                continue;
            }
            const Edge& e = graph[v][iter[v]++];
            long long u = e.dst;
            if (state[u] == 0) {
                state[u] = 1;
                in[u] = e;
                st.push_back(u);
            } else if (state[u] == 1) {
                // u はスタック上にあるので、u から v への木の辺と e で閉路になる
                std::vector<Edge> res = {e};
                for (long long w = v; w != u; w = in[w].src) res.push_back(in[w]);
                std::reverse(res.begin(), res.end());
                return res;
            }
        }
    }
    return {};
}

/**
 * @brief 無向グラフの閉路を 1 つ求める
 *
 * DFS を再帰を使わずに行う。O(n + m)
 * @return std::vector<Edge> 閉路の辺を、たどる順に並べたもの (e[k].dst == e[k + 1].src、末尾の dst は先頭の src)。閉路がなければ空
 * 自己ループは長さ 1、多重辺は長さ 2 の閉路になる (同じ辺を戻ることはない)。頂点の列は各辺の src で得られる
 */
template <typename Cost = int>
std::vector<typename Graph<Cost>::Edge> find_cycle_undirected(const Graph<Cost>& graph) {
    using Edge = typename Graph<Cost>::Edge;
    std::vector<int> state(graph.n, 0), iter(graph.n, 0);
    std::vector<Edge> in(graph.n);
    for (long long root = 0; root < graph.n; root++) {
        if (state[root]) continue;
        std::vector<long long> st = {root};
        state[root] = 1;
        while (!st.empty()) {
            long long v = st.back();
            if (iter[v] == static_cast<int>(graph[v].size())) {
                state[v] = 2;
                st.pop_back();
                continue;
            }
            const Edge& e = graph[v][iter[v]++];
            // 親へ戻る辺は、頂点ではなく辺の番号で区別する (多重辺を閉路として見つけるため)
            if (e.id == in[v].id) continue;  // 根では in[v].id == -1
            long long u = e.dst;
            if (state[u] == 0) {
                state[u] = 1;
                in[u] = e;
                st.push_back(u);
            } else if (state[u] == 1) {
                // u は v の祖先 (か v 自身) なので、u から v への木の辺と e で閉路になる
                std::vector<Edge> res = {e};
                for (long long w = v; w != u; w = in[w].src) res.push_back(in[w]);
                std::reverse(res.begin(), res.end());
                return res;
            }
            // state[u] == 2 にはならない (なるなら、u の側でこの辺により閉路が見つかっている)
        }
    }
    return {};
}

/**
 * @brief 有向グラフを強連結成分分解する
 *
 * Tarjan のアルゴリズムを再帰を使わずに行う。O(n + m)
 *
 * @return std::vector<std::vector<long long>> 強連結成分ごとの頂点のリスト。
 * 成分はトポロジカル順に並ぶ (成分 A から成分 B へ辺があれば A が B より前)
 */
template <typename Cost = int>
std::vector<std::vector<long long>> strongly_connected_components(const Graph<Cost>& graph) {
    const long long n = graph.n;
    std::vector<int> ord(n, -1), low(n), comp(n, -1), iter(n, 0);
    std::vector<long long> visiting;  // 成分がまだ決まっていない訪問済みの頂点
    std::vector<long long> call;      // DFS の呼び出しスタック
    int now = 0, k = 0;
    for (long long s = 0; s < n; s++) {
        if (ord[s] != -1) continue;
        ord[s] = low[s] = now++;
        visiting.push_back(s);
        call.push_back(s);
        while (!call.empty()) {
            long long v = call.back();
            if (iter[v] < static_cast<int>(graph[v].size())) {
                long long to = graph[v][iter[v]++].dst;
                if (ord[to] == -1) {
                    ord[to] = low[to] = now++;
                    visiting.push_back(to);
                    call.push_back(to);
                } else if (comp[to] == -1) {
                    low[v] = std::min(low[v], ord[to]);
                }
                continue;
            }
            call.pop_back();
            if (!call.empty()) low[call.back()] = std::min(low[call.back()], low[v]);
            if (low[v] == ord[v]) {
                while (true) {
                    long long u = visiting.back();
                    visiting.pop_back();
                    comp[u] = k;
                    if (u == v) break;
                }
                k++;
            }
        }
    }
    // Tarjan のアルゴリズムは成分をトポロジカル順の逆に見つけるので、逆順に並べる
    std::vector<std::vector<long long>> groups(k);
    for (long long v = 0; v < n; v++) groups[k - 1 - comp[v]].push_back(v);
    return groups;
}

/**
 * @brief 最小全域木の結果
 */
template <typename Cost>
struct MinimumSpanningTree {
    Cost cost;                                      //!< 使った辺のコストの総和
    std::vector<typename Graph<Cost>::Edge> edges;  //!< 使った辺 (コストの昇順、同じコストなら辺の番号順)
};

/**
 * @brief 無向グラフの最小全域木 (Kruskal 法) O(m log m)
 *
 * 連結でなければ、各連結成分の最小全域木を合わせたもの (最小全域森) を返す
 * 連結かどうかは edges.size() == n - 1 で判定できる
 */
template <typename Cost = int>
MinimumSpanningTree<Cost> minimum_spanning_tree(const Graph<Cost>& graph) {
    auto es = graph.edges();
    std::stable_sort(es.begin(), es.end(), [](const auto& a, const auto& b) { return a.cost < b.cost; });
    UnionFind uf(graph.n);
    MinimumSpanningTree<Cost> res{Cost{}, {}};
    for (const auto& e : es) {
        if (uf.merge(e.src, e.dst)) {
            res.cost += e.cost;
            res.edges.push_back(e);
        }
    }
    return res;
}

/**
 * @brief 最小シュタイナー木の辺重みの総和
 * @note X を連結にできなければ inf を返す
 */
template <typename Cost, typename T>
Cost minimum_steiner_tree(const Graph<Cost>& graph, const std::vector<T>& X, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    if (X.empty()) return Cost{};
    for (const auto& x : X) assert(0 <= x && x < graph.n);
    std::vector<std::vector<Cost>> dp(graph.n, std::vector<Cost>(1 << X.size(), inf));
    for (int p = 0; p < static_cast<int>(X.size()); p++) dp[X[p]][1 << p] = {};
    for (int i = 0; i < (1 << X.size()); i++) {
        for (int j = i; j > 0; j = (j - 1) & i) {
            for (long long k = 0; k < graph.n; k++) {
                if (dp[k][j] != inf && dp[k][i ^ j] != inf) {
                    if (dp[k][i] == inf || dp[k][i] > dp[k][j] + dp[k][i ^ j]) dp[k][i] = dp[k][j] + dp[k][i ^ j];
                }
            }
        }
        std::priority_queue<std::pair<Cost, long long>, std::vector<std::pair<Cost, long long>>, std::greater<std::pair<Cost, long long>>> que;
        for (long long k = 0; k < graph.n; k++)
            if (dp[k][i] != inf) que.emplace(dp[k][i], k);
        while (que.size()) {
            auto [d, u] = que.top();
            que.pop();
            if (d > dp[u][i]) continue;
            for (auto e : graph[u]) {
                if (dp[e.dst][i] == inf || dp[e.dst][i] > d + e.cost) {
                    dp[e.dst][i] = d + e.cost;
                    que.emplace(d + e.cost, e.dst);
                }
            }
        }
    }
    Cost res = inf;
    for (long long k = 0; k < graph.n; k++) {
        if (dp[k].back() != inf && (res == inf || res > dp[k].back())) res = dp[k].back();
    }
    return res;
}
