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
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/graph/graph_util.hpp

/**
 * @brief 無向グラフについて、二部グラフなら0と1に彩色した結果をひとつ返し、二部グラフでないなら空のvectorを返す。
 * 連結成分のうち、インデックスの最も小さいものを0にする。
 * @return std::vector<int> 各頂点の彩色結果
 */
template <typename Cost = long long>
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
template <typename Cost = long long>
bool is_bipartite(const Graph<Cost>& graph) {
    return !bipartite_coloring(graph).empty();
}

/**
 * @brief 無向グラフについて、連結成分分解する。
 * @return std::vector<std::vector<long long>> 「同じ連結成分となる頂点のリスト」のリスト
 */
template <typename Cost = long long>
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
 * @brief 無向グラフの補グラフ (辺のない 2 頂点の間に辺を張ったグラフ) を連結成分分解する O(n + m)
 * まだどの成分にも入っていない頂点の列を持ち、取り出した頂点と隣り合わないものを同じ成分に移す
 */
template <typename Cost = long long>
std::vector<std::vector<long long>> complement_connected_components(const Graph<Cost>& graph) {
    const long long n = graph.n;
    std::vector<long long> rest(n);
    for (long long i = 0; i < n; i++) rest[i] = i;
    std::vector<bool> adj(n, false);
    std::vector<std::vector<long long>> groups;
    while (!rest.empty()) {
        groups.push_back({rest.back()});
        rest.pop_back();
        // groups.back() の i 番目の頂点を取り出し、rest のうち隣り合わないものを成分に移す
        for (size_t i = 0; i < groups.back().size(); i++) {
            const long long u = groups.back()[i];
            for (long long v : graph[u]) adj[v] = true;
            std::vector<long long> keep;
            for (long long v : rest) {
                if (adj[v]) {
                    keep.push_back(v);
                } else {
                    groups.back().push_back(v);
                }
            }
            rest.swap(keep);
            for (long long v : graph[u]) adj[v] = false;
        }
    }
    return groups;
}

/**
 * @brief 無向グラフについて、連結グラフかどうかを判定する。
 * @return bool 連結グラフならtrue、連結グラフでないならfalseを返す。
 */
template <typename Cost = long long>
bool is_connected(const Graph<Cost>& graph) {
    return connected_components(graph).size() == 1;
}

/**
 * @brief 無向グラフについて、木かどうかを判定する。
 * @return bool 木ならtrue、木でないならfalseを返す。
 */
template <typename Cost = long long>
bool is_tree(const Graph<Cost>& graph) {
    return graph.m == graph.n - 1 && is_connected(graph);
}

/**
 * @brief 有向グラフをトポロジカルソートする
 * @return std::vector<long long> トポロジカル順に並べた頂点のリスト。DAG でなければ途中まで (閉路からたどり着けない頂点だけ) を並べたもので、長さが n より短くなる
 */
template <typename Cost = long long>
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
    return sorted;
}

/**
 * @brief 有向グラフについて、サイクルをもつかどうかを判定する。
 * @return bool サイクルをもつならtrue、非巡回ならfalseを返す。
 */
template <typename Cost = long long>
bool has_cycle(const Graph<Cost>& graph) {
    return static_cast<long long>(topological_sort(graph).size()) < graph.n;
}

/**
 * @brief 有向グラフの閉路を 1 つ求める
 *
 * DFS を再帰を使わずに行う。O(n + m)
 * @return std::vector<Edge> 閉路の辺を、たどる順に並べたもの (e[k].dst == e[k + 1].src、末尾の dst は先頭の src)。閉路がなければ空
 * 自己ループは長さ 1 の閉路になる。頂点の列は各辺の src で得られる
 */
template <typename Cost = long long>
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
template <typename Cost = long long>
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
template <typename Cost = long long>
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
template <typename Cost = long long>
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
 * @brief 無向グラフの彩色数 (隣り合う頂点が同じ色にならないように塗るときの色の最小数。頂点数 20 程度まで) O(3^n) より少し速い
 * 独立集合の表を O(2^n) で作り、dp[S] = min (dp[S \ T] + 1) を、T を「S の最小の頂点 v を含み、v と隣り合わない S の頂点からなる独立集合」に限って求める
 * 頂点がなければ 0。自己ループがあれば塗れないので -1
 */
