#pragma once

/**
 * @file centroid-decomposition.hpp
 * @brief 重心分解
 */

#include <cassert>
#include <utility>
#include <vector>

#include "tree.hpp"
// https://github.com/shogo314/shogo-library/blob/17d2ef2dd4949a8970024228f924be340a812e67/src/graph/centroid-decomposition.hpp

/**
 * @brief 重心分解
 *
 * 木の重心を取り除いて、残った部分木それぞれで同じことをくり返す。取り除いた重心を親子にした木 (重心木) の深さは O(log n)
 * 重心 c の成分 (c を取り除く直前に c を含んでいた部分木) は、c から level が c より大きい頂点だけをたどって得られる
 * 前計算 O(n log n)。DFS は再帰を使わない
 * 例:
 * CentroidDecomposition cd(tree);  // tree は Tree<Cost>
 * for (long long c : cd.order()) {
 *     for (auto& sub : cd.subtrees(tree, c)) {
 *         for (auto [v, d] : sub) { ... }  // c を通るパスを、c の先の部分木ごとに数える
 *     }
 * }
 */
class CentroidDecomposition {
    long long n;
    // 内部の配列は int で持ち、公開する関数で long long にする
    std::vector<int> par, lev;
    std::vector<long long> ord;

   public:
    /**
     * @brief 木 tree を重心分解する
     */
    template <typename Cost>
    explicit CentroidDecomposition(const Tree<Cost>& tree) : n(tree.size()), par(n, -1), lev(n, -1) {
        ord.reserve(n);
        // 成分の頂点を 1 つと、その成分の重心の親 (重心木での親) を積む
        std::vector<std::pair<int, int>> st;
        if (n > 0) st.emplace_back(0, -1);
        std::vector<int> bfs, bfs_par(n), sz(n);
        while (!st.empty()) {
            auto [s, p] = st.back();
            st.pop_back();
            int depth = p == -1 ? 0 : lev[p] + 1;
            // s を含む成分を BFS で列挙する (取り除いた頂点は lev が決まっている)
            bfs.assign(1, s);
            bfs_par[s] = -1;
            for (int i = 0; i < (int)bfs.size(); i++) {
                int v = bfs[i];
                for (long long u : tree[v]) {
                    if (u == bfs_par[v] || lev[u] != -1) continue;
                    bfs_par[u] = v;
                    bfs.push_back(u);
                }
            }
            int total = bfs.size();
            for (int i = total - 1; i >= 0; i--) {
                int v = bfs[i];
                sz[v] = 1;
                for (long long u : tree[v]) {
                    if (u != bfs_par[v] && lev[u] == -1) sz[v] += sz[u];
                }
            }
            // s から、大きさが半分を超える子の方へ進む。進めなくなったところが重心
            int c = s;
            while (true) {
                int next = -1;
                for (long long u : tree[c]) {
                    if (u != bfs_par[c] && lev[u] == -1 && sz[u] * 2 > total) next = u;
                }
                if (next == -1) break;
                c = next;
            }
            par[c] = p;
            lev[c] = depth;
            ord.push_back(c);
            for (long long u : tree[c]) {
                if (lev[u] == -1) st.emplace_back(u, c);
            }
        }
        assert((long long)ord.size() == n && "CentroidDecomposition: not connected");
    }

    /**
     * @brief 頂点数を返す
     */
    long long size() const { return n; }
    /**
     * @brief 重心木の根 (最初に取り除いた重心)
     */
    long long root() const {
        assert(n > 0);
        return ord[0];
    }
    /**
     * @brief 重心木での v の親 (根なら -1)
     */
    long long parent(long long v) const {
        assert(0 <= v && v < n);
        return par[v];
    }
    /**
     * @brief 重心木での v の深さ (根なら 0)。O(log n) 以下になる
     */
    long long level(long long v) const {
        assert(0 <= v && v < n);
        return lev[v];
    }
    /**
     * @brief 重心を取り除いた順に並べた列 (重心木の親は子より前にある)
     */
    const std::vector<long long>& order() const { return ord; }

    /**
     * @brief 重心 c の成分から c を取り除いた、それぞれの部分木の (頂点, c からの距離) の一覧を返す
     *
     * 距離は辺の重みの和 (重みなしで読んだ木なら辺の数)。すべての c で合わせて O(n log n)
     */
    template <typename Cost>
    std::vector<std::vector<std::pair<long long, Cost>>> subtrees(const Tree<Cost>& tree, long long c) const {
        assert(0 <= c && c < n);
        std::vector<std::vector<std::pair<long long, Cost>>> res;
        for (const auto& e : tree[c]) {
            if (lev[e.dst] <= lev[c]) continue;
            // c の先の部分木を、親をたどらないように BFS する
            std::vector<std::pair<long long, Cost>> sub = {{e.dst, e.cost}};
            std::vector<long long> from = {c};
            for (int i = 0; i < (int)sub.size(); i++) {
                auto [v, d] = sub[i];
                for (const auto& f : tree[v]) {
                    if (f.dst == from[i] || lev[f.dst] <= lev[c]) continue;
                    sub.emplace_back(f.dst, d + f.cost);
                    from.push_back(v);
                }
            }
            res.push_back(std::move(sub));
        }
        return res;
    }
};
