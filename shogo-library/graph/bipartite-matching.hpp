#pragma once
// https://github.com/shogo314/shogo-library/blob/a2fefc7ec36d798ade24522d799381341eac20dd/src/graph/bipartite-matching.hpp

/**
 * @file bipartite-matching.hpp
 * @brief 二部グラフの最大マッチング (Hopcroft-Karp 法)
 */

#include <algorithm>
#include <cassert>
#include <queue>
#include <utility>
#include <vector>

/**
 * @brief 二部グラフの最大マッチング (Hopcroft-Karp 法)
 *
 * 左側の頂点 0, ..., l-1 と右側の頂点 0, ..., r-1 の間の辺から、最大マッチングを求める
 * O(m √n)。増加路を探す DFS は再帰を使わない
 * 例:
 * BipartiteMatching bm(l, r);
 * bm.add_edge(0, 1);
 * int k = bm.solve();
 */
class BipartiteMatching {
    int l, r;
    std::vector<std::vector<int>> g;
    std::vector<int> match_l, match_r;

    // 左側の自由な頂点からの交互路の長さで層を作る。増加路があれば true
    bool bfs(std::vector<int>& dist) const {
        dist.assign(l, -1);
        std::queue<int> que;
        for (int u = 0; u < l; u++) {
            if (match_l[u] == -1) {
                dist[u] = 0;
                que.push(u);
            }
        }
        bool found = false;
        while (!que.empty()) {
            int u = que.front();
            que.pop();
            for (int v : g[u]) {
                int w = match_r[v];
                if (w == -1) {
                    found = true;
                } else if (dist[w] == -1) {
                    dist[w] = dist[u] + 1;
                    que.push(w);
                }
            }
        }
        return found;
    }

   public:
    BipartiteMatching(int l, int r) : l(l), r(r), g(l), match_l(l, -1), match_r(r, -1) {}

    /**
     * @brief 左側の頂点数
     */
    int left_size() const { return l; }
    /**
     * @brief 右側の頂点数
     */
    int right_size() const { return r; }

    /**
     * @brief 左側の頂点 u と右側の頂点 v の間に辺を追加する
     */
    void add_edge(int u, int v) {
        assert(0 <= u && u < l && 0 <= v && v < r);
        g[u].push_back(v);
    }

    /**
     * @brief 最大マッチングを求め、その辺の数を返す
     * 辺を追加してからもう一度呼ぶと、前回のマッチングを広げる
     */
    int solve() {
        std::vector<int> dist, iter(l);
        std::vector<int> st;
        while (bfs(dist)) {
            std::fill(iter.begin(), iter.end(), 0);
            for (int s = 0; s < l; s++) {
                if (match_l[s] != -1) continue;
                // st は s からの交互路の左側の頂点。st[k] は g[st[k]][iter[st[k]]] を通って次へ進む
                st.assign(1, s);
                while (!st.empty()) {
                    int u = st.back();
                    if (iter[u] == (int)g[u].size()) {
                        // u からは増加路がないので、このフェーズでは使わない
                        dist[u] = -1;
                        st.pop_back();
                        if (!st.empty()) iter[st.back()]++;
                        continue;
                    }
                    int v = g[u][iter[u]];
                    int w = match_r[v];
                    if (w == -1) {
                        // 増加路が見つかったので、交互路に沿ってマッチングを入れ替える
                        for (int x : st) {
                            int y = g[x][iter[x]];
                            match_l[x] = y;
                            match_r[y] = x;
                        }
                        st.clear();
                    } else if (dist[w] == dist[u] + 1) {
                        st.push_back(w);
                    } else {
                        iter[u]++;
                    }
                }
            }
        }
        int res = 0;
        for (int u = 0; u < l; u++) res += match_l[u] != -1;
        return res;
    }

    /**
     * @brief 左側の頂点 u とマッチした右側の頂点 (なければ -1)。solve() の後で使う
     */
    int left_match(int u) const { return match_l[u]; }
    /**
     * @brief 右側の頂点 v とマッチした左側の頂点 (なければ -1)。solve() の後で使う
     */
    int right_match(int v) const { return match_r[v]; }
    /**
     * @brief マッチングの辺 (左側の頂点, 右側の頂点) のリスト (左側の頂点の昇順)。solve() の後で使う
     */
    std::vector<std::pair<int, int>> matching() const {
        std::vector<std::pair<int, int>> res;
        for (int u = 0; u < l; u++) {
            if (match_l[u] != -1) res.emplace_back(u, match_l[u]);
        }
        return res;
    }
};
