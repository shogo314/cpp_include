#pragma once
// https://github.com/shogo314/shogo-library/blob/b591e1ab6db09d0025e385b1bb5f4d8982cc4d1f/src/graph/min-cost-flow.hpp

/**
 * @file min-cost-flow.hpp
 * @brief 最小費用流 (primal-dual 法)
 */

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 最小費用流 (primal-dual 法)
 *
 * 辺のコストは 0 以上であること。ポテンシャルを使い、最短路を Dijkstra 法で求める
 * 最短路 1 本ごとに Dijkstra 法を 1 回行うので O(F (n + m) log m) (F は流量)。実際には、1 本の路で多く流れることが多い
 * 例:
 * MinCostFlow<int, long long> mcf(n);
 * mcf.add_edge(0, 1, 5, 3);
 * auto [f, c] = mcf.flow(s, t, limit);
 *
 * @tparam Cap 容量の型
 * @tparam Cost コストの型。コストの総和が収まること
 */
template <typename Cap, typename Cost>
class MinCostFlow {
   public:
    /**
     * @brief 辺の情報
     */
    struct Edge {
        long long from, to;
        Cap cap;    //!< 容量
        Cap flow;   //!< 流れている量
        Cost cost;  //!< 1 だけ流したときのコスト
    };

   private:
    // 内部の辺と配列は int で持ち、公開する関数で long long にする
    struct InnerEdge {
        int to, rev;
        Cap cap;  // 残りの容量
        Cost cost;
    };
    long long n;
    std::vector<std::vector<InnerEdge>> g;
    std::vector<std::pair<int, int>> pos;  // 追加した辺が g のどこにあるか
    // ポテンシャル。容量が残っているすべての辺 (v, u) で cost - dual[u] + dual[v] >= 0 を保つ
    std::vector<Cost> dual;
    std::vector<Cost> dist;
    std::vector<int> prev_v, prev_e;
    std::vector<bool> vis;
    bool flowed = false;

    // s から t への最短路を求め、ポテンシャルを更新する。t に行けなければ false
    bool dual_ref(int s, int t) {
        std::fill(dist.begin(), dist.end(), std::numeric_limits<Cost>::max());
        std::fill(vis.begin(), vis.end(), false);
        using P = std::pair<Cost, int>;
        std::priority_queue<P, std::vector<P>, std::greater<P>> que;
        dist[s] = 0;
        que.emplace(0, s);
        while (!que.empty()) {
            int v = que.top().second;
            que.pop();
            if (vis[v]) continue;
            vis[v] = true;
            if (v == t) break;
            for (int i = 0; i < (int)g[v].size(); i++) {
                const auto& e = g[v][i];
                if (vis[e.to] || e.cap == 0) continue;
                Cost c = e.cost - dual[e.to] + dual[v];
                if (dist[e.to] - dist[v] > c) {
                    dist[e.to] = dist[v] + c;
                    prev_v[e.to] = v;
                    prev_e[e.to] = i;
                    que.emplace(dist[e.to], e.to);
                }
            }
        }
        if (!vis[t]) return false;
        // 確定した頂点だけ更新する。確定していない頂点 u は dist[u] >= dist[t] なので、条件は保たれる
        for (int v = 0; v < n; v++) {
            if (vis[v]) dual[v] -= dist[t] - dist[v];
        }
        return true;
    }

   public:
    explicit MinCostFlow(long long n = 0) : n(n), g(n), dual(n), dist(n), prev_v(n), prev_e(n), vis(n) {}

    long long size() const { return n; }

    /**
     * @brief from から to への容量 cap、コスト cost の辺を追加し、辺の番号 (追加した順に 0, 1, ...) を返す
     * flow や slope を呼んだ後には追加できない
     */
    long long add_edge(long long from, long long to, Cap cap, Cost cost) {
        assert(0 <= from && from < n && 0 <= to && to < n && cap >= 0 && cost >= 0);
        assert(!flowed && "MinCostFlow: add_edge after flow");
        long long id = pos.size();
        int from_id = g[from].size();
        int to_id = g[to].size() + (from == to);
        pos.emplace_back(from, from_id);
        g[from].push_back({(int)to, to_id, cap, cost});
        g[to].push_back({(int)from, from_id, 0, -cost});
        return id;
    }

    /**
     * @brief 流量とコストの関係を表す折れ線の頂点 (流量, コスト) のリスト
     *
     * s から t へ、limit を上限として流量が最大になるまで流す
     * 最初は (0, 0)、最後は (流した量, そのときの最小コスト)。傾きが変わる点だけを並べる
     * 何度も呼ぶと、前回までの流れに追加して流す (それぞれの呼び出しの中での量を返す)
     */
    std::vector<std::pair<Cap, Cost>> slope(long long s, long long t, Cap limit = std::numeric_limits<Cap>::max()) {
        assert(0 <= s && s < n && 0 <= t && t < n && s != t);
        flowed = true;
        Cap flow = 0;
        Cost cost = 0, prev_cost_per_flow = 0;
        std::vector<std::pair<Cap, Cost>> res = {{0, 0}};
        while (flow < limit) {
            if (!dual_ref(s, t)) break;
            Cap c = limit - flow;
            for (long long v = t; v != s; v = prev_v[v]) c = std::min(c, g[prev_v[v]][prev_e[v]].cap);
            for (long long v = t; v != s; v = prev_v[v]) {
                auto& e = g[prev_v[v]][prev_e[v]];
                e.cap -= c;
                g[v][e.rev].cap += c;
            }
            // 最短路の上で cost - dual[to] + dual[from] = 0 なので、和をとると 1 あたりのコストになる
            Cost d = dual[t] - dual[s];
            flow += c;
            cost += c * d;
            if (res.size() >= 2 && prev_cost_per_flow == d) res.pop_back();
            res.emplace_back(flow, cost);
            prev_cost_per_flow = d;
        }
        return res;
    }

    /**
     * @brief s から t へ、limit を上限として流量が最大になるまで流し、(流した量, その中での最小コスト) を返す
     * 何度も呼ぶと、前回までの流れに追加して流す
     */
    std::pair<Cap, Cost> flow(long long s, long long t, Cap limit = std::numeric_limits<Cap>::max()) {
        return slope(s, t, limit).back();
    }

    /**
     * @brief 番号 i の辺の情報
     */
    Edge get_edge(long long i) const {
        assert(0 <= i && i < (long long)pos.size());
        const auto& e = g[pos[i].first][pos[i].second];
        const auto& re = g[e.to][e.rev];
        return {pos[i].first, e.to, e.cap + re.cap, re.cap, e.cost};
    }
    /**
     * @brief すべての辺の情報 (追加した順)
     */
    std::vector<Edge> edges() const {
        std::vector<Edge> res;
        for (long long i = 0; i < (long long)pos.size(); i++) res.push_back(get_edge(i));
        return res;
    }
};