template <typename Cost = long long>
long long chromatic_number(const Graph<Cost>& graph) {
    const int n = graph.n;
    if (n == 0) return 0;
    assert(n <= 30);
    std::vector<int> adj(n, 0);
    for (int v = 0; v < n; v++) {
        for (const auto& e : graph[v]) {
            if (e.dst == v) return -1;
            adj[v] |= 1 << e.dst;
        }
    }
    // indep[S]: S が独立集合か (最小の頂点を除いた集合が独立で、最小の頂点がそれと隣り合わない)
    std::vector<bool> indep(1 << n, false);
    indep[0] = true;
    for (int S = 1; S < (1 << n); S++) {
        const int v = __builtin_ctz(S), rest = S & (S - 1);
        indep[S] = indep[rest] && !(adj[v] & rest);
    }
    std::vector<int> dp(1 << n, n + 1);
    dp[0] = 0;
    for (int S = 1; S < (1 << n); S++) {
        if (indep[S]) {
            dp[S] = 1;
            continue;
        }
        const int v = __builtin_ctz(S), low = 1 << v, cand = S & ~adj[v] & ~low;
        for (int U = cand;; U = (U - 1) & cand) {
            const int T = U | low;
            if (indep[T] && dp[S ^ T] + 1 < dp[S]) dp[S] = dp[S ^ T] + 1;
            if (U == 0) break;
        }
    }
    return dp[(1 << n) - 1];
}

/**
 * @brief 無向グラフの最大独立集合 (どの 2 頂点も隣り合わない頂点集合で、要素数が最大のもの) を 1 つ求め、頂点を昇順に返す (頂点数 64 以下)
 * 次数 1 以下の頂点は選んでよいので選び、そうでなければ次数最大の頂点を選ぶか選ばないかで分岐する。O(1.39^n) 程度 (n = 40 で十分速い)
 * @note 自己ループのある頂点は選ばない
 */
template <typename Cost = long long>
std::vector<long long> maximum_independent_set(const Graph<Cost>& graph) {
    const int n = graph.n;
    assert(n <= 64);
    using u64 = unsigned long long;
    std::vector<u64> adj(n, 0);
    u64 all = n == 64 ? ~0ULL : (1ULL << n) - 1;
    for (int u = 0; u < n; u++) {
        for (const auto& e : graph[u]) {
            if (e.dst == u) {
                all &= ~(1ULL << u);
            } else {
                adj[u] |= 1ULL << e.dst;
            }
        }
    }
    u64 best = 0;
    // rest: まだ決めていない頂点、cur: 選んだ頂点
    auto rec = [&](auto self, u64 rest, u64 cur) -> void {
        if (__builtin_popcountll(cur) + __builtin_popcountll(rest) <= __builtin_popcountll(best)) return;
        if (rest == 0) {
            best = cur;
            return;
        }
        int v = -1, deg = -1;
        for (u64 r = rest; r; r &= r - 1) {
            const int u = __builtin_ctzll(r), d = __builtin_popcountll(adj[u] & rest);
            if (d <= 1) {
                // 次数 1 以下なら選んでも損をしない
                self(self, rest & ~adj[u] & ~(1ULL << u), cur | 1ULL << u);
                return;
            }
            if (d > deg) v = u, deg = d;
        }
        self(self, rest & ~adj[v] & ~(1ULL << v), cur | 1ULL << v);
        self(self, rest & ~(1ULL << v), cur);
    };
    rec(rec, all, 0);
    std::vector<long long> res;
    for (int v = 0; v < n; v++)
        if (best >> v & 1) res.push_back(v);
    return res;
}

/**
 * @brief 単純な無向グラフの三角形 (互いに隣り合う 3 頂点) をすべて列挙し、それぞれについて f(a, b, c) (a < b < c) を呼ぶ O(m sqrt(m))
 * 辺を次数の小さい方から大きい方へ向けると、各頂点から出る辺は O(sqrt(m)) 本になる
 * @note 自己ループと多重辺はないこと
 */
template <typename Cost, typename F>
void enumerate_triangles(const Graph<Cost>& graph, F f) {
    const long long n = graph.n;
    auto less = [&](long long u, long long v) {
        return graph[u].size() != graph[v].size() ? graph[u].size() < graph[v].size() : u < v;
    };
    std::vector<std::vector<long long>> out(n);
    for (long long u = 0; u < n; u++) {
        for (const auto& e : graph[u])
            if (less(u, e.dst)) out[u].push_back(e.dst);
    }
    std::vector<bool> mark(n, false);
    for (long long u = 0; u < n; u++) {
        for (long long v : out[u]) mark[v] = true;
        for (long long v : out[u]) {
            for (long long w : out[v]) {
                if (!mark[w]) continue;
                long long t[3] = {u, v, w};
                std::sort(t, t + 3);
                f(t[0], t[1], t[2]);
            }
        }
        for (long long v : out[u]) mark[v] = false;
    }
}

