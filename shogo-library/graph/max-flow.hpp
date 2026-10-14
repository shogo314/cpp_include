#pragma once
// https://github.com/shogo314/shogo-library/blob/a2fefc7ec36d798ade24522d799381341eac20dd/src/graph/max-flow.hpp

/**
 * @file max-flow.hpp
 * @brief 最大流 (Dinic 法)
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 最大流 (Dinic 法)
 *
 * O(n^2 m) だが、実際にはずっと速いことが多い。容量がすべて 1 なら O(m min(m^(1/2), n^(2/3)))
 * 例:
 * MaxFlow<long long> mf(n);
 * mf.add_edge(0, 1, 5);
 * long long f = mf.flow(s, t);
 *
 * @tparam Cap 容量の型
 */
template <typename Cap>
class MaxFlow {
   public:
    /**
     * @brief 辺の情報
     */
    struct Edge {
        int from, to;
        Cap cap;   //!< 容量
        Cap flow;  //!< 流れている量
    };

   private:
    struct InnerEdge {
        int to, rev;
        Cap cap;  // 残りの容量
    };
    int n;
    std::vector<std::vector<InnerEdge>> g;
    std::vector<std::pair<int, int>> pos;  // 追加した辺が g のどこにあるか
    std::vector<int> level, iter;

    bool bfs(int s, int t) {
        level.assign(n, -1);
        level[s] = 0;
        std::queue<int> que;
        que.push(s);
        while (!que.empty()) {
            int v = que.front();
            que.pop();
            for (const auto& e : g[v]) {
                if (e.cap > 0 && level[e.to] == -1) {
                    level[e.to] = level[v] + 1;
                    que.push(e.to);
                }
            }
        }
        return level[t] != -1;
    }
    // v から t へ、up を上限としてレベルグラフ上で流す
    Cap dfs(int v, int t, Cap up) {
        if (v == t) return up;
        Cap res = 0;
        for (int& i = iter[v]; i < (int)g[v].size(); i++) {
            InnerEdge& e = g[v][i];
            if (e.cap <= 0 || level[e.to] != level[v] + 1) continue;
            Cap d = dfs(e.to, t, std::min(up - res, e.cap));
            if (d <= 0) continue;
            e.cap -= d;
            g[e.to][e.rev].cap += d;
            res += d;
            if (res == up) return res;
        }
        // これ以上流せないので、このフェーズでは使わない
        level[v] = n;
        return res;
    }

   public:
    explicit MaxFlow(int n = 0) : n(n), g(n) {}

    int size() const { return n; }

    /**
     * @brief from から to への容量 cap の辺を追加し、辺の番号 (追加した順に 0, 1, ...) を返す
     */
    int add_edge(int from, int to, Cap cap) {
        assert(0 <= from && from < n && 0 <= to && to < n && cap >= 0);
        int id = pos.size();
        int from_id = g[from].size();
        int to_id = g[to].size() + (from == to);
        pos.emplace_back(from, from_id);
        g[from].push_back({to, to_id, cap});
        g[to].push_back({from, from_id, 0});
        return id;
    }

    /**
     * @brief s から t へ、limit を上限としてできるだけ流し、流した量を返す
     * 何度も呼ぶと、前回までの流れに追加して流す
     */
    Cap flow(int s, int t, Cap limit = std::numeric_limits<Cap>::max()) {
        assert(0 <= s && s < n && 0 <= t && t < n && s != t);
        Cap res = 0;
        while (res < limit && bfs(s, t)) {
            iter.assign(n, 0);
            Cap f = dfs(s, t, limit - res);
            if (f == 0) break;
            res += f;
        }
        return res;
    }

    /**
     * @brief 番号 i の辺の情報
     */
    Edge get_edge(int i) const {
        assert(0 <= i && i < (int)pos.size());
        const auto& e = g[pos[i].first][pos[i].second];
        const auto& re = g[e.to][e.rev];
        return {pos[i].first, e.to, e.cap + re.cap, re.cap};
    }
    /**
     * @brief 番号 i の辺の容量を new_cap に、流れている量を new_flow に変える
     * ほかの辺の流量は変えないので、流量の保存則が崩れることがある
     */
    void change_edge(int i, Cap new_cap, Cap new_flow) {
        assert(0 <= i && i < (int)pos.size() && 0 <= new_flow && new_flow <= new_cap);
        auto& e = g[pos[i].first][pos[i].second];
        auto& re = g[e.to][e.rev];
        e.cap = new_cap - new_flow;
        re.cap = new_flow;
    }
    /**
     * @brief すべての辺の情報 (追加した順)
     */
    std::vector<Edge> edges() const {
        std::vector<Edge> res;
        for (int i = 0; i < (int)pos.size(); i++) res.push_back(get_edge(i));
        return res;
    }

    /**
     * @brief 直前の flow(s, t) の後で、残余グラフで s から行ける頂点か
     * flow(s, t) で流しきっていれば、true の頂点の集合が最小カットの s 側になる
     */
    std::vector<bool> min_cut(int s) const {
        std::vector<bool> visited(n);
        std::vector<int> st = {s};
        visited[s] = true;
        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            for (const auto& e : g[v]) {
                if (e.cap > 0 && !visited[e.to]) {
                    visited[e.to] = true;
                    st.push_back(e.to);
                }
            }
        }
        return visited;
    }
};
