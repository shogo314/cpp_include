#pragma once

/**
 * @file general-matching.hpp
 * @brief 一般グラフの最大マッチング
 */

#include <queue>
#include <vector>

#include "graph.hpp"
// https://github.com/shogo314/shogo-library/blob/02a0f569963e9e49fb42cc2b5e9e3cb802c9d9f5/src/graph/general-matching.hpp

namespace internal_general_matching {

/**
 * @brief Edmonds の花アルゴリズム
 *
 * 内部の配列は int で持つ
 */
template <typename Cost>
class Blossom {
   public:
    explicit Blossom(const Graph<Cost>& graph) : graph(graph), n(static_cast<int>(graph.n)), match(n, -1), p(n), base(n), used(n), in_blossom(n) {}

    std::vector<int> solve() {
        // 貪欲に作ったマッチングから始める
        for (int v = 0; v < n; v++) {
            if (match[v] != -1) continue;
            for (long long u : graph[v]) {
                if (u != v && match[u] == -1) {
                    match[u] = v, match[v] = static_cast<int>(u);
                    break;
                }
            }
        }
        for (int v = 0; v < n; v++) {
            if (match[v] != -1) continue;
            // 見つかった増加路に沿ってマッチングを入れ替える
            for (int t = find_path(v); t != -1;) {
                int pt = p[t], next = match[pt];
                match[t] = pt, match[pt] = t;
                t = next;
            }
        }
        return match;
    }

   private:
    const Graph<Cost>& graph;
    int n;
    std::vector<int> match, p, base;
    std::vector<bool> used, in_blossom;

    // 交互木の上で a と b の (花を縮めた後の) 最小共通祖先
    int lca(int a, int b) const {
        std::vector<bool> seen(n, false);
        while (true) {
            a = base[a];
            seen[a] = true;
            if (match[a] == -1) break;
            a = p[match[a]];
        }
        while (true) {
            b = base[b];
            if (seen[b]) return b;
            b = p[match[b]];
        }
    }
    void mark_path(int v, int b, int child) {
        while (base[v] != b) {
            in_blossom[base[v]] = in_blossom[base[match[v]]] = true;
            p[v] = child;
            child = match[v];
            v = p[match[v]];
        }
    }
    // root から増加路を探し、見つかればその終点 (マッチしていない頂点) を返す。なければ -1
    int find_path(int root) {
        used.assign(n, false);
        p.assign(n, -1);
        for (int i = 0; i < n; i++) base[i] = i;
        used[root] = true;
        std::queue<int> que;
        que.push(root);
        while (!que.empty()) {
            int v = que.front();
            que.pop();
            for (long long to_ : graph[v]) {
                int to = static_cast<int>(to_);
                if (base[v] == base[to] || match[v] == to) continue;
                if (to == root || (match[to] != -1 && p[match[to]] != -1)) {
                    // 奇閉路 (花) ができたので、1 つの頂点に縮める
                    int b = lca(v, to);
                    in_blossom.assign(n, false);
                    mark_path(v, b, to);
                    mark_path(to, b, v);
                    for (int i = 0; i < n; i++) {
                        if (!in_blossom[base[i]]) continue;
                        base[i] = b;
                        if (!used[i]) {
                            used[i] = true;
                            que.push(i);
                        }
                    }
                } else if (p[to] == -1) {
                    p[to] = v;
                    if (match[to] == -1) return to;
                    used[match[to]] = true;
                    que.push(match[to]);
                }
            }
        }
        return -1;
    }
};

}  // namespace internal_general_matching

/**
 * @brief 無向グラフの最大マッチング (Edmonds の花アルゴリズム) O(n^3)
 *
 * @return std::vector<Edge> マッチングに使った辺のリスト (各辺は src < dst の向きで 1 回ずつ、src の昇順)
 * @note 自己ループは無視する。多重辺があってもよい
 */
template <typename Cost = long long>
std::vector<typename Graph<Cost>::Edge> general_matching(const Graph<Cost>& graph) {
    std::vector<int> match = internal_general_matching::Blossom<Cost>(graph).solve();
    std::vector<typename Graph<Cost>::Edge> res;
    for (long long v = 0; v < graph.n; v++) {
        if (match[v] <= v) continue;
        for (const auto& e : graph[v]) {
            if (e.dst == match[v]) {
                res.push_back(e);
                break;
            }
        }
    }
    return res;
}