namespace internal_graph_util {
/**
 * @brief minimum_steiner_tree の DP
 * dp[k][i]: 頂点 k を含み、X の部分集合 i をすべて含む部分木の重みの最小値
 * from[k][i]: 復元用。type が -1 なら X の頂点そのもの、0 なら i を x と i ^ x に分けた、1 なら頂点 x から辺 id を通って来た
 */
struct SteinerFrom {
    int type = -1;
    long long x = -1, id = -1;
};
template <typename Cost>
struct SteinerDP {
    std::vector<std::vector<Cost>> dp;
    std::vector<std::vector<SteinerFrom>> from;
};
template <typename Cost, typename T>
SteinerDP<Cost> steiner_dp(const Graph<Cost>& graph, const std::vector<T>& X, Cost inf, bool restore) {
    for (const auto& x : X) assert(0 <= x && x < graph.n);
    const int full = 1 << X.size();
    SteinerDP<Cost> res;
    auto& dp = res.dp;
    dp.assign(graph.n, std::vector<Cost>(full, inf));
    if (restore) res.from.assign(graph.n, std::vector<SteinerFrom>(full));
    for (int p = 0; p < static_cast<int>(X.size()); p++) dp[X[p]][1 << p] = {};
    for (int i = 0; i < full; i++) {
        for (int j = i; j > 0; j = (j - 1) & i) {
            for (long long k = 0; k < graph.n; k++) {
                if (dp[k][j] != inf && dp[k][i ^ j] != inf) {
                    if (dp[k][i] == inf || dp[k][i] > dp[k][j] + dp[k][i ^ j]) {
                        dp[k][i] = dp[k][j] + dp[k][i ^ j];
                        if (restore) res.from[k][i] = {0, j, -1};
                    }
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
                    if (restore) res.from[e.dst][i] = {1, u, e.id};
                    que.emplace(d + e.cost, e.dst);
                }
            }
        }
    }
    return res;
}
}  // namespace internal_graph_util

/**
 * @brief 頂点集合 X をすべて含む部分木の辺の重みの総和の最小値 O(3^k n + 2^k m log m) (k = |X|)
 * @note 無向グラフ。X が空なら 0。X を連結にできなければ inf を返す
 */
template <typename Cost, typename T>
Cost minimum_steiner_tree(const Graph<Cost>& graph, const std::vector<T>& X, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    if (X.empty()) return Cost{};
    const auto dp = internal_graph_util::steiner_dp(graph, X, inf, false).dp;
    Cost res = inf;
    for (long long k = 0; k < graph.n; k++) {
        if (dp[k].back() != inf && (res == inf || res > dp[k].back())) res = dp[k].back();
    }
    return res;
}

/**
 * @brief 頂点集合 X をすべて含む重み最小の部分木を 1 つ求め、その辺の番号を昇順に返す
 * 計算量は minimum_steiner_tree と同じ
 * @note 無向グラフ。X が空なら空。X の頂点どうしが連結でないときは呼ばないこと
 */
template <typename Cost, typename T>
std::vector<long long> minimum_steiner_tree_restore(const Graph<Cost>& graph, const std::vector<T>& X, Cost inf = std::numeric_limits<Cost>::max() / 4) {
    if (X.empty()) return {};
    const auto [dp, from] = internal_graph_util::steiner_dp(graph, X, inf, true);
    long long root = -1;
    for (long long k = 0; k < graph.n; k++) {
        if (dp[k].back() != inf && (root == -1 || dp[root].back() > dp[k].back())) root = k;
    }
    assert(root != -1);
    // 辺の重みが 0 だと同じ辺を 2 回使うことがあるので、最後に重複を除く
    std::vector<long long> res;
    std::vector<std::pair<long long, int>> stk = {{root, static_cast<int>(dp[root].size()) - 1}};
    while (!stk.empty()) {
        auto [k, i] = stk.back();
        stk.pop_back();
        const auto& f = from[k][i];
        if (f.type == 0) {
            stk.emplace_back(k, static_cast<int>(f.x));
            stk.emplace_back(k, i ^ static_cast<int>(f.x));
        } else if (f.type == 1) {
            res.push_back(f.id);
            stk.emplace_back(f.x, i);
        }
    }
    std::sort(res.begin(), res.end());
    res.erase(std::unique(res.begin(), res.end()), res.end());
    return res;
}
